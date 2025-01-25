
---
title: Embedded Coroutines
description: Yet another blog about coroutines, this time in embedded
author: Veverak
tags: [c++, coroutines]
---

<!-- excerpt start -->

In the last few years, it seems to me that there was quite a high activity about coroutines in C++.
That is, we got plenty of blog posts/videos on the topic of coroutines that are officially available since C++20.


This is another such blog post, but this time focused more on the embedded environment.
I will try to simplify the part about an explanation of what coroutine is (there is plenty of good blog posts about that), focus more on relevant properties for embedded and provide my input about possible abstractions we can build with coroutines that might be interesting for embedded.
(Given that there is a high chance of few new blog posts appearing as this article was written, it might lose some originality at a certain moment)

<!-- excerpt end -->

The text is structured into two sections:
 - basic introduction to coroutines
 - embedded-related specifics

## Introduction

Coroutines are not something new to the programming world, we are talking about a feature that exists for a long term in various languages.
(Lua had them since 2003, Python mentions them since 2005, and C has various libraries that bring in coroutines)

C++ got low-level language support for the coroutines in version C++20.
The standard does not yet define any useful constructs with coroutines, which makes usage more troublesome.

### Basics

We all should be aware of what `function` is. It is a segment of code that can be executed.
When we `call` a function, the code of the function is executed and at a certain point the `function` will finish its execution and flow returns to the place from which the function was called.

For the sake of this blog post, we will define coroutines as follows:

A coroutine is a segment of code that can be executed.
When we call a coroutine, the process will start the execution of the coroutine code.
The coroutine might suspend itself and return control to its parent.
When suspended, the coroutine can be resumed again to continue the execution of the code.
Eventually, the coroutine will finish its execution and can't be resumed again.

That is, `coroutine` is just a `function` that can interrupt itself and be resumed by the caller.

### How does it look like

The simplest coroutine might look like this:

```cpp
coroutine_type coro(int i)
{
    int j = i*3;
    std::cout << i << std::endl;
    co_await awaiter_type{};
    std::cout << i*2 << std::endl;
    co_await awaiter_type{};
    std::cout << j << std::endl;
}
```

`coro` is a coroutine that interrupts itself on the `co_await` calls and can be resumed to continue the execution.
The exact interface of how that works depends on the `coroutine_type`, but in simple cases, we can assume something like this:

```cpp

coroutine_type ct = coro(2); 
// `2` is printed now
ct.step();
// `4  was printed now
ct.step();
// `6` was printed now

