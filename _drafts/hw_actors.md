---
title: The hardware-implemented actor model inside modern MCUs
description: Description of how interrupt controllers may be used as schedulers
author: ramanf
image:
---

<!-- A little bit of background of why the reader should read this post. -->

These days when embedded RTOS-es are huge and bloated, often a simple lightweight framework is needed for embedded devices. Since ARM is the most popular embedded CPU (I mean both microcontrollers like Cortex-M and 'big ARMs') and almost every ARM processor contains internal hardware scheduler, I think it would be interesting to implement a compact framework utilizing these features. As far as I know, the most popular approach to RTOS-less event-driven systems is super-loop consuming events that are produced by interrupts. This article discusses an alternative approach which seems to be more efficient. It has minimal overhead compared to RTOS but also brings some benefits to event-driven systems because of scalability and absence of busy waiting.

<!-- excerpt start -->

This article is intended to describe the alternative approach somewhere in between RTOS and bare-metal no-OS systems.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}


## Hardware scheduler


Do you remember how the 'usual' RTOS scheduler works? It maintains a set of executable entities with some user-defined priority. These entities may activate each other either directly or indirectly. The scheduler takes responsibility that the most prioritized entity will 'run' and activation of other entities will trigger 'rescheduling' and possibly preemption of the currently active one.

If you look at the programmer's model of interrupt controllers like NVIC or GIC, you notice that these devices do exactly that. For example, in the NVIC case, the device contains a special register called STIR (Software Trigger Interrupt Register). Writing a vector number to that register causes corresponding interrupt to be activated exactly the same way as hardware does. Since the user may also adjust interrupt preemption priority, interrupt controller itself may be treated as implementation of the so-called [actor model](https://en.wikipedia.org/wiki/Actor_model) in **hardware**. When you trigger an interrupt its priority management (i.e. delaying when interrupt with greater priority is active) and activation is also performed by hardware. So, necessary ingredients for the hardware-implemented actor model are:

- Ability to trigger interrupts with different vectors programmatically
- Independently adjustable priority for each vector

The approach is to use this model and map priorities to interrupt vectors with user-defined priority. Note that this model contains no threads and runs at a single shared stack. It is inherently asynchronous and even applicable to multiprocessor systems since (in the case of GIC) you may trigger interrupts on other CPUs too.


## The concept


Since the actor model itself is a very complex topic for a single pocket article, instead of talking about abstractions, I mostly focus on their representation in the MCU environment.

In the classic actor model, an actor is just a message handler similar to interrupt handlers. Unlike threads, it has no dedicated stack and its purpose is to react to messages arriving into its internal mailbox.

During message handling, the actor may:
- create new actors
- send messages to other actors
- read/write its local state

Despite the actor and its mailbox are tightly coupled in this model, I think it is less practical than relying on separate queues. The latter case is more flexible since an actor may choose which queue to poll depending on the message that is being processed.

Since there are no threads, the main() purpose is just an initialization. It ends with an infinite loop waiting for interrupts. Note that after initialization completes, there are no other  executable entities except interrupt handlers.

Interrupts are split into two classes: interrupts producing messages and interrupts consuming messages. Also, there are only three types of software objects: queues, actors and messages.

It is expected that interrupts designated to peripheral devices will post messages to queues which in turn have subscribed actors, so, pseudocode for the interrupt handler may be written as:


        device_interrupt_handler() {
            message = alloc_message(pool)
            ... read device registers and set message payload ...
            push_to_queue(queue, message)
        }


Interrupt handlers that are dedicated to actors contain just a call to function, examining a list of active actors with priority corresponding to that vector:


        actor_interrupt_handler() {
            schedule_actors(VECTOR_ID)
        }


On initialization phase, each actor subscribes to some queue. Once message arrives to that queue the actor is activated. Since each actor may be subscribed to exactly one queue, subscription may be implemented as return value: the actor should return a valid pointer to the queue it wants to be subscribed to. Return values may change during an actor’s lifetime but must always be a valid queue. This is because of the absence of internal queue: when an actor has not been subscribed to a queue, it will have no chance to run again. So, pseudocode for an actor is shown below:


        actor(self, message) {
            ... message handling ...
            free(message)
            return queue_to_be_subscribed_to
        }


Queues may contain both messages and actors. If someone posts a message in an empty queue, it will save that message until some actor will try to poll. On the other hand, if an actor tries to get a message from an empty queue it will be saved as a subscriber and later when a message arrives the actor will be activated. Therefore, a queue at any moment of time may contain either messages or actors, but never both. Note that queue is just a head of linked list, it has no internal buffers, so queue overflow cannot happen.

For simplicity, messages are fixed-sized. Message allocator is just a pool of fixed-sized memory chunks. For latency-critical systems, it may be even implemented as lock-free. When the message pointer arrives to an actor, it is assumed that message type is known to its recipient since it knows what queue it polls.


## How it works


As in the case of a thread scheduler, there is some global context containing runqueues, or ready-to-run lists. Each runqueue is mapped to some interrupt vector (unused by devices in that given application). Priorities of such vectors are adjusted to reflect runqueue priority.

When some event (hardware interrupt) occurs, it allocates and posts a message to some queue, this causes activation of the subscribed actor, moving the message into its incoming mailbox, moving actor to the list of ready ones and also triggering interrupt vector corresponding to actor's priority. After leaving the interrupt handler CPU hardware automatically transfers control to the activated vector, its handler then calls framework's function 'schedule', which eventually calls the corresponding actor.
During this process, the system remains fully preemptable and asynchronous: other interrupt handlers may post messages to other queues, if corresponding actors are delayed then messages just will be accumulated in queues. When some actor or interrupt activates another high-priority actor, then preemption occurs immediately, so this system has good response times and less jitter than loop-based solutions.

