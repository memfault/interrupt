---
title: "Why std::this_thread::sleep_for() is broken on ESP32"
description:
  A deep dive into how std::this_thread::sleep_for() is implemented on ESP32,
  and why it is broken in IDF v5.
author: stevenoonan
tags: [esp32, freertos, c++, c, idf]
---

<!-- excerpt start -->

A curious bug appearing after upgrading to IDF v5 led me into a deep dive of how
`std::this_thread::sleep_for()` is implemented on the ESP32. I discuss how the
IDF implements `pthreads` and `newlib` to provide C++ threading functionality.
The results are surprising: a simple 10 millisecond sleep was killing
performance, but only in the new version of IDF due to an interaction between
`libstdc++` and `usleep()`.

<!-- excerpt end -->

> 🎬
> [Listen to Steve on Interrupt Live](https://youtube.com/live/dwL-PI7TuDY?feature=share)
> talk about the content and motivations behind writing this article.

{% include newsletter.html %}

{% include toc.html %}

## Why so slow IDF v5?

One of my [ESP32](https://www.espressif.com/en/products/socs/esp32) projects was
at version v4.4 of Espressif's
[IoT Development Framework](https://www.espressif.com/en/products/sdks/esp-idf)
(IDF). After about two days of work, the project compiled and ran in IDF v5.3.
Except, something was not quite right. Things were sluggish. FreeRTOS's
[system state API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html#_CPPv420uxTaskGetSystemStatePC12TaskStatus_tK11UBaseType_tPC27configRUN_TIME_COUNTER_TYPE)
indicated that one of the two Xtensa LX6 processors on the ESP32 was pinned at
100% usage. The culprits were two high-priority threads, both of which had a
relatively small loop with a 10 millisecond sleep between each iteration.

Sleeping for 10 milliseconds should leave plenty of time to do other things.
Plus, this code worked fine in IDF v4 without an issue. What did IDF v5 do?

## FreeRTOS on the ESP32

Let's go into some background first. Experienced embedded engineers will likely
find parts of this section as a review, but we need to be solid in our
understanding of the fundamentals before moving on.

### System Tick Period

My project is set to the default ESP32 FreeRTOS tick period of 10 milliseconds,
represented by the `portTICK_PERIOD_MS` macro:

```c
#define CONFIG_FREERTOS_HZ 100 /* sdkconfig.h */
#define configTICK_RATE_HZ CONFIG_FREERTOS_HZ /* FreeRTOSConfig.h */
#define portTICK_PERIOD_MS ((uint32_t)1000 / configTICK_RATE_HZ) /* portmacro.h */
```

This is used by ESP32 to setup the system tick (systick) interrupt, done by
`vSystimerSetup()` in `components\freertos\port_systick.c`. Notably, there are
two system ticks because there are two cores. Here is the interesting part done
for each core:

```c
/* configure the timer */
uint32_t alarm_id = SYSTIMER_ALARM_OS_TICK_CORE0 + cpuid;
systimer_hal_connect_alarm_counter(&systimer_hal, alarm_id, SYSTIMER_COUNTER_OS_TICK);
systimer_hal_set_alarm_period(&systimer_hal, alarm_id, 1000000UL / CONFIG_FREERTOS_HZ);
systimer_hal_select_alarm_mode(&systimer_hal, alarm_id, SYSTIMER_ALARM_MODE_PERIOD);
```

Obviously, by "sleeping" a thread for a period of time, we want to _yield_
execution to other threads. This is accomplished by the
[FreeRTOS scheduler](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html#smp-scheduler).
The basic unit of the scheduler is the system tick period. Other things can
occur based on interrupts as well, but terms of sleeping threads, the system
tick period is what really matters.

ESP32's FreeRTOS scheduler is running a
[Best Effort Round Robin time slicing](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html#time-slicing)
algorithm. This is slightly different than the standard FreeRTOS algorithm
("best effort" instead of perfect) due to
[symmetric-multiprocessing](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html#symmetric-multiprocessing).
The important bit to note here is even when a thread does _not_ explicitly
yield, the scheduler will
[time slice](<https://en.wikipedia.org/wiki/Preemption_(computing)#Time_slice>)
amongst the highest priority ready-state threads on the system tick period
intervals.

### Small Sleeps and Delays

The following code yields the minimal amount of time:

```c
vTaskDelay(1);
```

This will guarantee the thread will sleep _at least_ until the next system tick
interrupt.

But when is the next system tick interrupt in relation to the execution of
`vTaskDelay(1)`? It could be in 1 nanosecond, all the way up to our system tick
period of 10 milliseconds. Exactly how much time the thread will be sleep is
unknown to this code.

So to be clear: sleeping _"1 tick"_ does not mean it will sleep for one tick
_period of time_. In fact, it will always sleep some amount less than one tick
period, and sometimes it will not sleep at all.

So then, is calling `vTaskDelay(1)` in a small loop a problem? Not
intrinsically. Like most things, it's fine if you know what you are doing. If
you want to ensure a thread (technically called a
[Task](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines)
in FreeRTOS) executes its logic is performed on every tick, this a way to do it.
There are no problems so long as the logic is not sensitive to tick jitter and
its processing only requires a small fraction of the tick period.

We could attempt to be more idiomatic by using code like this:

```c
vTaskDelay(pdMS_TO_TICKS(10));
```

The macro `pdMS_TO_TICKS()` converts milliseconds into ticks by accounting for
the FreeRTOS configuration tick period. If the period changes or, god forbid,
the code is copied and pasted into a different project, the proper tick count
would be calculated. If our tick period is 10 milliseconds it will call
`vTaskDelay(1)`. All is great.

Except not always. Say we want to sleep 9 milliseconds. If we write
`vTaskDelay(pdMS_TO_TICKS(9))` the macro will actually call `vTaskDelay(0)`.
This is because it does integer math:

```c
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS( xTimeInMs )    ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000U ) )
```

So `(9*100)/1000 = 0.9`, but as a `uint32_t`, just `0`. This is certainly not
what we want, because passing `0` to `vTaskDelay()` does nothing at all.

Okay, but what if we _really_ need to wait for less than a tick period? We can
[busy wait](https://en.wikipedia.org/wiki/Busy_waiting). The IDF has
`esp_rom_delay_us()`, which does just that. But this is an
[Internal API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/internal-unstable.html)
which we don't have source code for, and its functionality may change or be
removed in the future. ESP does give us `usleep()` though, and that allows
precise delays. The caveat here is that the FreeRTOS scheduler is still free to
context switch out of the busy loop due to the important bit I said above about
time slicing. If you must _absolutely_ guarantee timing, then you must disable
the FreeRTOS scheduler (and perhaps all interrupts) for that period.

The other option is to use a free running timer and trigger an interrupt when it
expires. The interrupt can signal FreeRTOS to wake the waiting task. This is
great as long as your context switch overheads allow it and you have a free
hardware timer to do it.

All of this is to say the following:

1. If you care about precision timing, details matter.
2. If you _don't care_ about precision timing but are using it, you are probably
   wasting resources.

## C++: the Solution to and Cause of All My Problems

Alright then, so what does all this have to do with our issue related to a 10
millisecond delay? Well, I am not actually using `vTaskDelay(1)` directly.
Instead, I am using C++ thread APIs, and therefore, the actual code causing all
the CPU usage is `std::this_thread::sleep_for(10ms)`.

Espressif provides
[good support](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/cplusplus.html)
for C++, including
[`std::thread`](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/cplusplus.html).
This allows us to write portable C++ code. But it's also where our problem lies.
So how does something like an ESP32 MCU support C++ threads anyway?

### `std::thread`, POSIX Threads, and FreeRTOS Tasks, oh my!

Espressif's IDF compiles with GCC and therefore uses
[The GNU C++ Library](https://gcc.gnu.org/onlinedocs/libstdc++/) (`libstdc++`).
`libstdc++`'s implementation of `std::thread` is built on top of
[`pthreads`](https://en.wikipedia.org/wiki/Pthreads), but it is up to the
platform to provide a `pthread` implementation. Espressif did just
[that](https://github.com/espressif/esp-idf/tree/v5.3.2/components/pthread),
including things like
[thread local storage](https://en.wikipedia.org/wiki/Thread-local_storage).

If you read the code you'll see that IDF's `pthreads` is using FreeRTOS APIs for
thread creation, synchronization, and all functionality found within
`std::this_thread`, such as
[`sleep_for()`](https://en.cppreference.com/w/cpp/thread/sleep_for).

If that's not enough, we also need to consider the
[C standard library](https://en.wikipedia.org/wiki/C_standard_library). IDF uses
[newlib](https://en.wikipedia.org/wiki/Newlib) and has custom platform
implementations of its sleeping functions. Thankfully, `newlib` is not
precompiled and you can easily find its source code in the `components/newlib`
directory of an IDF install.

My problem appears in IDF v5 because it upgraded GCC from 8.4 to 13.2, and
`libstdc++`'s implementation of `std::this_thread::sleep_for()` changed. But is
`libstdc++` really to blame? Let's see.

### IDF 4.x: `std::this_thread::sleep_for()`

Let's look at the working side of things first. When calling
`std::this_thread::sleep_for(10ms)` we first enter here:

```c++
/// sleep_for
template<typename _Rep, typename _Period>
inline void
sleep_for(const chrono::duration<_Rep, _Period>& __rtime)
{
  if (__rtime <= __rtime.zero())
    return;
  auto __s = chrono::duration_cast<chrono::seconds>(__rtime);
  auto __ns = chrono::duration_cast<chrono::nanoseconds>(__rtime - __s);
#ifdef _GLIBCXX_USE_NANOSLEEP
  __gthread_time_t __ts =
    {
      static_cast<std::time_t>(__s.count()),
      static_cast<long>(__ns.count())
    };
  while (::nanosleep(&__ts, &__ts) == -1 && errno == EINTR)
    { }
#else
  __sleep_for(__s, __ns);
#endif
}
```

If you are following along in your own IDF project, you can find the
`std::this_thread::sleep_for()` implementation in the ESP tools directory under
`xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf/xtensa-esp32-elf/include/c++/8.4.0/thread`
(or similar, depending on the version).

IDF does not define `_GLIBCXX_USE_NANOSLEEP`, so no `nanosleep()`. That
shouldn't be too surprising for an MCU implementation. Therefore, we go straight
to `__sleep_for(__s, __ns)`.

Here is GCC 8.4 C++11
[libstdc++](https://github.com/gcc-mirror/gcc/tree/releases/gcc-8.4.0/libstdc%2B%2B-v3)'s
code for
[`__sleep_for()`](https://github.com/gcc-mirror/gcc/blob/releases/gcc-8.4.0/libstdc%2B%2B-v3/src/c%2B%2B11/thread.cc#L186):

```c++
namespace this_thread
{
  void
  __sleep_for(chrono::seconds __s, chrono::nanoseconds __ns)
  {
#ifdef _GLIBCXX_USE_NANOSLEEP
    __gthread_time_t __ts =
      {
        static_cast<std::time_t>(__s.count()),
        static_cast<long>(__ns.count())
      };
    while (::nanosleep(&__ts, &__ts) == -1 && errno == EINTR)
      { }
#elif defined(_GLIBCXX_HAVE_SLEEP)
# ifdef _GLIBCXX_HAVE_USLEEP
    ::sleep(__s.count());
    if (__ns.count() > 0)
      {
        long __us = __ns.count() / 1000;
        if (__us == 0)
          __us = 1;
        ::usleep(__us);
      }
# else
    ::sleep(__s.count() + (__ns.count() >= 1000000));
# endif
#elif defined(_GLIBCXX_HAVE_WIN32_SLEEP)
    unsigned long ms = __ns.count() / 1000000;
    if (__ns.count() > 0 && ms == 0)
      ms = 1;
    ::Sleep(chrono::milliseconds(__s).count() + ms);
#endif
  }
}

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std
```

Let's clean it up by taking out the parts not compiled for the ESP32:

```c++
 __sleep_for(chrono::seconds __s, chrono::nanoseconds __ns)
  {
    ::sleep(__s.count());
    if (__ns.count() > 0)
      {
        long __us = __ns.count() / 1000;
        if (__us == 0)
          __us = 1;
        ::usleep(__us);
      }
  }
```

Pretty simple: get the number of seconds and call `sleep()`, then get the number
of microseconds and call `usleep()`. Both of these are calls into `newlib`. We
will get to those in a moment. First, let's look at the IDF v5's `sleep_for()`.

### IDF 5.x: `std::this_thread::sleep_for()`

Once again, `sleep_for()` is a wrapper for `__sleep_for()`. Here is the newer
code
([see online](https://github.com/gcc-mirror/gcc/blob/1c0ecf06c0c9f3268ba52d4027e9b78aae4acb51/libstdc%2B%2B-v3/src/c%2B%2B11/thread.cc#L235)):

```c++
__sleep_for(chrono::seconds __s, chrono::nanoseconds __ns)
  {
    const auto target = chrono::steady_clock::now() + __s + __ns;
    while (true)
      {
        unsigned secs = __s.count();
        if (__ns.count() > 0)
          {
            long us = __ns.count() / 1000;
            if (us == 0)
              us = 1;
            ::usleep(us);
          }
        if (secs > 0)
          {
            // Sleep in a loop to handle interruption by signals:
            while ((secs = ::sleep(secs)))
              { }
          }
        const auto now = chrono::steady_clock::now();
        if (now >= target)
          break;
        __s = chrono::duration_cast<chrono::seconds>(target - now);
        __ns = chrono::duration_cast<chrono::nanoseconds>(target - (now + __s));
    }
  }
```

Now we have a `while` loop. In it, `usleep()` is called, but afterward, there is
an explicit check to determine if the minimal amount of time has actually gone
by. The only way out of the loop is if `now` is at or after the `target` time.

Why the change? This is because `nanosleep()` may return early if a signal is
delivered to the thread during its sleep. The change is in
[commit cfef4c3](https://github.com/gcc-mirror/gcc/commit/cfef4c324ac300c0ad120f0fcee376159de84a0c),
with the relevant commit comment as the following:

```git
Loop while sleep call is interrupted and until steady_clock
 shows requested duration has elapsed.
```

ESP IDF doesn't support signals, so the change should have no impact on the
ESP32. Oh, but it does!

## IDF Newlib

A few things to unpack here which lead us into Newlib. Let's start with how
`chrono::steady_clock::now()` works.

### `clock_gettime()`

Calling `chrono::steady_clock::now()` on the ESP32 is just a wrapper for
`clock_gettime()` located in `newlib` (`idf/components/newlib/time.c`), where
`clock_id` is `CLOCK_MONOTONIC`.

```c
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
#if IMPL_NEWLIB_TIME_FUNCS
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }
    struct timeval tv;
    uint64_t monotonic_time_us = 0;
    switch (clock_id) {
    case CLOCK_REALTIME:
        _gettimeofday_r(NULL, &tv, NULL);
        tp->tv_sec = tv.tv_sec;
        tp->tv_nsec = tv.tv_usec * 1000L;
        break;
    case CLOCK_MONOTONIC:
        monotonic_time_us = esp_time_impl_get_time();
        tp->tv_sec = monotonic_time_us / 1000000LL;
        tp->tv_nsec = (monotonic_time_us % 1000000LL) * 1000L;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    return 0;
#else
    errno = ENOSYS;
    return -1;
#endif
}
```

Calling `esp_time_impl_get_time()` gets a value based on a free-running hardware
timer with at least 1 us accuracy. So that is how the monotonic clock is
implemented on the ESP32. Note that this clock is completely independent of the
timer that is controlling the FreeRTOS system tick interrupt. No problems there.

### `sleep()`

ESP32 implements `sleep()` as a simple wrapper of `usleep()`:

```c
unsigned int sleep(unsigned int seconds)
{
    usleep(seconds * 1000000UL);
    return 0;
}
```

### `usleep()`

Okay, so in both versions of IDF, `usleep()` is at the bottom of all sleeping
calls. Here it is:

```c
int usleep(useconds_t us)
{
    const int64_t us_per_tick = portTICK_PERIOD_MS * 1000;
    if (us < us_per_tick) {
        esp_rom_delay_us((uint32_t) us);
    } else {
        /* since vTaskDelay(1) blocks for anywhere between 0 and portTICK_PERIOD_MS,
         * round up to compensate.
         */
        vTaskDelay((us + us_per_tick - 1) / us_per_tick);
    }
    return 0;
}
```

Remember when I said that `esp_rom_delay_us()` is an internal API and we should
use `usleep()` instead? Well, `usleep()` is just a wrapper to
`esp_rom_delay_us()` for short periods (e.g. less than one tick period). We can
consider `usleep()` the "public" exposure of `esp_rom_delay_us()`, but only when
the specified time is less than a system tick period. As mentioned above, this
is a busy wait, and since it does not disable the scheduler, it still allows
other threads _of equal or higher priority_ to run. So, the timing represents a
guaranteed _minimum_ only. More importantly, if there are other threads of lower
priority, it will _not_ context switch during this busy time. It will just sit
in the thread until the wait is over.

This is all good. A guaranteed minimum is how I expect `usleep()` to work.

In terms of blocking vs yielding, IDF
[docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/pthread.html?#rtos-integration)
say:

> When calling a standard libc or C++ sleep function, such as usleep defined in
> unistd.h, the task will only block and yield the core if the sleep time is
> longer than one FreeRTOS tick period. If the time is shorter, the thread will
> busy-wait instead of yielding to another RTOS task.

It should say for sleeping _equal to_ or longer than one tick period cause
yielding vs. busy waiting. In any case, the yielding is done via `vTaskDelay()`.

There is a problem here though. The ticks to yield calculations often produce
times yielded _less than_ the specified amount.

Let's play out an example. If we wanted to sleep for 15 milliseconds, the
calculations would give us `vTaskDelay(2)`:

1. `portTICK_PERIOD_MS` is defined as `10`. So `us_per_tick` is `10,000`.
2. `us` is `15000`
3. `(15000 + 10000 - 1) / 10000)` is `(24999) / 10000` which gives `2.4999`
4. But since we are using integers, it is just `2`

But what does `2` mean? It means two occurrences of the tick interrupt. Once
again, we can ask: when is the next tick interrupt from the moment that
`vTaskDelay(2)` is called? The tick interrupt could be in 1 nanosecond all the
way up to 10 milliseconds from now. After that, the second tick will be the
system tick period of 10 milliseconds. So for a tick count of `2`, we will
actually sleep between between 10 and 20 milliseconds.

Even though the comment says it is rounding up to compensate for the first tick
potentially not blocking at all, the compensation does not account for the
worse-case minimal timing. In the example I gave, a 15-millisecond request will
sometimes only sleep for 10 milliseconds. Likewise, a 10 millisecond `usleep()`
will sometimes sleep about 0 milliseconds. The greatest _potential_ differential
comes with calling `usleep()` with a multiple of the tick period. In that case,
the time spent may be short by an entire tick period.

Whether this "shorter than specified" time matters depends on requirements. In
my experience, a lot of application-level logic would probably be fine with a
"10 to 20" millisecond wait. On the other hand, hardware driver logic and audio
pipelines have specific realtime characteristics where this would absolutely be
a problem.

According to `man 3 sleep` and
[POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/usleep.html),
`usleep()` should always sleep _at least_ the time specified. It is allowed to
sleep more if needed.

> The usleep() function shall cause the calling thread to be suspended from
> execution until either the number of realtime microseconds specified by the
> argument useconds has elapsed or a signal is delivered to the calling thread
> and its action is to invoke a signal-catching function or to terminate the
> process. The suspension time may be longer than requested due to the
> scheduling of other activity by the system.

The fact that ESP32's IDF `usleep()` can be short by up to one system tick
period means it doesn't follow the specifications.

## Would You Like Precision or Efficiency?

Let's recap.

In IDF v4, calling `std::this_thread::sleep_for()` calls `usleep()` once. When
`std::this_thread::sleep_for(10ms)` is executed, it calls `vTaskDelay(1),` and
the thread will sleep between 0 and 10 milliseconds. It will usually sleep for
less than the time specified.

In IDF v5, calling `std::this_thread::sleep_for(10ms)` almost always calls
`usleep()` _twice_. The first time will use `vTaskDelay(1)`, and it will usually
sleep for less than the time specified. Then, back in `libstdc++`
`__sleep_for()`, the monotonic clock will be checked and it will be seen that
some fractional component of 10 milliseconds remains, causing a second call to
`usleep()`. The second call will always be with a time less than the system tick
period. This will cause a blocking call to `esp_rom_delay_us()`. That blocking
call is the problem.

Let's break it down further. For a 10 millisecond delay:

1. `(10000 + 10000 - 1) / 10000)` is `(19999) / 10000` which gives `1.9999`
2. But since we are using integers, it is just `1`

But once again, saying `vTaskDelay(1)` means "wait for 1 tick interrupt." That
will almost always be less than 10 milliseconds from now. When `vTaskDelay(1)`
returns, somewhere between 0.000001 and 10.000000 milliseconds will have
actually transpired. The newer version of `__sleep_for()` in IDF v5 / GCC 13
will double-check that the correct time has actually elapsed according to the
monotonic clock. That check will fail, so it calls `usleep()` again, this time
with the remainder of the 10 milliseconds. The specified time is less than a
system tick period, so the blocking `esp_rom_delay_us()` is now called.

So what about time slicing? Even if `esp_rom_delay_us()` blocks, the FreeRTOS
scheduler can switch to another task. Firstly, if this thread is of a higher
priority, _no lower priorities will ever run_. But even if everything is of the
same priority, the CPU will just switch back to the blocking call on the next
round robin, continuing the blocking wait. In our current scenario, this is
horribly inefficient, unnecessary, and unexpected.

Any call to `sleep_for()` greater than the tick period has this problem because
the tick interrupt is asynchronous to the `sleep_for()` call. This means when
the scheduler returns from `vTaskDelay()` some random remainder of time will be
done with `esp_rom_delay_us()` in order to sleep for the _precise_ amount of
time requested.

The new version of `sleep_for()` is much more precise, but it is at the cost of
computing efficiency on the ESP32 because some fraction of the tick period will
be _busy waited_ instead of yielded. That is very bad to do on an MCU.

Of course, none of this is transparent to the application code, and I doubt it
was something intentional from Espressif. It is just a consequence of upgrading
to a new version of GCC combined with how IDF implements `usleep()`.

## Is `usleep()` broken in IDF v5?

Did Espressif actually implement `usleep()` wrong? Yes. It needs to be fixed.

For periods at or longer than the system tick, `usleep()` can return before the
specified time. It shouldn't do that. It must error on the side of sleeping too
long to ensure it _never_ sleeps too little. So yes, it is broken in my view.
`stdlibc++` isn't to blame.

Since `usleep()` is sometimes short by 1 system tick period, we could just add
another tick count to our calculations. That would sometimes cause `usleep()` to
take longer, but never shorter than the specified time. This would prevent
`__sleep_for()` from calling `usleep()` a second time with a fraction of a
system tick period, so no more blocking busy wait.

We can be a little more sophisticated by taking a cue from the newer
`__sleep_for()` and keep checking the monotonic clock. Something like this:

```c
int usleep(useconds_t us)
{
    const int64_t us_per_tick = portTICK_PERIOD_MS * 1000;
    if (us < us_per_tick) {
        esp_rom_delay_us((uint32_t) us);
    } else {
        /* vTaskDelay may return up to (n-1) tick periods due to the tick ISR
           being asynchronous to the call. We must sleep at least the specified
           time, or longer. Checking the monotonic clock allows making an
           additional call to vTaskDelay when needed to ensure minimal time is
           actually slept. Adding `us_per_tick - 1` prevents ever passing 0 to
           vTaskDelay().
        */
        uint64_t now_us = esp_time_impl_get_time();
        uint64_t target_us = now_us + us;
        do {
            vTaskDelay((((target_us - now_us) + us_per_tick - 1) / us_per_tick));
            now_us = esp_time_impl_get_time();
        } while (now_us < target_us);
    }
    return 0;
}
```

With this approach, short sleep times are precise (as they always were), and
longer sleep times are non-blocking and always as long or longer than the
requested time. By testing the monotonic clock, the extra tick is only applied
when needed.

I opened a [PR](https://github.com/espressif/esp-idf/pull/15132) with this
change to IDF. Hopefully it gets approved.

Since I didn't want to work with a custom, patched IDF, I replaced all calls to
`std::this_thread::sleep_for()` with our own function. It has the same default
signature, with the option to specify a "sleep strategy." We can now force the
custom `sleep_for()` to busy wait or to yield:

```c++
enum class SleepStrategy {
    Default,            // Platform decides when to use busy wait vs. yield
    PreciseBusyWait,    // Busy wait, which is usually very precise
    EfficientYield      // Efficently yield to other threads, but will often sleep longer than specified
};

template<typename _Rep, typename _Period>
static void sleep_for(const std::chrono::duration<_Rep, _Period>& rtime, SleepStrategy strat = SleepStrategy::Default) {

    static constexpr int64_t us_per_tick = portTICK_PERIOD_MS * 1000;
    if (rtime <= rtime.zero()) return;
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(rtime).count();

    if (strat == SleepStrategy::Default && (us < us_per_tick)) {
        // Mimic how std::this_thread::sleep_for() would act, which is to
        // (eventually) call usleep() after going through libstdc++. We only do
        // this for periods less than a tick because for longer periods the
        // implementation is broken (often returns in shorter than specified
        // time). If `usleep()` is fixed then we will update this to always call
        // it for the default strategy.
        usleep(us);
        return;
    }

    uint64_t now_us = esp_timer_get_time();
    const uint64_t target_us = now_us + us;
    do {
        if (strat == SleepStrategy::PreciseBusyWait) {
            // This is an "internal and unstable API" according to ESP, but
            // `usleep()` is just a wrapper for it anyway. If it does change,
            // this function needs to be updated.
            esp_rom_delay_us(target_us - now_us);
        } else {
            // Ensure we never call vTaskDelay(0)
            static constexpr int prevent_zero_ticks = us_per_tick - 1;
            vTaskDelay((TickType_t)(((target_us - now_us) + prevent_zero_ticks) / us_per_tick));
        }
        // Validate against monotonic clock
        now_us = esp_timer_get_time();
    } while (now_us < target_us);
}
```

This approach gives us the minimal change needed to ensure things work correctly
while allowing more control over how to perform the sleep when using C++.

## Conclusion

I cut my teeth on bare metal C code where _everything_ was statically allocated.
No `malloc()`. No floating point math because there was no FPU. Custom linker
scripts. Debugging using GPIO pins and an oscilloscope. Using precalculated
value tables to save a few microseconds in an ISR. We ran at 24 MHz. At that
time, C was luxurious because the "old" stuff was running at 1 MHz, usually in
assembly.

Back then common wisdom was that C++ was just not appropriate for
microcontrollers. Of course, that was a long time ago, before "modern C++" and
before MCUs were clocked at hundreds of MHz with L1 caching and multistage
instruction pipelines.

It seems today that using C++ for firmware brings up a lot of strong reactions.
A lot of embedded people hate it. A lot of people love it. For myself, I think
it can be a great tool, but it does have much complexity you need to get right,
_especially_ when using it on an MCU. This seems to be a good example of such.

I sincerely hope `usleep()` is fixed. Until then, don't use
`std::this_thread::sleep_for()` in your IDF v5 projects. It's a waste of time!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