```

In normal use cases, the call of coroutine causes allocation, as all variables that exist between the suspension of the coroutine and its resume (`j`) have to be stored somewhere.
To do that the coroutine allocates memory for its `coroutine frame` which contains all the necessary information for it:
 - `promise_type` (explained later)
 - copy arguments of the coroutine
 - compiler defined holder of all variables of coroutine that survives between the suspension points
(Note: The dynamic memory can be avoided, will be explained later)

The `coroutine_type` is the type of the coroutine, and usually represents the `handle` that points to the allocated frame. (By owning the `std::coroutine_handle<promise_type>` which is a handle given by the compiler for the frame)
To allow data exchange between the coroutine, the coroutine frame contains an instance of `promise_type`, that is accessible from the `coroutine_type` and from the code of the coroutine itself.
(Compiler will select `std::coroutine_traits<coroutine_type>::promise_type` as the promise, this type can be an alias, nested structure, or some other valid type)

`awaiter` is an entity that is used for the suspension process. It is a type that should be passed to the `co_await` call and that is used by the compiler to handle the suspension.
When the coroutine is suspended by the `co_await`, the compiler will call `void awaiter::await_suspend(std::coroutine_handle<promise_type>)` which gets access to the promise via `coroutine_handle` and after that, the coroutine is suspended.
Once the parent of the coroutine resumes it, the `U awaiter::await_resume()` of the awaiter used is called.
The `U` returned by this method is the return value of the `co_await` statement.

The purpose of the `awaiter` is to serve as a customization point that makes it possible to extract data from the coroutine (awaiter can get data in its constructor and pass those to the promise) and also give data to the coroutine by extracting it from promise_type in the await_resume method.

In this context, it is a good idea to point out a few properties of the mechanism:
 - Suspended coroutine can be destroyed. This destruction is safe: all destructors of promise_type, arguments, and stored variables... are called properly
 - The `coroutine_type` does not need to have `step` semantics, the `coroutine_type` has access to `std::coroutine_handle<promise_type>` which provides the interface to resume the coroutine. The `coroutine_handle` might as well be implemented in a way that one method keeps resuming the coroutine until it finishes.
 - Coroutines can be nested. One can combine `coroutine_type` to be also valid `awaiter`, this gives a possibility to have recursive coroutines, in which one `step` of the top coroutine does one step of the inner coroutine.

More detailed description and a good source of truth is cppreference: https://en.cppreference.com/w/cpp/language/coroutines

### Some coroutines

Given that the features exist for some time and have some background from other languages, we can already talk about interesting types of coroutines to work with.
For this, I would like to point out https://github.com/lewissbaker/cppcoro as one of the interesting libraries with coroutines.

The first common concept is the `generator`.
The generator is a coroutine that spits one value at each suspension point which is provided to the caller.
This can be used to generate simple sequences like:

```cpp
generator<int> sequence()
{
    i = 0;
    while(true){
        co_yield i;
        i += 1;
    }
}
```
Here, `co_yield` is just another expression in coroutine API, you can imagine it as a statement with different semantics than `co_await`: `co_await` should wait for awaiter, `co_yield` throws a value to the parent.
Implementation-wise, `co_yield x` causes a call of `promise.yield_value(x)` which constructs awaiter which is awaited.

Generators can have the same API as containers, which gives as the ability to create this nice infinite loop:

```cpp
for(int i : sequence()){
    std::cout << i << std::endl;
}
```

(The idea usually is to either build generators that are not infinite or to eventually stop using the infinite ones)

Another concept of coroutines that can be interesting is: `io coroutines`
That is, we can use a coroutine to represent a process that requires IO operations that are processed during the suspension points.

For example, assume that we have a library for network communication that provides us coroutine to do the io with:

```cpp
network_coroutine send_data(tcp_connection& con)
{
    co_await con.make_data_send_awaiter("How are you?");
    std::string response = co_await con.make_receive_data_awaiter();
    if(response == "good")
    {
        co_await con.make_data_send_awaiter("Good, me too!");
    }
    else
    {
        co_await con.make_data_send_awaiter("Oh, I see");
    }
}
```

It is good to summarize some properties of this coroutine:
 - It does not have to block, once the `co_await` call the parent will get control of the program flow and can just send the data provided by `awaiter` asynchronously.
 - After that, the parent is free to ask another coroutine for data or to destroy this coroutine entirely. This makes sense mostly in cases the request to get data would fail.

### More

This was a fast and simple explanation of coroutines, the purpose of this article is not to give a detailed explanation. I can suggest a blog post such as this one: https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html

## Embedded coroutines

Let's get back to embedded and try to talk about how are coroutines relevant for embedded development.
That means possible problems with coroutines in embedded development, the relevance of coroutines, and some hints for proper usage.

### Relevance

A common task for embedded is to do plenty of IO communication via peripherals and in the meantime to take care of multiple processes in parallel on a single-core system.

That is, we have to handle:
 - complex computations (Matrix computations)
 - IO (send/receive data over UART/I2C/...)
 - long-term processes (PID regulator regulating the temperature of the room)

To handle all those processes at a single point in time, we have two major approaches:
 1) implement it as an iterative process and schedule it
 2) use threads for the processes

Let's take communication over UART as an example:
 1) iterative process means the usage of structure to store the state of the communication, and provide a function that based on the current state advances the state. (Commonly with the usage of an enum representing actual status)
 2) implement the communication in a thread which might release its time if it waits for some operation

Approach 1) has multiple potential issues, it might take a lot of code to implement a complex exchange of data in this way (a lot of state variables) and the approach is problematic, as there is a chance that one of the steps might take longer than expected and we can't prevent that.

Approach 2) has another set of issues. Each thread requires its own stack space (which might not scale), and we got all the problems of parallelism - exchanging data between can suffer various potential concurrency issues.

From this perspective, coroutines are a third way of approaching these processes, which is not better than 1) or 2), but also not worse.
Coroutines bring in a new (and not yet finetuned) mechanism that makes it possible to write an exchange of data over UART in a way that does not require as complex code as 1), does not suffer so many concurrency issues as 2) (and does not need its own stack, just frame). Note that with coroutines you still have to deal with concurrent access to resources, but in more preditacble way.

However, coroutines still suffer from the issue that one step of computation might take longer than expected, and we can assume that any errors in coroutines might be harder to inspect.

What is interesting, is that coroutines have good potential to have more efficient context switches than threads, which was shown in this article: https://www.researchgate.net/publication/339221041_C20_Coroutines_on_Microcontrollers_-What_We_Learned

Note that the article does not compare coroutines in a variety of situations, but it still shows something.

### Problems

There are multiple potential issues with coroutines and using them correctly, but I think there are a few that are noteworthy for the embedded community.

#### Dynamic memory

The first thing that I assume most of you noticed at the beginning is the fact the coroutine requires dynamic memory.
This is not favourable in embedded in many cases, but there are ways to avoid that.

Coroutines have `allocator` support, we can provide the coroutine with an allocator that can be used to get memory for the coroutine and hence avoid the dynamic allocation. (Approach that I can suggest)
This is done by implementing custom `operator new` and `operator delete` on the `promise_type` which allocates the `entire frame`.
Note that you can reuse any allocator implementation for this, or any general implementation that give you a chance to get and release a memory.

An alternative is to rely on `halo` optimization (Heap Allocation Elision Optimization) if the coroutine is implemented correctly and the parent function executes the entire coroutine in its context.
The compiler can optimize away the dynamic allocation and just store the frame on the stack of the coroutine's parent.
This can be enforced by deleting appropriate `operator new` and `operator delete` overloads of the `promise_type`.
The issue with this approach is that you can't control the `halo` optimization, the compiler either decides to do it or does not do it, if it does not happen it might be tricky to find out why and force the compiler to cooperate.

And to kinda ruin the pretty thing here, there is one catch. As of now, the compiler can't tell you how much memory the coroutine needs, as it is known only during the link time - the size of the coroutine frame can't be compiler constant. (TODO: link to the source for this)
This means that you effectively can't prepare static buffers for the coroutines.

What I would suggest (that is what I do), is to use coroutines for the long-term process and just build them during the initialization of the device.
Or just live with the dynamic memory.

(Note: if I could dream a bit here, I would want an explicit way of forcing the coroutine to live on the stack, which would have clearly defined behaviour of when and how it should happen, and with proper compiler errors if one fails to do so)

#### Frame size is big

One more invisible caveat that appeared is that GCC is not yet smart enough with the frame size.
That is, the section of the frame for storing variables that live during the suspension calls is quite big.

The intuitive way to work with the section would be: At each suspension point, we only want to remember variables that survive `that` suspension point and do not care about the rest.

The issue is that currently, GCC is storing all variables for all suspension points there.
This might become problematic if you have a lot of `co_await` statements with various types/interfaces.
(For example: if each `awaiter` stores a buffer of data, there might be separate space for each buffer in the frame)

That might improve in the future, a friend of mine works on a patch for gcc: https://gcc.gnu.org/bugzilla/attachment.cgi?id=53290&action=diff that uses unions of struct, with a separate struct for each suspension point - we pay much less memory.

#### Inspectability

For all that we do on embedded, one of the biggest limitations is our limited ability to inspect what went wrong. Either due to limited resources on the devices or due to the real-time nature of the things.

At this point, `gdb` has support or threads, and mechanisms that use manually written state machines for asynchronous processing are inspectable by default.
But there is no explicit support for coroutines.

By its nature, all variables in the coroutine that survive suspension points form a state.
And yet that state is kinda problematic for us, as other parts of the code can't access it while it is suspended.
(For inspection/logging/data recording...)

In this scenario, I am afraid that it might take a while until our debug tools are smart enough to give an easy life with coroutines in the system. Mostly: what if something went wrong in the coroutine? how do you inspect the state the coroutine is in?

### Suggestions

We talked about what coroutines are, and what are potential problems, now the question is: What can we use them for?

For that, I am afraid it is hard to give a good answer.
The functionality is simply quite new and I suspect that it will take a while (years) until we gain enough experience to be sure in which ways it is beneficial.

That said, based on my (little) experience with the usage of coroutines in embedded, I have some suggestions and patterns that might be interesting.
Mostly to pique the interest of the community in these, as I believe the potential is here.

#### IO Coroutine


I think that the IO example above is the most obvious way we can benefit from coroutines embedded.
That is, we can use coroutines as an abstraction to interact with peripherals that do IO operations.

To describe this, I will use a personal experience with `i2c_coroutine`.
We can design it as follows:

When `i2c_coroutine` is used as the type of the coroutine, the developer can interact with the `i2c` peripheral by `suspending` the coroutine with an `awaiter` that initiates operation on the `i2c` peripheral, the coroutine is resumed once that operation finishes.
That is, we use the suspension point to allow interaction with the peripheral.

For example:

```cpp
i2c_coroutine interact_with_device(uint8_t addr)
{
    std::array<uint8_t, 42> some_data;
    co_await i2c_transmit_awaiter(addr, some_data);

    std::span<const uint8_t> data = co_await i2c_receive_awaiter(addr, 42);
    // do something with data
}
```

This has the benefit that while the `i2c operation` is processed by the peripheral, the main loop can be busy doing something else, and the execution is not stuck in the `interact_with_device` with busy waiting.
And we do not have to pay for that by having complexity (manual state machine doing the interaction) or having threads (which bring in other problems).

Given the bus nature of `i2c`, it is also quite easy to achieve sharing of the `i2c bus` with multiple `device drivers` for various devices on the bus itself.
We can just implement the `i2c_coroutine round_robin_run(std::span<i2c_coroutine> coros)` coroutine that uses round robin to share access to the peripheral between multiple coroutines (devices). Example of RRR:

```cpp
// returns true if all coroutines finished
bool are_all_done(std::span<i2c_coroutine>);

