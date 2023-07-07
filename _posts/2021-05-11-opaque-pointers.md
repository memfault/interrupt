---
title: "Practical Design Patterns: Opaque Pointers and Objects in C"
description: "An overview of the opaque pointer pattern in C. How to create objects in C."
author: nickmiller
tags: [c]
---

I've written a lot of C++ in my career, but I still prefer to design in
C for most embedded projects ("why" is the subject of a much longer, rant-filled post).
I'm not a big proponent of OOP in general,
but I do think having an "instance" of something which contains stateful data is a generally
useful thing for embedded software. For example, you may want to have several
instances of a ring buffer (aka circular FIFO queue) on your system.
Each instance contains stateful data,
like the current position of read and write pointers. What's the best way
to model this in C?

<!-- excerpt start -->

Objects are not a native concept in C, but you can achieve something
resembling objects by using a design pattern known as the "opaque pointer".
This post will show you what the pattern is, explain some of the finer
details, and provide guidance on when you should and shouldn't use it.

<!-- excerpt end -->

The opaque pointer is one of the most useful and frequently-encountered design
patterns in C.
Even if you are a die-hard C++ proponent (or *gasp* Rust!), the reality is
that there are, and will continue to be, a lot of embedded projects out there
written in C. And you may need to contribute to one of them.

{% include newsletter.html %}

{% include toc.html %}

## The Opaque Pointer Pattern

*opaque: not able to be seen through; not transparent.*

In traditional OOP, an object has state (data) and behavior (functions) coupled together in
one class.

If you want to have something resembling objects in C, you can:

1. Define a struct with your state data
2. Define functions which take a pointer to the struct as the first parameter
3. Declare a variable with that struct type to create an instance of the "object"

Need another object instance? Declare another variable with that struct type.
Boom! Objects in C, baby!

Of course, there's a little more to it than that (but not much).

Before getting into the details, let me show you the complete pattern first, using a ring buffer
that stores `uint8_t` as an example.
This is not a complete ringbuffer implementation.
It's just enough to show you the general pattern.

`ringbuffer.h`

```c
#pragma once

#include <stdint.h>
#include <stdbool.h>

// Opaque pointer type to represent a ringbuffer instance
typedef struct ringbuffer_instance_t* ringbuffer_t;

// Functions that operate on an instance
ringbuffer_t ringbuffer_create(uint32_t capacity);
uint32_t ringbuffer_capacity(ringbuffer_t instance);
bool ringbuffer_enqueue(ringbuffer_t instance, uint8_t item);
bool ringbuffer_dequeue(ringbuffer_t instance, uint8_t* item);
void ringbuffer_destroy(ringbuffer_t instance);
```

`ringbuffer.c`

```c
#include "ringbuffer.h"
#include <stdlib.h>

// Private struct, only accessible from within this file
struct ringbuffer_instance_t {
    volatile uint32_t wr_pos;
    volatile uint32_t rd_pos;
    uint8_t* data;
    uint32_t capacity;
};

ringbuffer_t ringbuffer_create(uint32_t capacity) {
    ringbuffer_t inst = calloc(1, sizeof(struct ringbuffer_instance_t));
    inst->data = calloc(capacity, sizeof(uint8_t));
    inst->capacity = capacity;
    inst->wr_pos = 0;
    inst->rd_pos = 0;
    return inst;
}

uint32_t ringbuffer_capacity(ringbuffer_t instance) {
    return instance->capacity;
}

bool ringbuffer_enqueue(ringbuffer_t instance, uint8_t item) {
    // implementation omitted
    return true;
}

bool ringbuffer_dequeue(ringbuffer_t instance, uint8_t* item) {
    // implementation omitted
    return true;
}

void ringbuffer_destroy(ringbuffer_t instance) {
    if (instance) {
        if (instance->data) {
            free(instance->data);
        }
        free(instance);
    }
}
```

## Object Life Cycle

When you use this pattern, it's customary to declare `*_create`
and `*_destroy` functions. These are analogous to constructors and destructors
in OOP.

