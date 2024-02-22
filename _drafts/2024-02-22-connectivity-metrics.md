---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
#
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
#
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md
#
# It will now show up in the front page when running Jekyll locally.

title: Connecting Monitoring and Observability to Your Devices
description: Measure and monitor device connectivity using techniques such as logging, protocol captures, and metrics like connection uptime and sync success.
author: ericj
---

With the number of wireless SoCs on the market, “Just add connectivity” is finally a reality! “Just” does a lot of lifting in that phrase. Connectivity, whether wired or wireless, adds numerous layers of complexity to your device. Treating your connectivity as a black box early in development is easy, but this strategy will implode when thousands of devices enter the field. It’s not enough to test from end-to-end, pushing data through your device to the cloud. Controlled tests only partially emulate how a device’s connectivity might perform in the field.

It is crucial to thoroughly understand how your device’s connectivity performs, both presently and compared to future releases. Connectivity is the central conduit delivering core data to and from users, whether through a BLE connection to a phone or gateway, or a direct Internet connection with WiFi or LTE. Without reliable connectivity, user happiness and device utility plummet. Device makers must have a handle on this aspect of their product.

<!-- excerpt start -->

This post will cover why connectivity is complex and what methods we have available to diagnose and solve connectivity problems. We’ll survey briefly tools like logging and protocol analysis. Then, we'll look in-depth at the utility of metrics and finally, wrap up with some practical examples to use in your device.  


<!-- excerpt end -->

Too often teams jump into building a connected device without thinking about how to gain insights when the product is in the hands of users and not at their desk. Following these best practices, you'll be tuned into how your connectivity path is operating in the wild.

{% include newsletter.html %}

{% include toc.html %}

## The Fun Nightmares of Connected Devices

At one of my previous companies, we released a BLE hub device to forward data from our BLE-based wearable over WiFi/Ethernet. The hub allowed users to quickly upload their recorded data without requiring a phone to be near these BLE devices during transfer. Many times, customers would power on a new hub only to not be able to connect to a network (unsupported WiFi band) or connect to our backend (port 443 blocked). The only workaround required customers to use the Ethernet port (WiFi interface literally unplugged from inside the housing). Even crossing continents caused issues as the device booted with the incorrect system time. At some point, you will experience some of the fun nightmares of connectivity. Why do these problems exist in the first place, though? Here’s some of what makes connectivity challenging:

- **Complexity**: Connectivity schemes are complex, but for good reason. These protocols have separate layers responsible for security, routing, retransmission, framing, and error detection. This significant cost does bring the benefit of robustness in nearly all situations.
- **Interoperability**: Connectivity requires compatible operation between multiple components and entities. Let’s take the case of BLE. Here, we have to contend with our device’s BLE version, the phone/gateway’s BLE version, the phone/gateway OS-supported BLE features, the version of the device’s BLE service, the client-side compatibility with the service, etc. There’s an unlimited number of combinations to take into account.
- **Environmental Conditions**: One of the most challenging factors is how a device’s environment affects connectivity performance. Depending on the protocol and congestion, this can reduce bandwidth by orders of magnitude or even result in full denial of service. The device environment can manifest in strange intermittent failures or change the operating range of your device.
- **Accessibility**: One of the most frustrating problems is that when you need your device’s data the most, you may not be able to get it! To get insights on connectivity, either you need an out-of-band channel using a different method, or you need to be connected. To get connectivity data, we need connectivity in the first place!

All of these are difficult and challenging problems. I learned so much solving these problems, but I was terrified when a new one would pop up. It was a ton of effort each time and it took far too long for us to get an initial monitoring component in place.

## Techniques for Monitoring and Debugging Connectivity

But fear not! You do not have to enter this world blind. We have several tools at our disposal to tackle issues. As with any monitoring/debugging toolkit, each method complements the others. As problems pop up, it’s best to use a mix of these techniques to find the solution.

### Logging

Logging is a foundational tool for understanding the behavior of embedded devices’ connectivity. Logs summarize what transpires within the connectivity stack by capturing key events and status updates. The following example is from an ESP32S3 connecting to my home wifi network:

```text
I (454) mflt: Data poster task up and running every 60s.
I (494) pp: pp rom version: e7ae62f
I (504) net80211: net80211 rom version: e7ae62f
I (514) wifi:wifi driver task: 3fcb4c08, prio:23, stack:6656, core=0
I (524) wifi:wifi firmware version: 9aad399
I (524) wifi:wifi certification version: v7.0
I (524) wifi:config NVS flash: enabled
I (524) wifi:config nano formating: disabled
I (524) wifi:Init data frame dynamic rx buffer num: 32
I (544) wifi:Init management frame dynamic rx buffer num: 32
I (544) wifi:Init management short buffer num: 32
I (554) wifi:Init dynamic tx buffer num: 32
I (554) wifi:Init static tx FG buffer num: 2
I (554) wifi:Init static rx buffer size: 1600
I (554) wifi:Init static rx buffer num: 10
I (564) wifi:Init dynamic rx buffer num: 32
I (574) wifi_init: rx ba win: 6
I (574) wifi_init: tcpip mbox: 32
I (574) wifi_init: udp mbox: 6
I (574) wifi_init: tcp mbox: 6
I (584) wifi_init: tcp tx win: 5744
I (594) wifi_init: tcp rx win: 5744
I (594) wifi_init: tcp mss: 1440
I (594) wifi_init: WiFi IRAM OP enabled
I (604) wifi_init: WiFi RX IRAM OP enabled
I (614) phy_init: phy_version 601,fe52df4,May 10 2023,17:26:54
```

This first group provides information on the initialization and configuration of the WiFi stack. We get information on what version each component is running, the configurations selected, and memory and kernel object information. The version info is excellent for matching behavior to any known issues. The memory reservation and kernel object counts are especially useful when attempting to profile and understand the stack’s memory usage.

```
I (654) wifi:mode : sta (60:55:f9:f5:2a:d0)
I (664) wifi:enable tsf
I (674) wifi:new:<7,0>, old:<1,0>, ap:<255,255>, sta:<7,0>, prof:1
I (674) wifi:state: init -> auth (b0)
esp32> I (5024) wifi:state: auth -> init (200)
I (5024) wifi:new:<7,0>, old:<7,0>, ap:<255,255>, sta:<7,0>, prof:1
I (7474) wifi:new:<7,0>, old:<7,0>, ap:<255,255>, sta:<7,0>, prof:1
I (7474) wifi:state: init -> auth (b0)
I (8134) wifi:state: auth -> assoc (0)
I (8134) wifi:state: assoc -> run (10)
I (8154) wifi:connected with <my_network>, aid = 9, channel 7, BW20, bssid = <my_routers_mac_addr>
```

Next, the device proceeds and connects with a local WiFi router.

```text
I (8154) wifi:security: WPA3-SAE, phy: bgn, rssi: -49
I (8164) wifi:pm start, type: 1

I (8164) wifi:set rx beacon pti, rx_bcn_pti: 0, bcn_timeout: 0, mt_pti: 25000, mt_time: 10000
I (8184) wifi:<ba-add>idx:0 (ifx:0, <my_routers_mac_addr>), tid:6, ssn:0, winSize:64
I (8184) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (8904) wifi:<ba-add>idx:1 (ifx:0, <my_routers_mac_addr>), tid:5, ssn:2, winSize:64
I (9174) esp_netif_handlers: sta ip: 192.168.50.235, mask: 255.255.255.0, gw: 192.168.50.1
I (9174) wifi station: got ip:192.168.50.235
I (9174) wifi station: connected to ap SSID:<my_network>
I (9874) mflt: Result: 0
I (9874) mflt: Checking for OTA Update
I (10494) mflt: Up to date!
```

Finally, we see the device complete the connection, obtain an IP address, successfully post Memfault data, and check for updates. The collected logs are great for following along with a device; we know the approximate steps and sequence taken during the session.

**Strengths**:

- Easy to implement
- Human readable

**Best Used For**:

- Small numbers of devices
- Additional context when events happen
- Local development

