---
title: Matter, Thread, and Memfault
description:
  Matter and Thread are changing the smart home landscape with IPv6-based
  connectivity. This post shows how to use Memfault in a Matter/Thread
  environment - the changes required to the Memfault SDK, the required set-up
  and ideas for future integration of Memfault with Thread.
author: mabe
tags: [matter, thread, memfault, nordic]
---

<!-- excerpt start -->

I'm Markus, software engineer @ [Tridonic](https://tridonic.com), where we are working on [Internet-connected wireless lighting solutions](http://www.tridonic.com/matter) based on the [Matter standard](https://csa-iot.org/all-solutions/matter/). To be able to monitor the reliability of those devices we've been using Memfault and tied it into Matter/Thread and its UDP/IPv6 stack based on the Nordic Connect SDK. In the following, I'll show you the modifications we've done to enable Memfault in an IPv6 solution.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Matter & Thread

[Matter](https://csa-iot.org/all-solutions/matter/) is an interoperability standard for IoT devices from the [connectivity standards alliance (csa)](https://csa-iot.org) previously known as the Zigbee Alliance. The Matter standard is implemented in the open-source reference implementation in the [GitHub project-chip repository](https://github.com/project-chip/connectedhomeip/). Matter currently uses UDP on top of IP(v6) and can run on top of different lower layers: Ethernet, WiFi, Thread/802.15.4. [Thread](https://www.threadgroup.org/) is a low-power IPv6 connectivity standard. Its reference implementation is available in the [openThread Github repository](https://github.com/openthread/openthread). The Matter SDK as well as the openThread SDK are also available bundled in the [Nordic Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/).

At Tridonic, we view Matter as the unifying force in both the Smart Home industry and the professional lighting space. It consolidates wireless options and provides new opportunities to utilize the consolidated wireless ecosystem for the benefit of users. Thread has been chosen for its suitability in supporting low-power sensor devices and its efficient handling of multicast to numerous wireless devices.

The Tridonic products are therefore based on matter/Thread/802.15.4. Thus I'll focus on UDP, IPv6, and Thread in the article.

## Methods to connect a Matter/Thread device to Memfault

There is a lot of flexibility that comes with IP. This gives us multiple options to send data to Memfault.

1. Directly via HTTP & TCP
    1. via IPv6
    2. via IPv4
2. via a Memfault UDP relay
    1. via a public Memfault UDP relay
        1. DNS entry for a Memfault UDP relay <memfault-udp-relay.example.com> (if required via NAT64)
    2. via a local Memfault UDP relay
        1. via an announcement using multicast DNS service discovery
        2. via a well-known IPv6 multicast address, e.g.<ff05::f417> (shortened hexspeak for Memfault)

In the following, we'll discuss the various options.

### Via HTTP & TCP

TCP is currently an option for non-Thread Matter devices. TCP is being discussed and implemented also for Thread but might have an impact on the code size.

This could be enabled with

```kconfig
CONFIG_OPENTHREAD_TCP_ENABLE=y
CONFIG_HTTP_CLIENT=y
```

Since this is not enabled by default for our Matter / Thread firmware right now, we'll skip this option.

#### Via TCP via IPv6

Using IPv6 is at least for now not an option, since is not yet supported directly by Memfault <https://support.memfault.com/hc/en-us/articles/7568953852317-Does-Memfault-support-receiving-data-via-IPv6->.

#### Via TCP via IPv4 and NAT64

In the meantime, you can work around it by using NAT64 and connect using IPv4.

Using the Zephyr shell you could check this option:

```shell
> dns resolve4 api.memfault.com
> tcp connect <NAT64 address> 20001
> tcp send hello
```

Since TCP is not enabled by default in our firmware,  we'll focus on the UDP options.

### Via UDP

Memfault does not support the delivery via UDP directly, but you can use a UDP relay as described in <https://docs.memfault.com/docs/mcu/nrf-connect-sdk-guide/#sending-data-to-memfault-over-udp>.

#### Via public UDP relay via IPv6

If the UDP relay is reachable via IPv6 use the following commands:

```shell
> dns resolve memfaultudprelay.example.com
> udp send <IPv6 address> 20001 hello
```

#### Via public UDP relay via IPv4/NAT64

If the UDP relay is only reachable via IPv4:

```shell
> dns resolve4 memfaultudprelay.example.com
> udp send <NAT64 address> 20001 hello
```

#### Via mDNS

Another mechanism could be to distribute this service to the devices via multicast DNS service discovery which is also used by Matter devices.

E.g. on a machine one could advertise a Memfault service:

```shell
dns-sd -R MEMFUDP _memfault._udp . 20001
# or
avahi-publish-service ...
avahi-publish -s MEMFUDP _memfault._udp 20001
```

Or announce it via the Border Router:

```shell
> srp client host name BR_MFLT_UDP
> srp client host address ML-EID
> srp client service add otbr1 _memfault._udp 20001
```

This could be tested on the device:

```shell
> dns browse _memfault._udp.default.service.arpa
> udp send <HostAddress> 20001 hello
```

#### Via multicast

We have not been using and implementing any of the options from above but instead decided to send the Memfault reports to a dedicated IPv6 multicast address. The UDP relay is listening on IPv6 and joins that multicast group.

This mechanism requires very little setup on the customer premise and very implementation in the device. If the customer has no UDP relay, no data is transferred - which is good from a privacy point of view. However, if a customer has a complaint error reporting can easily be enabled.

On the device shell, this could be tested with:

```shell
> udp send ff05::f417 20001 hello
```

This approach is described in more detail in the following sections.

## Getting to a working Matter/Memfault solution

Memfault offers a [UDP relay sample](https://github.com/memfault/memfault-nRF9160-relay/tree/main/server). We'll use this relay and adapt it.

Additionally, we'll change the default Memfault MCU SDK (respectively Nordic's integration of it) and adapt it to IPv6.

### Firmware

Nordic provides a [Matter light bulb sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/matter/light_bulb/README.html) as part of their SDK which we'll use as a base for this investigation and compile it according to the provided instructions.

Following [Memfault's MCU nRF Connect SDK Guide](https://docs.memfault.com/docs/mcu/nrf-connect-sdk-guide/), we'll take the example code and combine it with the Matter light bulb sample.

The final code can be found in <https://github.com/markus-becker-tridonic-com/matter-thread-memfault> and <https://github.com/memfault/interrupt/tree/master/example/matter-thread>.

#### Add Memfault to the Matter sample

The files `memfault_udp.cpp`/`.h` files have been added. They are based on the code in <https://github.com/memfault/memfault-nRF9160-relay/blob/main/firmware/src/main.c> with some modifications.

The key change is in `memfault_server_init()` to use IPv6 instead of IPv4:

```c
  struct sockaddr_in6 *server6 = ((struct sockaddr_in6 *)&host_addr);

  server6->sin6_family = AF_INET6;
  server6->sin6_port = htons(CONFIG_UDP_SERVER_PORT);

  zsock_inet_pton(AF_INET6, CONFIG_UDP_SERVER_ADDRESS_STATIC, &server6->sin6_addr);
```

In `CMakeLists.txt` the file needs to be added to the build:

```cmake
  if(CONFIG_MEMFAULT)
    target_sources(app PRIVATE src/memfault_udp.c)
  endif()
```

In `Kconfig` the "Memfault UDP Sample Settings" have been added.

Also, the Memfault initialization has been added in the function `AppTask::StartApp()` in `app_task.cpp`:

```c
  LOG_INF("Memfault over UDP sample has started\n");

  int memerr = memfault_server_init();
  if (memerr)
  {
    LOG_ERR("Failed to initialize UDP server connection");
  }

  memerr = memfault_server_connect();
  if (memerr)
  {
    LOG_ERR("Failed to connect to UDP server");
  }
```

Once the device is connected to the Thread network the `AppTask::ChipEventHandler()` is called with `DeviceEventType::kDnssdPlatformInitialized` and `sIsNetworkProvisioned` and `sIsNetworkEnabled` are true, the device can send the Memfault chunks to the multicast IPv6 address.

```c
  if (sIsNetworkProvisioned && sIsNetworkEnabled) {
    LOG_INF("Connected now. Schedule Memfault sending");
    memfault_init_udp_message();
    init_memfault_chunks_sender();
    memfault_schedule();
  }
```

Additionally, the Memfault message definitions have been added in `config/`.

In `prj.conf` several changes have been made:

* Several options have been set to make the Matter build smaller so that Memfault could be added.
* The Memfault config options have been added.
  * `CONFIG_UDP_SERVER_ADDRESS_STATIC` is set to the IPv6 multicast address `ff05::f417` (NOTE: shortened hex-speak of memFAuLT).
  * Change `CONFIG_MEMFAULT_NCS_PROJECT_KEY` with a key you can get from [Memfault Cloud - Settings - General - Project Key](https://mflt.io/project-key).

### Memfault UDP relay

Minor modifications of <https://github.com/memfault/memfault-nRF9160-relay/tree/main/server> were required for IPv6 and the modified file is located in `server/` of <https://github.com/markus-becker-tridonic-com/matter-thread-memfault>.

* The script has been changed to use an IPv6 socket.
* The script joins the IPv6 multicast group on the relevant interface (here `utun3`, adapt to your interface name).

```python
INTERFACE = 'utun3'
LOCAL_PORT = 20001
BUFFER_SIZE = 508 + 28

MEMFAULT_IPV6_GROUP_6 = 'ff05::f417'

addrinfo = socket.getaddrinfo(MEMFAULT_IPV6_GROUP_6, None)[0]

print(addrinfo)

UDPServerSocket = socket.socket(addrinfo[0], type=socket.SOCK_DGRAM)
UDPServerSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
UDPServerSocket.bind(('', LOCAL_PORT))

group_bin = socket.inet_pton(addrinfo[0], addrinfo[4][0])
mreq = group_bin + struct.pack('@I', socket.if_nametoindex(INTERFACE))
UDPServerSocket.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

print(f"Listening on interface {INTERFACE} port {LOCAL_PORT}")
```

### Build the light bulb sample

You can build the light bulb sample according to [Nordic's documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/samples/matter/light_bulb/README.html).

## Run Border Router and Commission Matter device

Set up a border router and commission the Matter light bulb according to the [Nordic Matter documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/protocols/matter/index.html).

## Run the UDP relay

Execute the relay:

```shell
cd server
python3 __main__.py
```

## Trigger a fault with the mflt CLI on the Nordic DK

Connect to the UART shell on the DK with e.g. putty and issue e.g. `mflt test hardfault` or other faults from <https://docs.memfault.com/docs/mcu/zephyr-guide#testing-the-integration>.

```shell
> mflt test hardfault
```

Observe the transmission of the UDP packets to the UDP relay. Observe the UDP relay forwarding the information to the Memfault cloud. And then see the chunks arriving in the [Memfault cloud - Chunks Debug](https://mflt.io/chunks-debug).

## Conclusion

I hope this write-up helps other developers interested in Matter to integrate Memfault more easily. If you have suggestions, create an [issue](https://github.com/markus-becker-tridonic-com/matter-thread-memfault/issues) or better create a [pull request](https://github.com/markus-becker-tridonic-com/matter-thread-memfault/pulls).

Using Memfault you can also improve the quality of your Matter device firmware and investigate issues during development and testing more easily as well as be able to on-demand investigate issues at a customer.

In the future, a tighter integration of Memfault and Matter can improve the developer experience.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

<!-- prettier-ignore-end -->