The `*_create` function will perform allocation and initialization of an instance and return an
[opaque pointer](https://en.wikipedia.org/wiki/Opaque_pointer)
to the user, which represents the instance. It's really just a pointer to a struct,
but the user can't see what's in the struct. The user's calling code will fail to compile
if it tries to dereference the struct pointer because the struct is not defined in the header
(it is only declared).

Likewise, `*_destroy` will clean up
the instance, making it null-and-void for future use (it would be user error if they try
to use the instance after it's been destroyed).

After the user has created an instance, they call functions which take the instance as the first
parameter. This is analogous to class member functions in C++, which implicitly take a `this`
pointer as the first parameter.

## It's a Handle, Not a Pointer

The opaque pointer typedef is such that it hides the fact that it's a pointer:

```c
typedef struct ringbuffer_instance_t* ringbuffer_t;
```

It's an abstract representation of the instance, often referred to as a
[handle](https://en.wikipedia.org/wiki/Handle_(computing)).

We could just as easily define the handle as a `uint32_t` (maybe it's an index
into an array), and none of the using code would change:

```c
typedef uint32_t ringbuffer_t;
```

Once created, the handle is passed in *by value* to each function.
At the call site, a variable of type `ringbuffer_t` almost looks like a
normal value, like a number:

```c
ringbuffer_t rbuf = ringbuffer_create(10);
ringbuffer_enqueue(rbuf, 0xA5);
```

Using a non-pointer handle type is a stylistic choice, but I think it makes the API easier
to use. When it looks like a normal value, users don't have to use the `*` or `&`
operators. Conversely, if the user sees an API that deals with pointers, there is a small
bit of cognitive overhead and anxiety (questions of ownership, calling mechanics).

## Encapsulation

Because the pointer is opaque, the user's calling code cannot access the data inside the struct.
Why is this a good thing?

* It simplifies the API. The user does not need to know or care about internal data.
* The struct definition can change without impacting any of the using code.
* It prevents misuse of internal struct data.

On the topic of misuse, imagine if the user could see inside the opaque pointer. They might think
they're clever enough to manipulate read and write pointers in the ringbuffer themselves, to get some
kind of custom behavior they're looking for, but most likely, they will end up shooting themselves in
the foot (maybe they forgot to account for wraparound).
Furthermore, it creates hidden coupling in the system. If the ringbuffer implementation
changes the way it uses read and write pointers, it will need to visit all the using code that
accesses the pointers as well. In effect, the ringbuffer implementation would be spread out across
multiple files in the system (good luck finding them all!) instead of being isolated to one file.

By defining the struct in `ringbuffer.c`, the data is "private" to that file
(or more precisely, the
[translation unit](https://en.wikipedia.org/wiki/Translation_unit_(programming))).

What if you have a large module, spread over many `.c` files, that all need
access to the struct? In that case, a good option is to define the struct inside of
a private header file - `ringbuffer_private.h` in the example - which you
can `#include` in other module files. This keeps the public interface in
`ringbuffer.h` clean and minimal, and gives internal module files access to the
private struct data.

## What If I Don't Want to Dynamically Allocate Memory?

There are some reading this that have their spidey sense tingling when
they see the word `calloc` in `ringbuffer_create`.

In many embedded systems, dynamic memory allocation is strongly discouraged
or outright forbidden. Even if it's not forbidden,
it's often a good idea to eliminate dynamic memory if static memory suffices.

In the example, we could statically allocate an array of some maximum number of ringbuffers,
and give the user a `uint32_t` handle, which is an index into the array.

The example code would look slightly different:

`ringbuffer.h`

```c
// ...
typedef uint32_t ringbuffer_t;
// ...
```

`ringbuffer.c`

```c
#include "ringbuffer.h"

#define MAX_NUM_RINGBUFFERS 10
#define MAX_RINGBUFFER_CAPACITY 100

struct ringbuffer_instance_t {
    volatile uint32_t wr_pos;
    volatile uint32_t rd_pos;
    uint8_t data[MAX_RINGBUFFER_CAPACITY];
    uint32_t capacity;
};

static struct ringbuffer_instance_t g_instances[MAX_NUM_RINGBUFFERS];
static bool g_instance_in_use[MAX_NUM_RINGBUFFERS];

ringbuffer_t ringbuffer_create(uint32_t capacity) {
    // Search for an instance that is not in use
    for (uint32_t i = 0; i < MAX_NUM_RINGBUFFERS; i++) {
        if (!g_instance_in_use[i]) {
            g_instance_in_use[i] = true;
            g_instances[i].wr_pos = 0;
            g_instances[i].rd_pos = 0;
            g_instances[i].capacity = capacity;
            return i;
        }
    }

    // No available instances, fail hard here.
    // Alternatively, could return an error code and let the user handle it.
    assert(false);
}

uint32_t ringbuffer_capacity(ringbuffer_t handle) {
    return g_instances[handle].capacity;
}

// ...

void ringbuffer_destroy(ringbuffer_t handle) {
    g_instance_in_use[handle] = false;
}
```

Notice that it's still possible to run out of instances, which is similar to a dynamic
memory allocation failure. The benefit of static memory is that problems like
fragmentation are avoided.
Ultimately, the system designer still needs to think about how to handle failed
instance creation.

One downside to this approach is that it can be wasteful of memory. If you create an
array that is capable of holding 100 instances, and you only ever create one instance,
then you've wasted the memory of 99 instances. Furthermore, for a specific instance, if the
user only needs a capacity of 25, then the other 75 slots are completely unused.

Some might balk at the performance of the linear scan in `ringbuffer_create`.
In practice, I've found it's rarely a concern, especially if you have a small number of instances
(ten or less), or if you rarely call `*_create` (e.g. only once at system startup).

The linear scan could become a concern if you create and destroy instances many times (e.g. inside
of a loop), or if you have a large number of instances, in which case you probably
don't want to pay the `O(n)` price each time. For those cases, a linked list
of available instances is a good approach.

My personal approach - dynamic memory is okay if it's only allocated at system startup,
and not during normal operation.
The implementation is much simpler using dynamic memory.

## When Should I Use This Pattern?

Opaque pointers are a good fit if you need multiple instances. They are also
a good fit for general-purpose libraries, like
[cJSON](https://github.com/DaveGamble/cJSON).

If you only ever need one of something (e.g. a state machine),
then the pattern can be overkill. Since there's just one instance (in OOP, a singleton),
there's no need for handles. The opaque pointer would just be useless overhead.

In my experience, a large portion of C modules do not need multiple instances,
so I use this pattern sparingly.

Once you are comfortable with the pattern, you may be tempted to apply it to every C module
in the system ("We must objectify all the things!"), but this would be a mistake. APIs which
don't have opaque pointers are easier to use than ones that do, so default to not using them.
The less "stuff" that's in the header file, the better.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->