However, logs have many shortcomings, especially when your fleet grows beyond the "co-workers as beta testers" stage. Relying solely on logging will consume valuable time because searching through logs for each device is tedious, brittle, and slow. The Ghost of Engineer’s Past will find and haunt you… in addition to the 50 tickets whose attached logs you have yet to sift through. Check out this post on [logging and heartbeat metrics]({% post_url 2020-09-02-device-heartbeat-metrics %}#transforming-logs-into-metrics) for more info! We'll get to metrics later in this post.

### Capturing Packets With Protocol Analyzers

Protocol analyzers offer a more sophisticated approach to connectivity monitoring, enabling developers to delve deeper into the intricacies of communication protocols. These tools provide detailed insights into packet structure, message sequences, and protocol validation. Protocol analyzers consist of hardware to capture and process the signals and software to visualize and further analyze the captured packets. Dedicated hardware and software can be a worthwhile investment, but these are pricey solutions. Check their capabilities and software compatibility against what you need; you would be surprised how often only older versions of Windows are supported. For 90% of us, some excellent free software and affordable hardware will suffice.

With regards to software, I absolutely must recommend Wireshark. Wireshark is an incredibly powerful tool that captures and analyzes WiFi + networking protocols (IP, TCP, UDP, MQTT, etc), USB, and even BLE. This tool has saved me tons of effort and helped me numerous times. I’ve used Wireshark to find a bug in a BLE stack, profile MQTT connections, and improve my USB throughput, to name a few instances. It’s insane that it’s free, and I’d pay (practically) any price they offered. Here’s an example of a capture I collected to find a problem in my BLE stack:

<p align="center">
  <img width="650" src="{% img_url connectivity-metrics/wireshark-ble-capture.png %}" alt="BLE Packet Capture in Wireshark" />
</p>

For this problem, I observed lower throughput than expected during data transfer. Logs showed that the connection parameters were set correctly initially, but the data rate did not match this. Inspecting the capture in Wireshark led me to discover that two different connection parameter update methods in the BLE stack clobbered values set by the other, resulting in decreased throughput.

Depending on the protocol, you may not need additional hardware. Wireshark can use your [WiFi interface’s monitor or promiscuous mode](https://wiki.wireshark.org/CaptureSetup/WLAN) to capture any signals in the air. It can also hook into your local Ethernet and [USB interfaces](https://wiki.wireshark.org/CaptureSetup/WLAN) for packet capture. For BLE sniffing hardware, I reach for my closest nRF52 dev kit! Nordic Semiconductor offers a [BLE Sniffer package](https://www.nordicsemi.com/Products/Development-tools/nrf-sniffer-for-bluetooth-le/download#infotabs) to download that includes plugins for Wireshark. With these, Wireshark will use your dev kit as an interface to capture packets from.

**Strengths**:

- Incredibly detailed analysis of packets on the wire/over the air
- Great for investigating specific protocol issues

**Best Used For**:

- Single devices
- Local debugging/investigation
- Protocol errors and profiling specific devices

Use protocol analyzers when you need extreme detail for a **single** device at your bench or desk. These tools will not help in the field unless you’re present. They should rarely be used because packet captures are very large and process-intense. Packet captures focus on a single device and can’t be used to aggregate across a growing fleet.

### Metrics

So you’re ready to wrangle thousands, maybe millions of devices out in the wild, all operating in different locations and under different conditions. Not only that, but you’ve got a new release in the works to add a new feature. We know protocol traces aren’t possible without the device next to you, and logs can only provide so much context. The tool to reach for to gather fleet-wide connectivity insights are metrics. Collecting data using counters, timers, and gauge metrics allows you to measure various aspects of connectivity performance, such as packet loss rates, latency, and throughput across your fleet.

<p align="center">
  <img width="650" src="{% img_url connectivity-metrics/metrics-example-1.png %}" alt="Example metric chart showing sync success metrics over time" />
</p>

Looking at this first screenshot, we can see the battery level drop quite drastically in the middle of the chart. This does not correlate with our selected connectivity metrics though.

<p align="center">
  <img width="650" src="{% img_url connectivity-metrics/metrics-exmaple-2.png %}" alt="Example metric chart showing correlation between data transmission and battery percentage drop" />
</p>

**Strengths**:

- Efficient. Most implementations generate data points on the order of bytes.
- Simple. A simple integer to record the value of a metric and a storage structure to buffer data will suffice.
- Aggregate-friendly. It’s easy to apply aggregate functions to a metric across your devices.

**Best Used For**:

- Monitoring and alerting
- Entire fleets or individual devices
- Characterizing the overall performance and state of fleet connectivity

### Attributes

Attributes are used to supplement logging, metrics, and protocol traces. Attributes are bits of additional info related to the device that change infrequently or not at all. These attributes provide more context as to what is unique about the device. Some examples are component vendors, modem firmware versions, gateway/phone OS versions, or geography identifiers. An attribute could help identify a particular battery provider between devices that frequently lose connection due to out-of-spec power output.

<p align="center">
  <img width="650" src="{% img_url connectivity-metrics/wireshark-ble-capture.png %}" alt="BLE Packet Capture in Wireshark" />
</p>

In the screenshot above, I've collected attributes for a thingy91 related to it's modem version and network. If later I see several devices with similar attributes, this may help root cause the issue.

**Strengths**:

- Provide simple metadata to your devices

**Best Used For**:

- Additional context when investigating devices
- Selecting devices with matching values

## Connectivity Metrics In Practice

Once you’ve added an existing metrics implementation or created your own, the next step is figuring out how to instrument your connectivity stack. Some of the best examples apply to many different connectivity setups. To help generalize these metrics for any connectivity type, I will use the word connection to mean a data channel established between the downstream device and the upstream gateway or backend.

### Measuring Time in Connectivity States

Many protocols have different states that devices will transition into and out of during operation. BLE devices start by broadcasting an advertisement packet that a phone or gateway would scan to connect and transfer data with the device. Many IP connections will require a device to obtain a lease on an address when it joins the network. Understanding the time spent in different connection states for LTE devices can drastically impact battery life. To instrument these scenarios, we use a timer metric to measure how long the device spent in a particular state and take the average time to transition into and out of the state.

#### Equation

Avg time spent in state = Total time spent in state / number of state transitions

We can easily observe changes in this value across all of our devices using the average. We can quickly identify if tweaking a connection parameter affected device behavior. Connection-forming states such as the following are critical to measure:

- Connetion-forming states. These are important for measuring latency because they’re overhead on when a device can start sending or receiving data. Your users will notice if this increases significantly!
  - **BLE**: Total time between starting advertising to the connection established or bond formed
  - **LTE**: Total time between idle and connected state
  - **WiFi**: Total time to associate and obtain an IP address

### Connection Time

Just like we can measure device stability with crash-free hours, we should measure our connectivity with a similar measure: connection time. This is best suited for devices that have an  always-on or continuous connection. Instead of an absolute connection time in seconds, we’ll take a ratio between the time we expected to be connected and the actual time we were connected.

#### Equation

Connection Time = Actual Connection Time / Expected Connection Time
Connection Time demonstrates how well our devices can maintain their always-on connection. This metric can also be used when a device enters a state where it knows the connection should be maintained throughout.

### Sync Success Rate

For other devices, they may connect periodically. The metric best suited to measure the overall connection here is looking at each attempt to send/receive data. Each time we attempt a sync, we count either a success or failure. Again, instead of using the absolute counts, let’s take a ratio of success to the total sync attempts (whether successful or not):

#### Equation

Sync Success % = Sync Success Count / (Sync Success Count + Sync Failure Count)

This allows an understanding of how frequently these intermittent connections are succeeding or not. Since we are using a relative measurement, we can easily compare this across different aspects of our fleet without considering the equivalent number of samples.

## Conclusion

A robust monitoring and debugging system for connectivity requires several different tools. Use logging for local debugging and context for what the device was doing. Use granular and aggregated metrics to monitor and observe device or fleet trends. Use detailed protocol traces for specific and low-level problems. Combining these with attributes to identify and categorize devices, the fun nightmares of connectivity become less scary and turn into sweet dreams.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