i2c_coroutine round_robin_run(std::span<i2c_coroutine> coros)
{
    std::size_t i = 0;
    while(true){
        i2c_coroutine& coro = coros[i];
        i = (i+1)%coros.size();

        if(coro.done()){
            if(i == 0 && are_all_done(coros)){
                co_return;
            }
            continue;
        }

        co_await coro.tick();
    }
}

```

This can transfer to any interaction with our peripherals, we can model that interaction as a suspension of the coroutine, and let the suspension process initiate the operation with the peripheral, or simply give us a request for some operation.

Note that this has one clear benefit: The coroutine can be completely oblivious to whenever the method of transporting data over peripheral is:
 - blocking
 - using interrupts
 - using DMA

We can also delete any suspended coroutine once we conclude that it is beneficial for us.
In some cases, it might make sense to use this to restart the communication process with the device.
(Note that all variables in the coroutine would be destroyed, that is, it is a good idea to use RAII to revert any effects that the coroutine had on its environment (such as pulling GPIO pin high/low))

I believe there are two variations to how exactly this might be implemented (and I am using both for experimentation):
 - the coroutine throws back a token that represents operation, which is propagated to the peripheral by the APP (for example: `std::variant<no_op, read_request, write_request>`)
 - the awaiter directly interacts with the API of the peripheral

Given my architectural opinions and experience, I tend to prefer the token-based approach, as it feels like a lighter coupling between the peripheral and the coroutine.

However, I've found out that direct interaction has one strong benefit.
 It is much easier to point the peripheral back to the `awaiter`, this way the peripheral might interact with the awaiter during the suspension - a much stronger API.

#### Computing coroutines

One of the biggest advantages of threads over manual decomposition is that some computations are painful to be split into reasonable steps, let's take matrix multiplication as an example:

```cpp
void multiply_matrix(const matrix_t& A, const matrix_t& B, matrix_t& result){
    for(int i = 0; i < A.rows(); i++){
        for(int j = 0; j < B.cols(); j++){
            result[i][j] = 0;
            for(int k = 0; k < A.cols(); k++){
                result[i][j] += A[i][k] + B[k][j];           
            }
        }
    }
}
```

As much as it is feasible to convert this into a stateful object that contains the state of multiplication and hence can do just part of the multiplication at once... The coroutines bring in a more elegant solution:

```cpp
simple_coroutine multiply_matrix(const matrix_t& A, const matrix_t& B, matrix_t& result){
    for(int i = 0; i < A.rows(); i++){
        for(int j = 0; j < B.cols(); j++){
            result[i][j] = 0;
            for(int k = 0; k < A.cols(); k++){
                result[i][j] += A[i][k] + B[k][j];           
            }
            co_await simple_awaiter();
        }
    }
}
```

This is of course not perfect and given the problems with coroutine and dynamic memory, it might be more reasonable to write the `matrix_multiplicator` object.
But problems with dynamic memory of coroutines can be resolved in time, new patterns to handle them can emerge and suddenly something like this might be viable.

## Conclusion

Coroutines are a new feature of C++20 and there is potential for them to be of interest to us.
However, it seems that there is already plenty to be concerned about.
(Unless, of course, I am greatly mistaken in my observations)
