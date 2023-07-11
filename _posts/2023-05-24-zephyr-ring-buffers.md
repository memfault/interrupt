---
title: "Zephyr Deep Dive: Ring Buffers"
description: "Overview of Zephyr ring buffers covering how they work and when to use them in your design"
author: ericj
tags: [zephyr, rtos]
---

Zephyr includes many built-in features like stacks for networking and BLE, Flash
storage APIs, and many kernel services. These components allow you to quickly
get up and running with a project and maintain less code! Taking advantage of
these is a huge win for small firmware teams and was a huge motivation in
bringing Zephyr to my teams.

<!-- excerpt start -->

This post covers Zephyr's built-in ring buffer API, a component
commonly used in producer-consumer scenarios. We will cover how ring buffers
in Zephyr work, when to use them, and their strengths and weaknesses. This post
will close with an example of augmenting ring buffers with waiting capabilities.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Zephyr Ring Buffers

Ring buffers[^1] in Zephyr provide a way to pass data through a shared memory
buffer. Ring buffers are safely used in single-consumer, single-producer
scenarios. Their usage generally follows the diagram below:

![General usage of a ring buffer in Zephyr](/img/zephyr-ring-buffers/ring-buffers.svg)

The producer writes to the ring buffer via the `ring_buf_put` functions, while
the reader reads from the ring buffer with the `ring_buf_get` functions. Next,
we will explore the two interfaces provided: the bytes-based API and the
items-based API.

### Bytes API

The ring buffer bytes API is often used in driver implementations, where a piece
of hardware needs to send chunks of data up to a higher layer for further
processing. We will look at the `eswifi` driver[^2] as a guide for ring buffer
bytes API operation and walk through some of the driver code to understand its
use. This driver uses AT commands to interface with a WiFi module over a UART.
Let’s start with the initialization code:

```c
struct eswifi_uart_data {
  // ... rest of code
  uint8_t iface_rb_buf[ESWIFI_RING_BUF_SIZE];
  struct ring_buf rx_rb;
};

int eswifi_uart_init(struct eswifi_dev *eswifi)
{
  // ... rest of code
  ring_buf_init(&uart->rx_rb, sizeof(uart->iface_rb_buf),
          uart->iface_rb_buf);
  // ... rest of code
}
```

We can see from this initialization that we have two components to set up. The
structure containing the state of the ring buffer object,
`eswifi_uart_data.rx_rb`, and the underlying shared buffer,
`eswifi_uart_data.iface_rb_buf`. Next, let’s examine how the driver writes to
the ring buffer:

```c
static void eswifi_iface_uart_isr(const struct device *uart_dev,
        void *user_data)
{
struct eswifi_uart_data *uart = &eswifi_uart0; /* Static instance */
  int rx = 0;
  uint8_t *dst;
  uint32_t partial_size = 0;
  uint32_t total_size = 0;

  ARG_UNUSED(user_data);

  while (uart_irq_update(uart->dev) &&
          uart_irq_rx_ready(uart->dev)) {
    if (!partial_size) {
      partial_size = ring_buf_put_claim(&uart->rx_rb, &dst,
                UINT32_MAX);
    }
    if (!partial_size) {
      LOG_ERR("Rx buffer doesn't have enough space");
      eswifi_iface_uart_flush(uart);
      break;
    }

    rx = uart_fifo_read(uart->dev, dst, partial_size);
    if (rx <= 0) {
      continue;
    }

    dst += rx;
    total_size += rx;
    partial_size -= rx;
  }

  ring_buf_put_finish(&uart->rx_rb, total_size);
}
```

When the MCU receives data over the UART from the WiFi module, the UART
peripheral runs the interrupt handler, `eswifi_iface_uart_isr`. Next, the
interrupt handler repeatedly checks for available data. If data is available,
the driver calls `ring_buf_put_claim`. The `put_claim` operation reserves up to
the provided size in the ring buffer for writing and returns the number of bytes
claimed. A return of 0 indicates the ring buffer is full. The write operation
then becomes a simple copy into the shared buffer. After copying the data, the
handler calls `ring_buf_put_finish` to signal completion.

