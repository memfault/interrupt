---
title: Debugging Android Devices
description:
  Exploring the tools and data available to embedded Android developers to fix issues with their AOSP based devices.
author: chayes
tags: [android, debugging]
---

All hardware devices experience bugs and need debugging. Android devices in specific are exceptionally complex with several hundred gigabytes of source code, dozens of components, and wide range of uses. 

<!-- excerpt start -->

In this article we will explore the different facilities and tools available to debug Android based devices and produce robust systems that can handle a wide range of applications from smart fridges, to payment terminals, and of course mobile phones.

<!-- excerpt end -->

> 📺 **Watch the Webinar Recording**
>
> This topic was also the subject of a Memfault webinar on [How to Debug Android Devices in Production](https://hubs.la/Q01YJZXG0).

{% include toc.html %}

## Logging

The first place any debugging starts is usually looking through logs. Android has two sets of logs, Logcat and Kernel logs.

### Logcat

Logcat logs are the bread and butter of debugging android devices. There are circular buffers for main, system, crash, radio, and event logs for binaries running within the Android runtime. Each of these buffers are written to corresponding files in `/dev/log/`. Log messages can be sent from system services, applications, and native binaries. The logcat CLI tool allows you to filter on specific packages and control the verbosity on a per package basis.

### Kernel Logs

`/proc/kmesg` can be read to get the kernel level logs. These logs are crucial for diagnosing and troubleshooting various system issues, including hardware failures, software bugs, and performance problems. Kernel logs provide insights into the inner workings of the operating system and its interaction with hardware devices. If you are integrating a new device and writing a driver for it, you'll find any logging the driver outputs in the kernel logs. You can also use the dmesg binary on the system to get colored output and access to a several other useful options.

### LNAV

[lnav (The Logfile Navigator)](https://lnav.org/) is one of the most useful utilities I've ever found for reading Android logs, though out of the box, lnav doesn’t support Android’s logcat format but a json scheme for logcat can be found [here](https://github.com/phoenixuprising/lnav-android-scheme). It’s a generic, but feature rich log navigator on the CLI which supports multi-log interpolation. This means you can combine both the kernel and logcat logs into a single log viewer and understanding what’s happening at both layers of the system. When working at Square, I worked on a product which was two discrete Android devices talking to each other over USB. I wrote a script to add a device identifier to each set of logs after the time stamp and was able to combine all 4 sets of log files into one view, thus debugging the entire product as a whole. It also supports things like syntax highlighting, custom regex highlighting, and pretty printing of structured data such as json.

## Crash Files & Types

Along with the two types of logs, there are also a handful of crash files and types that are recorded when exceptions are thrown.

### Tombstones

When a dynamically linked executable starts, several signal handlers are registered that, in the event of a crash, cause a basic crash dump to be written to logcat and a more detailed tombstone file to be written to `/data/tombstones/` and are sent to DropBoxManager. The tombstones files contain a wealth of information about the crash including:
  - GNU Build Fingerprint
  - ABI
  - Name of the crashing process
  - Stack traces for all the threads in the crashing process
  - Full memory map
  - List of all open file descriptors
  - What signal handler was triggered

A truncated example would look like:
```
*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
Build fingerprint: 'Android/aosp_angler/angler:7.1.1/NYC/enh12211018:eng/test-keys'
Revision: '0'
ABI: 'arm'
pid: 17946, tid: 17949, name: crasher  >>> crasher <<<
signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0xc
    r0 0000000c  r1 00000000  r2 00000000  r3 00000000
    r4 00000000  r5 0000000c  r6 eccdd920  r7 00000078
    r8 0000461a  r9 ffc78c19  sl ab209441  fp fffff924
    ip ed01b834  sp eccdd800  lr ecfa9a1f  pc ecfd693e  cpsr 600e0030

backtrace:
    #00 pc 0004793e  /system/lib/libc.so (pthread_mutex_lock+1)
    #01 pc 0001aa1b  /system/lib/libc.so (readdir+10)
    #02 pc 00001b91  /system/xbin/crasher (readdir_null+20)
    #03 pc 0000184b  /system/xbin/crasher (do_action+978)
    #04 pc 00001459  /system/xbin/crasher (thread_callback+24)
    #05 pc 00047317  /system/lib/libc.so (_ZL15__pthread_startPv+22)
    #06 pc 0001a7e5  /system/lib/libc.so (__start_thread+34)
Tombstone written to: /data/tombstones/tombstone_06
```

You'll notice that the backtrace is not symbolicated. Within a standard AOSP source tree, you will find the `stack` binary that can be used to symbolication which will provide specific line numbers for the backtrace using the debug symbols from the compilation process.

`$ development/scripts/stack < /data/tombstones/tombstone_05`

```
Reading symbols from /huge-ssd/aosp-arm64/out/target/product/angler/symbols
Revision: '0'
pid: 17946, tid: 17949, name: crasher  >>> crasher <<<
signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0xc
     r0 0000000c  r1 00000000  r2 00000000  r3 00000000
     r4 00000000  r5 0000000c  r6 eccdd920  r7 00000078
     r8 0000461a  r9 ffc78c19  sl ab209441  fp fffff924
     ip ed01b834  sp eccdd800  lr ecfa9a1f  pc ecfd693e  cpsr 600e0030
Using arm toolchain from: /huge-ssd/aosp-arm64/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/

Stack Trace:
  RELADDR   FUNCTION                   FILE:LINE
  0004793e  pthread_mutex_lock+2       bionic/libc/bionic/pthread_mutex.cpp:515
  v------>  ScopedPthreadMutexLocker   bionic/libc/private/ScopedPthreadMutexLocker.h:27
  0001aa1b  readdir+10                 bionic/libc/bionic/dirent.cpp:120
  00001b91  readdir_null+20            system/core/debuggerd/crasher.cpp:131
  0000184b  do_action+978              system/core/debuggerd/crasher.cpp:228
  00001459  thread_callback+24         system/core/debuggerd/crasher.cpp:90
  00047317  __pthread_start(void*)+22  bionic/libc/bionic/pthread_create.cpp:202 (discriminator 1)
  0001a7e5  __start_thread+34          bionic/libc/bionic/clone.cpp:46 (discriminator 1)
```

### Kernel Panics

A Kernel Panic is a critical and unrecoverable error that occurs within the Linux kernel, the core component of the Linux operating system. When a kernel panic happens, the kernel detects a situation where it cannot safely continue executing, and as a result, it halts all operations to prevent further damage or data corruption to the running device. It's a last-resort safety mechanism to prevent the system from continuing to execute in an unpredictable or unstable state. Kernel panics will be found in the kmsg buffer and will also be written to the Android DropBoxManager for analysis once the system comes back up. 

### Kernel Oops

A Kernel Oops on the other hand, is a non-fatal error, but can still proceed a Kernel Panic. That said, Android can be configured to trigger a Panic anytime an Oops occurs. This follows the "crash fast" mentality that is often used to build reliable systems. You can check to see if this feature is enabled at runtime by checking the value of `/proc/sys/kernel/panic_on_oops`. If it's not enabled, you can write a `1` to the file descriptor in an init script or by setting the `CONFIG_PANIC_ON_OOPS` kconfig.  

### Anatomy of Kernel Panics & Oops

The files produced by a Kernel Panic or Oops can be quite long so we will look at specific parts of them.

```
isPrevious: true
Build: OnePlus/OnePlus6/OnePlus6:8.1.0/OPM1.171019.011/06140300:user/release-keys
Hardware: msm8916
Revision: 0
Bootloader: unknown
Radio: null
Kernel: Linux version 4.9.113 (leo@LeoHomePC) (gcc version 6.3.1 20170109 (Linaro GCC 6.3-2017.02) ) #3 SMP PREEMPT Wed Mar 24 17:23:14 CDT 2021
```

First, we have a header that provides detailed information about the build fingerprint, hardware used, and specifics of the kernel. This information is important to look through to make sure when you are using the correct copy of source code that is associated with the exception.

```
[[TRUNCATED]]
84] Detected VIPT I-cache on CPU3
[ 1413.286424] CPU3: Booted secondary processor 0x0000000103 [0x410fd034]
[ 1413.287278] CPU3 is up
[ 1413.287719] Detected VIPT I-cache on CPU4
[ 1413.287814] CPU4: Booted secondary processor 0x0000000000 [0x410fd034]
...
[ 1461.452727] usb 1-1: reset high-speed USB device number 2 using ci_hdrc
[ 1461.631038] usb 1-1.3: reset full-speed USB device number 3 using ci_hdrc
[ 1461.783350] OOM killer enabled.
[ 1461.790291] Restarting tasks ... done.
```

We then will have a truncated kmsg log that will indicate what the system was doing leading up to the exception. 

```
[ 1462.134541] Internal error: Oops: 96000044 [#1] PREEMPT SMP
[ 1462.134545] Modules linked in: wcn36xx btqcomsmd btqca wcnss_ctrl rfcomm bluetooth ecdh_generic virtio_crypto crypto_engine qcom_wcnss_pil qcom_common lm75 crqgt4_i2c
[ 1462.184916] Process swapper/3 (pid: 0, stack limit = 0x00000000226d4865)
[ 1462.184925] CPU: 3 PID: 0 Comm: swapper/3 Not tainted 4.19.235-g5d431ab0cc0d-abe9d6ac6 #1
[ 1462.184927] Hardware name: Memfault
[ 1462.184934] pstate: 00400085 (nzcv daIf +PAN -UAO)
[ 1462.184949] pc : queued_spin_lock_slowpath+0x104/0x284
[ 1462.184957] lr : _raw_spin_lock+0x48/0x50
[ 1462.184958] sp : ffff00000801bd70
[ 1462.184961] x29: ffff00000801bd80 x28: ffff80007df0d068 
[ 1462.184966] x27: ffff80007df0d040 x26: 000001546d61b824 
[ 1462.184970] x25: ffff80007df0d050 x24: ffff0000081412ec 
[ 1462.184975] x23: ffff000008cf0e80 x22: ffff000008e1db50 
[ 1462.184979] x21: 0000000000000003 x20: 00610062003a0072 
[ 1462.184983] x19: ffff80006cd0fe80 x18: 0000000081005000 
[ 1462.184988] x17: 000000007571732f x16: 0000000000000000 
[ 1462.184992] x15: ffff000008e1db50 x14: ffff0000089e3378 
[ 1462.184997] x13: ffff000008cf1ad0 x12: 0000000000100000 
[ 1462.185001] x11: ffff80007df10ac8 x10: ffff80007df10ac0 
[ 1462.185005] x9 : ffff000008cf1ac0 x8 : ffff80006cd0fe80 
[ 1462.185010] x7 : ffff000013203c70 x6 : ffff000013203be4 
[ 1462.185014] x5 : ffff000013203c64 x4 : 0000000000000004 
[ 1462.185018] x3 : 0000000000000001 x2 : 0000000000000001 
[ 1462.185022] x1 : 000000007571732f x0 : ffff80006cd0fe80 
[ 1462.185027] Call trace:
[ 1462.185033]  queued_spin_lock_slowpath+0x104/0x284
[ 1462.185039]  scheduler_tick+0x48/0x100
[ 1462.185045]  update_process_times+0x80/0x98
[ 1462.185052]  tick_sched_timer+0x90/0xf0
[ 1462.185056]  __hrtimer_run_queues+0x118/0x1bc
[ 1462.185060]  hrtimer_interrupt+0xf0/0x3e0
[ 1462.185067]  arch_timer_handler_virt+0x34/0x40
[ 1462.185073]  handle_percpu_devid_irq+0x7c/0x154
[ 1462.185077]  __handle_domain_irq+0x7c/0xbc
[ 1462.185082]  gic_handle_irq+0x4c/0xb8
[ 1462.185086]  el1_irq+0xe8/0x190
[ 1462.185091]  arch_cpu_idle+0x10/0x18
[ 1462.185097]  do_idle+0xbc/0x268
[ 1462.185101]  cpu_startup_entry+0x20/0x24
[ 1462.185108]  secondary_start_kernel+0x14c/0x158
[ 1462.185115] Code: 8b2e4dee 120005ad f85f81ce 8b2d512d (f82d69ca) 
[ 1462.185119] ---[ end trace 9aea57dd6aa91794 ]---
[ 1462.235526] Kernel panic - not syncing: Fatal exception in interrupt
[ 1462.235539] SMP: stopping secondary CPUs
[ 1462.235550] Kernel Offset: disabled
[ 1462.235557] CPU features: 0x0,24002004
[ 1462.235560] Memory Limit: none
[ 1462.492479] Rebooting in 5 seconds..
```

Finally, we have the actual crash itself. Detailed information about the process that crashed is provided such as the name of the binary, what CPU it was running on, the call trace, and register values. This can all be incredibly helpful in understand what is happening in the kernel at the time of the panic. You'll notice here as well that call trace has memory offsets for each of the function calls. With the use of `addr2line` ([man page](https://man7.org/linux/man-pages/man1/addr2line.1.html)), you are able to get the filename and line numbers associated with those offsets.

### Application Not Responding (ANRs)

Any time the UI thread of an Android Application is excessively blocked, Android will trigger an ANR. This presents a dialogue in the foreground allowing the user to forcibly close the application.

An ANR is triggered for your app when one of the following conditions occur:

 - Input dispatching timed out: If your app has not responded to an input event (such as key press or screen touch) within 5 seconds.
 - Executing service: If a service declared by your app cannot finish executing Service.onCreate() and Service.onStartCommand()/Service.onBind() within a few seconds.
 - Service.startForeground() not called: If your app uses Context.startForegroundService() to start a new service in the foreground, but the service then does not call startForeground() within 5 seconds.
 - Broadcast of intent: If a BroadcastReceiver hasn't finished executing within a set amount of time. If the app has any activity in the foreground, this timeout is 5 seconds.

 Of these conditions, the most common cause is when the input dispatching has timed out, which in turn is generally caused by network or file io being done on the main thread. Both of these kinds of work should *always* be done on background threads.

 Please refer to [Android's official documentation](https://developer.android.com/topic/performance/vitals/anr) for more details.

### Java Exceptions

This is the most common type of crash that most Android developers are accustomed to in Android apps and services. In Java, and like many other languages, an exception is an event that occurs during the execution of a program, disrupting the normal flow of instructions. It indicates that something unexpected or erroneous has happened, preventing the program from continuing its normal execution. If you're looking for more information about reading and understanding Java based exceptions, I highly recommend reading [Twilio's excellent blog post](https://www.twilio.com/blog/how-to-read-and-understand-a-java-stacktrace) that goes into great depth about them.

### WTFs

[What Terrible Failures](https://developer.android.com/reference/android/util/Log.html#wtf(java.lang.String,%20java.lang.String)) according to Google should be used to:

> Report a condition that should never happen. The error will always be logged at level ASSERT with the call stack. Depending on system configuration, a report may be added to the DropBoxManager and/or the process may be terminated immediately with an error dialog.

These are logged in Android apps and services throughout the system to record things that as the documentation says, should never happen but we all know that things that should never happen on our systems do still happen, and that's the perfect time to trigger a WTF.

### SeLinux Violations

Android's SELinux (Security-Enhanced Linux) is an implementation of the SELinux framework specifically created by the National Security Agency (NSA). SELinux is used on Android devices to enhance security by enforcing mandatory access controls and ensuring that processes and applications operate within defined security contexts. Android's implementation of SELinux is designed to provide a robust security mechanism to mitigate risks associated with malware, unauthorized access, and other security threats. When incorrectly configured, violations will be logged to logcat via `auditd`. Violations can lead to crashes when the resources attempting to be accessed are protected. One important detail to remember is that in engineering builds, SSLinux is configured to be permissive and will only log a warning but will still allow access to the resource being requested. This can make it easy to miss violations during development. 

You can find more details here in the [official documentation](https://source.android.com/docs/security/features/selinux/).

## Diagnostic Tools

Due to the complexity of Android based devices, Google ships several tools to help diagnose what is happening on them.

### Bug Reports

Bug Reports are huge dumps of data from the device that collects information from all the services running on the device and creates a snapshot of the device at the time it was triggered. These are very resource intensive and often times will cause the device to appear frozen so you don’t normally want to do this frequently to a customer’s device.

Android's official documentation has dozens of short explanations on exploring different types of data contained within a [Bug Report](https://source.android.com/docs/core/tests/debug/read-bug-reports).

There are two ways to generate a Bug Report. First option is to use Android Debug Bridge (which is discussed later in this article) to connect to the device and request one. `adb bugreport bugreport.zip`, this command will generate a bug report in the form of a ZIP archive and write it to the host machine. The second way is via the "Developer options" in the settings of your Android device. You will find and an option called "Take bug report" which will generate one locally on the device, it is then up to the user of the device to upload the Bug Report somewhere for developers to read over.

### DropBoxManager

DropBoxManager is for app specific crash logs. As mentioned above, this is where java exceptions, tombstones, kernel panics, etc can be found as individual files as opposed to having to try and pull them out of logcat. [Android provides API's](https://developer.android.com/reference/android/os/DropBoxManager) to both write and retrieve the files sent to the DropBoxManager.

### Batterystats and Battery Historian

The Android documentation describes Batterystats as "a tool included in the Android framework that collects battery data on your device". After using the device for a bit, you can run `adb shell dumpsys batterystats > batterystats.txt` to dump the stats to a file. This is where Battery Historian comes into play. Battery Historian is a webapp that you can run locally to visualize the data created by Batterystats.

To explore more information about how to use Batterystats and Battery Historian, visit Google's [documentation](https://developer.android.com/topic/performance/power/setup-battery-historian#BatteryHistorianCharts).

While it is an older video, theres a great [YouTube video](https://www.youtube.com/watch?v=i3pw5aCAvOc) showing how to explore the visualizations created by Battery Historian.

### Perfetto

Perfetto is an open-source performance tracing and visualization tool primarily designed for analyzing and improving the performance of software systems, especially on the Android platform. It was initially developed by Google and is now maintained as a collaborative project by the Perfetto community.

The primary goal of Perfetto is to provide deep insights into the performance characteristics of software applications, operating systems, and hardware. It achieves this by collecting and visualizing various performance-related data, such as CPU utilization, memory usage, disk activity, network activity, and more. This data helps developers and system administrators identify performance bottlenecks, track down issues, and optimize their software for better efficiency and responsiveness.

### Android Debug Bridge (ADB) 

ADB is one of the most important tools available to both embedded AOSP developers and app developers alike. It allows you to connect to your device over usb or the network, run commands, pull logs, install apps, reboot the device, and push/pull files.

[Official Documentation](https://android.googlesource.com/platform/packages/modules/adb/+/refs/heads/master/docs/user/adb.1.md) goes into great depth of all of the capabilities of the tool and how to use it.

## Conclusion

Android devices are complex. There's thousands of different ways they can fail throughout their lifetime. Hopefully after reading through this article, you will have a better sense of where to find the information necessary to fix the bugs seen on your devices.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}