Normally, the actor model does not require explicit synchronization like semaphores and mutexes because there are no mutable shared state. However, there may be shared resources, such as devices. In such a case mutual exclusion may be represented as multiple actors posting their requests to a single queue and one actor associated with that queue who will do all the work sequentially. Nevertheless, in practice, it is often desirable to block preemption. This may be done using hardware features for temporarily raising a running handler’s priority to some ceiling. Ability to block certain interrupt priorities without disabling interrupts completely also helps with response times since high-priority actors may never be blocked.

Note that the interrupt vector is assigned to priority level, not to an actor, so the total number of actors is unlimited:

![](/img/hw_actors/vectors.jpg)

It is obvious that the number of possible priorities is limited to the number of interrupt vectors and available hardware preemption priorities. Modern controllers have about a thousand vectors with only a few of them actually used in each application, so it seems that there should be no vector shortage. If it is the case, then possibly your application is so complex that full-featured RTOS would be a better option.

Another limitation is vectors available for software triggering. For some controllers like NVIC, any vector may be triggered by both software and hardware, while other devices like GIC contain several vector classes and only one of them is available for software. In GIC, this is just 16 SGIs (software generated interrupts). Note that this limitation does matter only for preemption. If you have 16 vectors with different priorities, it means you have at most 16 actor nesting levels.


## Implementation


The concept described above may be [implemented](https://github.com/romanf-dev/magnesium) in less than 250 lines of C code. Please note that the framework is just an example, and it is not a production ready project yet. It is extremely lightweight compared to RTOS but supports preemption too. Also, its advantage compared to numerous ‘actor model implementations’ from Akka to Erlang is the fact that in the MCU world it is actually hardware-implemented. In the case of fully software implementation, the framework would have to deal with interrupt frames, its patching to emulate preemption, calls for rescheduling on interrupt return, examining bitmaps to find the most prioritized nonempty runqueue, and so on. With hardware implementation, you get all of this functionality for free along with bonuses like tail-chaining, depending on MCU. Drawbacks of hardware implementation is lack of flexibility: there is only one scheduling algorithm, fixed number of priorities, fixed number of available vectors, etc. Any other variant is not possible as it might be in software scheduler implementation.

It is also interesting that the actor model has a direct correspondence with [Petri nets](https://en.wikipedia.org/wiki/Petri_net). If you rename queues to places, actors to transitions and messages to tokens, then any program may be seen as some kind of runnable Petri net. These nets may be analyzed mathematically and some of its properties may be formally proven, such as reachability of some state.


## Pros & cons


Actor model has been around since 1973. It is well suited for event-driven systems and implementation of patterns like the chain of responsibility. In the context of embedded systems, it may help with modularity, testability, power efficiency, and configurability.

It is a common approach to use interrupt handlers as simple 'execution engine' in RTOS-less embedded systems. While it is better than loop-based solutions because of preemption, it may result in worse latencies in case when complex processing is performed in handlers directly. Many devices use interrupts by number of reasons, communication devices for example use them to signal transmission completion, new data receiving, errors and so on. When incoming data is processed in handlers, it blocks responses to other events, and this may be undesirable. Especially in case when communication protocol has timing constraints like USB where host periodically pings devices. So it is a good idea to split interrupt handlers to several independent parts and keep interrupt code as short as possible to be able to react to incoming events appropriately. Described approach may be considered as a way to do such split. Also, it simplifies timing and its use for timed events, and may help with processing events from multiple sources as well as utilization of multiple CPUs.

Communicating actors may be easily put into another environment, like a host computer, for testing purposes. They do not need busy waiting, as in RTOS-less cases when the main program waits for data from interrupt handlers using ring buffers. This helps with both testability and power efficiency. Compared with threads, actors are less vulnerable to issues like unbounded priority inversion (but these issues are still possible!) and allow for better formal reasoning.

However, the article would be incomplete without discussion about drawbacks. The most painful thing is the fact that actors may be deadlocked much easier than threads: any mistake in queue selection and the system will probably hang. Another disadvantage of the model is its requirement to redefine the task in terms of actors and messages, which may be not so easy. For example, just think about how to implement timeouts or waiting for a response before further message processing. Since the actor model relies on message allocations, it is less deterministic compared to fixed-memory solutions, like threads and semaphores. When message pools are exhausted, it is unclear how the handler should deal with it. In other words, design patterns for the actor model are less known and may be more complex than their threading-based counterparts are.

In general, the actor model is a bad choice when you need some kind of global consensus or transactions (i.e., rollback of previous work when some actor at the end of the chain fails). So, while not all systems are suitable for the actor model, it may be a good choice for some and hardware implementation of parts of the model in widely available MCUs makes it yet more attractive.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

- The actor model in 10 minutes:
  <https://www.brianstorti.com/the-actor-model>
- Actor Model Explained:
  <https://finematics.com/actor-model-explained>
- Wikipedia:
  <https://en.wikipedia.org/wiki/Actor_model>
- NVIC operation:
  <https://developer.arm.com/documentation/ddi0403/d/System-Level-Architecture/System-Address-Map/Nested-Vectored-Interrupt-Controller--NVIC/NVIC-operation>
- Using Preemption in Event Driven Systems with a Single Stack:
  <https://www-docs.b-tu.de/fg-betriebssysteme/public/publication/2008/walther-usingpreemption.pdf>