If the ring buffer is full (i.e. the `put_claim` returns 0), the driver drops
the current batch of data received because we cannot block to wait for new data
in this interrupt. Resolving this issue may require increasing the ring buffer
size to account for pressure on the ring buffer. It is difficult to trace the
specific instance when the buffer completely filled. One way to determine the
proper size is to add an assertion when the `put_claim` fails to reserve any
data. Asserting in this manner is a technique of offensive programming[^3], as
this modification proactively looks for an issue and fails purposefully.

The main driver work is done within the context of a dedicated driver workqueue.
The workqueue operates as a dedicated cooperative thread that sleeps until new
work is submitted. In our case, the work starts with requests sent to the WiFi
module and ends with responses sent back. The code below shows how the function,
`eswifi_uart_get_resp` reads data from the ring buffer to parse responses from
the WiFi module:

```c
static int eswifi_uart_get_resp(struct eswifi_uart_data *uart)
{
  uint8_t c;

  while (ring_buf_get(&uart->rx_rb, &c, 1) > 0) {
      LOG_DBG("FSM: %c, RX: 0x%02x : %c",
        get_fsm_char(uart->fsm), c, c);

      if (uart->rx_buf_size > 0) {
        uart->rx_buf[uart->rx_count++] = c;

        if (uart->rx_count == uart->rx_buf_size) {
          return -ENOMEM;
        }
      }
    // ... rest of code
  }
// ... rest of code
}
```

Two things stand out compared to the code that wrote data to the ring buffer.
The first is the usage of `ring_buf_get` instead of the claim-based API. When the
interrupt handler receives UART data, the handler does not immediately know how
much to write into the ring buffer. The claim-based function allows the handler
to use the buffer directly, minimizing copying operations. However, the driver
function, `eswifi_uart_get_resp`, must copy the data for later processing. There
is a bit of an asymmetry by design here. The interrupt handler does not do any
processing on the data. It simply hands the data off to the workqueue and moves
on to the next read. The workqueue, on the other hand, does need to keep this
data around to complete processing the response! The utility of the ring buffer
is that it supports both cases and allows for clear code.

The second thing to note is that the interrupt handler writes in variable
lengths while read operations are a constant length. The ring buffer offers
flexibility in the amount of data being written to or read from the buffer.

### Items API

In addition to the standard bytes-based API, there is the ring buffer items
API. In this version, data is written to and read from the ring buffer as an
item with three components:

1. An application-defined type
2. An application-defined integer value
3. An optional array of associated data

This metadata adds some structure to the data. The type helps identify what the
item contains. The integer value represents either additional metadata or the
value of the item. We can view ring-buffers using this API as better suited to
passing data with some structure rather than a raw array of bytes (e.g.
packet-oriented protocols). For instance, we could parse different items by type
to produce different messages, or route data to different destinations based on
the integer value.

## Ring Buffer Weaknesses

There are a few weaknesses present in ring buffers. First, they require
concurrency protections when their usage expands to multiple producers or
consumers. It is safe to use with a single producer and a single consumer, but
expanding beyond this simple case will require additional design, especially if
used in an interrupt context. The ring buffer does have separate state variables
for reading and writing. These allow for the scenario where an interrupt handler
is the only producer, allowing it to write without acquiring a lock, while
multiple consumer threads must use a lock for exclusive access.

The second weakness is ring buffers lack synchronization capabilities. Producers
cannot wait until space is available in the buffer. Consumers cannot wait until
data is available in the buffer. The ring buffer APIs are non-blocking, they
return immediately. We could use the `ring_buf_is_empty` function to poll for a
change, but this introduces many undesirable effects. A better option would be
to use a kernel synchronization primitive, such as a semaphore, to augment the
ring buffer.

## Augmenting With Wait Capabilities

In the simplest case of a single producer and consumer, the producer can use a
semaphore to signal to the consumer data is available. Let’s take a look at the
`eswifi` driver, but this time at the function that wraps
`eswifi_uart_get_resp`:

```c
static int eswifi_uart_wait_prompt(struct eswifi_uart_data *uart)
{
  unsigned int max_retries = 60 * 1000; /* 1 minute */
  int err;

  while (--max_retries) {
    err = eswifi_uart_get_resp(uart);
    if (err) {
      LOG_DBG("Err: 0x%08x - %d", err, err);
      return err;
    }

    if (uart->fsm == ESWIFI_UART_FSM_END) {
      LOG_DBG("Success!");
      return uart->rx_count;
    }

    /* allow other threads to be scheduled */
    k_sleep(K_MSEC(1));
  }

  LOG_DBG("Timeout");
  return -ETIMEDOUT;
}
```

We can see that this function relies on the response being sent within 60
seconds of the call to wait for the response. In the worst case, this code will
require a context-switch 60,000 times as each call to `k_sleep` allows a ready
thread to take over. Additionally, many short sleeps like this introduce more
jitter than a single timeout. So not only does this cause many more context
switches, but it is also less accurate in terms of timing. A semaphore signaled
from `eswifi_iface_uart_isr` would only require a single switch after timing
out. Here is an example of what this improvement could look like:

```c
// Initialize the semaphore with an initial count of 0 and a max of 1
K_SEM_DEFINE(response_sem, 0, 1);

static void eswifi_iface_uart_isr(const struct device *uart_dev,
        void *user_data)
{
  // ... rest of code
  ring_buf_put_finish(&uart->rx_rb, total_size);

  // Raise signal to eswifi_uart_wait_prompt
  k_sem_give(&response_sem);
}

static int eswifi_uart_wait_prompt(struct eswifi_uart_data *uart) {
  int err;

  // Wait for the UART state machine to reach end state
  // Timeout occurs after 10 seconds since last data received
  while (uart->fsm != ESWIFI_UART_FSM_END) {
    // Returns non-zero value on timeout
    if (k_sem_take(&response_sem, K_SECONDS(10))) {
      return -ETIMEDOUT;
    }

    err = eswifi_uart_get_resp(uart);
    if (err) {
      LOG_DBG("Err: 0x%08x - %d", err, err);
      return err;
    }
  }

  LOG_DBG("Success");
  return uart->rx_count;
}
```

The reworked code has several advantages:

- A more responsive driver when waiting for responses (pun intended)
- `eswifi_uart_wait_prompt` has simpler logic. It fails quickly if it does not
  receive data
- We reduce the number of unnecessary context switches
- The macro, `K_SEM_DEFINE`[^4], prevents annoying race conditions if the kernel
  object has not yet been initialized before the UART interrupt is enabled

The only disadvantage here is a small hit to RAM due to the new semaphore. I
think this is a worthwhile trade-off.

## Wrap-Up

Zephyr has a variety of components that we can combine to suit practically
anything your device requires. The hard part can be knowing where and what to
start with. As we have covered in this post, ring buffers shine when used to
implement single producer single consumer designs. The ring buffer design
introduces little overhead and is efficient in reading and writing through the
use of its internal buffer and indices. The data structure is flexible in
multiple ways. It allows for a variable length of data operations. It offers
multiple data passing methods, whether through direct copy or referencing the
internal buffer. Finally, its API offers either an unstructured bytes-oriented
API or a more structured items API geared toward messages.

This post touches on just one of the many features provided out of the box by
Zephyr. Future posts in this series will examine its other offerings such as
data passing with FIFOs and mailboxes, memory management with slabs and pools,
and subsystems like RTIO and zbus. Please feel free to leave feedback, comments,
and Zephyr topic suggestions below!

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^1]: [Zephyr Ring Buffer API](https://docs.zephyrproject.org/3.3.0/kernel/data_structures/ring_buffers.html)
[^2]: [eswifi driver](https://github.com/zephyrproject-rtos/zephyr/blob/zephyr-v3.3.0/drivers/wifi/eswifi/eswifi_bus_uart.c)
[^3]: [Offensive Programming]({% post_url 2020-12-15-defensive-and-offensive-programming %})
[^4]: [Zephyr Semaphore API](https://docs.zephyrproject.org/3.3.0/kernel/services/synchronization/semaphores.html)
<!-- prettier-ignore-end -->
