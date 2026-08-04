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

title: Embedded Linux Logging
description:
 "A look at embedded Linux logging challenges around collecting relevant kernel and user space information for device debugging while balancing flash wear and RAM pressure on resource-constrained devices."
author: grace
---

<!-- excerpt start -->

Embedded Linux offers developers a flexible platform to develop intricate and complex systems. By nature, embedded systems are memory and resource constrained and require more consideration around how to optimize usage of the resources that are available. When it comes to device observability in the field, there are lots of questions around how to balance gathering enough critical information to debug systems without consuming too many resources and harming your overall system performance. This article will walk through various types of embedded Linux logging platforms and consider various tradeoffs between logging storage media, compression, and verbosity to enhance device debugging without eating into your limited resource budget.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Introduction to Embedded Linux Logging 

Log messages are a valuable tool that an engineer has when debugging device issues, and they're typically a good first place to look when triaging a problem. As the complexity and number of devices enter the field, log files start to become cumbersome and hard to filter through to capture meaningful insights. Furthermore, as embedded systems are becoming more sophisticated, the logging architecture and design are playing a more critical role in overall system performance. Simply put, as compute is added more at the edge, log verbosity climbs, and memory writes climb with it. If this is not properly managed, it can lead to overall degradation of your system's performance.

Gathering diagnostic data on your systems involves a set of tradeoffs. If every single event is logged, you will flood your memory with writes and hand the next engineer thousands of lines of logs to dig through to diagnose an issue. Conversely, if you log too little, you are now left with nothing useful to further investigate. These challenges are only amplified when running on resource-constrained devices. For embedded Linux devices, flash wear is a major concern for device longevity in the field. Every log line that reaches persistent storage is a write, and on NAND Flash/eMMC that starts to eat into your flash lifetime. 


## Why Logging Can Stress Your Device

Before diving into specific Linux logging mechanics, it's worth understanding how logging can impact flash wear and degrade device performance.

Most embedded Linux systems use eMMC (Embedded MultiMediaCard) for persistent storage. eMMC consists of a flash controller that manages things like wear leveling and block management, and a NAND flash memory that is exposed as `/dev/mmcblk0`. Unlike other memory storage devices, flash has a finite life, primarily gated by the number of program/erase (P/E) cycles it has before it starts to have unpredictable storage behavior. Simply put, the more you write to your flash, the quicker the flash wears out. To add to the complexity, not all writes to flash memory are the same. How much you write and the size of what you write directly impact the flash wear. This concept is commonly known as write amplification factor (WAF), which is the ratio of physical bytes written versus the logical bytes your application actually needed to write to flash. Unsurprisingly, log messages, which are small, frequent writes by nature, have a very bad WAF.  

Another option is to just write all logs to RAM and prevent inducing any flash wear impact. However, if all logs are stored in RAM, none will persist after a reboot, so you lose critical debug information when a system malfunctions. Embedded devices also have finite amounts of RAM, and writing to RAM is not free. If the logs start to use up too much of your RAM, the kernel is left to try its best to free up memory, which can start to create performance latency in the device. Ultimately, if memory pressure gets too tight, the kernel will escalate to the OOM (out-of-memory) killer to try to kill off processes consuming too much RAM, which can start causing all sorts of unpredictable device behavior. 

A third option, and a happy middle ground between the two storage options, consists of a combination of both. A better-balanced approach to log storage is to consider writing logs to RAM with periodic rotation to flash or, better yet, push the logs up to the cloud. The greatest advantage of this approach is it allows you to limit flash writes and still gaining persistenet storage while not eating into too much of your RAM budget. 

In the end, many of the Linux logging daemons that exist today are tasked to try to balance these various tradeoffs to log and store enough information to be useful, but not more than necessary to preserve critical resources for optimal performance. For every log configuration option, the most important questions to keep in mind are **How much data is stored? How often does this write to physical flash? And how large is each write?** 

## Kernel Logging

The Linux kernel space is where the core operating system runs and has unrestricted access to hardware and memory. Essentially, any log message generated by code running in the kernel space is printed as a kernel message. Kernel logs include information related to hardware detection, driver initialization, boot sequence events, memory errors, and low-level warnings. Kernel logs are exposed by calling `printk()` which allows each log message to be classified into eight severity levels [^1]:

| Level | Macro | Numeric |
|---|---|---|
| Emergency | `KERN_EMERG` | 0 |
| Alert | `KERN_ALERT` | 1 |
| Critical | `KERN_CRIT` | 2 |
| Error | `KERN_ERR` | 3 |
| Warning | `KERN_WARNING` | 4 |
| Notice | `KERN_NOTICE` | 5 |
| Info | `KERN_INFO` | 6 |
| Debug | `KERN_DEBUG` | 7 |

All kernel messages land in the kernel's ring buffer, a fixed-size in-RAM buffer whose size is set at build time (`CONFIG_LOG_BUF_SHIFT`). The ring buffer is stored in RAM, so it is volatile by nature, meaning kernel messages do not survive a reboot. A common way to inspect the kernel message ring buffer is by using `dmesg`, which reads the ring buffer via `/dev/kmsg` or by leveraging `klogctl()`. Most modern Linux distributions are configured by default to ingest kernel logs and pipe them into their own storage mechanism (for journald, this is done with the `ReadKMsg=yes` option in `journald.conf`).

Since many logging daemons already hook into `/dev/kmsg` and store that information, it is worth checking that there is no redundant duplication of kernel messages and that the kernel verbosity filtering is set correctly for development vs. production devices.

## System Log Aggregation

Above the kernel sits the user space, which requires its own logging implementation. The user layer logs information printed by daemons, services, and applications. In Linux, there are two main approaches to system log aggregation and collection: the traditional **syslog** and the newer **journald**, which has gained significant adoption on systemd-based systems. 

### Syslog

Syslog historically has been the standard protocol used for log collection on Linux systems. A syslog daemon aggregates logs and pipes them into human-readable text in `/var/log/`. The main advantage of syslog is that the logs are easy to read and search via tools like `grep`. On standard Linux distros, `/var/log` is mounted to persistent storage, however, for embedded distros, it is more common to mount `/var` on tmpfs (RAM) to help preserve flash writes. Some common syslog daemons are rsyslog, syslog-ng, and for embedded Linux applications in particular, BusyBox syslogd. BusyBox is an open-source set of lightweight Unix utilities that is designed specifically for embedded systems. 

BusyBox syslogd, by default, does not hold logs in a large in-memory buffer. Instead, it writes each message straight through to its log file `/var/log/messages` as it arrives. This means that where those writes actually go is entirely a function of where `/var/log` is mounted, either on persistent storage, where every log line is pushed to the cache for a flash write, or on tmpfs as a RAM write that won't survive a reboot. 

BusyBox does offer the ability to configure syslogd to only log messages into a small shared-memory ring buffer in RAM that you can leverage to read back with logread. The required configurations at compile time are shown below: 

```
 CONFIG_FEATURE_IPC_SYSLOG=y
 CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE=16
 CONFIG_LOGREAD=y
```
This can be invoked by running syslogd with the `-C` flag. This capability allows you to clamp your RAM footprint to the buffer size only and gives you flexibility to implement your own log storage and forwarding functionality. The only caveat around this capability is that, since it is a ring buffer, on highly verbose systems the oldest entries will be lost and overwritten, but for some devices with limited memory space, this is the best option available.

BusyBox syslogd also implements simple severity filtering (-l LEVEL), which drops anything less urgent than the given priority. But it has none of the content-based filtering or rate-limiting functionality that some of the larger daemons like rsyslog and syslog-ng provide.

Since syslogd writes continuously to one file, syslogd should always be paired with some sort of log rotation utility to keep `/var/log` from growing without bounds. Busybox syslogd does include some rotation ability, but generally speaking, syslogd relies on logrotate for rotation. Either way, it is critical to set strict caps on maximum log size and age, compress and rotate the logs into persistent storage for debugging purposes. 

syslogd is a good fit for low-to-moderate verbosity systems with tight resource budgets; however, due to syslogd's limited configuration capabilities, its performance is not as desirable as some other logging systems that have finer filtering and rotation capabilities configured out of the box. If `/var/log` is mounted on flash, a noisy system will wear the flash. Conversely, with `/var/log` on tmpfs, high-volume logs can put pressure on RAM. Either way, if you need finer control over what gets written, you'll want to consider heavier log filtering layers or logs to metrics, which can help alleviate some logging storage pressure. [^2]

### journald 

journald has become increasingly popular over the last few years and is inherently tied to systemd and can only be used on systemd-based systems. Its main advantage over syslog is that it's designed for higher log volume. Instead of storing human-readable text, journald stores logs in a binary, structured, indexed format, which you can query faster with `journalctl -u <unit>` than by grepping through large text files. 

`journald.conf`:

```ini
[Journal]
Storage=persistent      # persistent | volatile | auto | none
Compress=yes            # compress objects above a size threshold (default: yes)
SystemMaxUse=50M        # cap on /var/log/journal size
RuntimeMaxUse=16M       # cap on /run/log/journal (tmpfs) size
SyncIntervalSec=5m      # max time between forced fsync() to physical storage
RateLimitIntervalSec=30s
RateLimitBurst=1000     # drop messages beyond this burst within the interval
ForwardToSyslog=no
```

journald allows you to store logs in a few different places on the device: 

-  `persistent` writes to flash at `/var/log/journal`
-  `volatile` writes to tmpfs/RAM at `/run/log/journal`
-  `auto` will use persistent storage if `/var/log/journal` already exists, otherwise, it falls back to volatile. 
-  `none` disables the on-disk/in-memory journal entirely and only forwards the logs to whatever `ForwardTo*=` targets are enabled. 

As mentioned before, for embedded Linux, `/var` is most likely mounted to `tmpfs`, so it is always worth confirming your mounting scheme is configured as desired. Storing logs in volatile storage is the strongest wear-reduction option available if you don't need logs to survive a reboot, as RAM writes cost zero flash P/E cycles. However, for the majority of logging use cases, this won't gather meaningful insights on what your system is doing and defeats the purpose of including logs in the program in the first place. If you are using other log forwarding to cloud storage, or logs to metrics, then you may be able to leverage either `none` or `volatile` storage, since most of your logs will be stored off device. 

Enforcing size limits on your log files will directly impact how much you are storing. Things like **`Compress=`** will reduce the physical bytes written for the same logical log volume. Utilizing configurations like **`SystemMaxUse=` / `RuntimeMaxUse=`** provides hard caps on the overall size impact in Flash + RAM. Other configurations for tweaking the overall storage strategy of these logs can be found in journald.conf. 

journald also includes rate limiting and write amplification optimizations. The **`SyncIntervalSec=`** batches writes and doesn't `fsync()` on every message, except for the CRIT/ALERT/EMERG messages. A longer sync interval typically means, on average, fewer, larger physical writes for the same log volume, which will correlate with less flash wear impact, at the cost of losing more of the tail of the log if power is lost between syncs. The **`RateLimitIntervalSec=` / `RateLimitBurst=`** cap how many messages per interval will get accepted by journald at all, so a single noisy service can't flood your logs with useless information [^3].

## Reducing What You Store

### Log Centralization and Processing

As the number of deployed devices in the field increase, it is worth thinking beyond common logging tools to consider log centralization and processing to optimize what information you need to store. What if everything landing in journald (or syslog) could be tweaked to only save critical information? This is where tools like `Fluent Bit` can come in and start to filter out logs from important processes. Fluent Bit or a similar application can listen to log streams you configure to apply filters and forward only messages you care about.

Below is a very simple example of a `fluent-bit.conf`:

```ini
[INPUT]
	Name systemd
    Tag kernel.power
    Systemd_Filter _TRANSPORT=kernel

[FILTER]
	Name grep
	Match kernel.power
    Regex   MESSAGE (?i)(low.power|power.management|suspend|thermal|throttl|battery|cpufreq|pmu)


[OUTPUT]
	Name tcp
	Host 127.0.0.1
	Port 5170
```

This current configuration ingests all the kernel logs hitting systemd, listens for logs that discuss things like power and battery channels, and pushes any relevant log to a tcp port. This is a very simple use of filtering, but it can be quite powerful when applied across different application logs entering the system. The various output configurations give flexibility for other applications to ingest this information, which could do further log processing and/or send the critical logs up to the cloud for device monitoring purposes [^4]. 

### Logs to Metrics

The last tool you can leverage to optimize information captured from your devices in the field is to convert Logs to Metrics. What if, instead of storing hundreds of lines of logs for thousands of deployed devices in the field, you counted occurrences of the critical states the device enters? Instead of persisting every Ethernet TX failure as a log line that has to be searched and aggregated on the backend, you could just increment a counter each time this event occurs. Now you can answer questions like "How often is this issue happening?" and gauge issue severity without needing to store all these log lines.

Converting log events to metrics can allow you to filter out textual noise while still gaining meaningful insights. Most log-to-metrics tools are simple to implement and require you to write regular expressions to match against incoming log lines. There are applications that will hook into your logging daemon and increment local metrics of critical events instead of (or in addition to) passing the log through. Both Fluent Bit and memfaultd have easy integrations that let you configure which regex patterns in your logs should increment metrics. 

For example, in memfaultd, if we want to track the number of times the OOM killer has terminated a process, we can increment a counter metric: `OOMKill_<ProcessName>` that increments whenever a we see a log line resembling the following:

```
{
  "counter_name": "oomkill_$1",
  "pattern": "Out of memory: Killed process \\d+ \\((.*)\\)",
  ...
}
```

This now converts textual information into a quick snapshot of the system's state, which can be easily scraped or forwarded to the cloud. When these kinds of metrics compounded over a large number of devices over time,  they help capture a snapshot of failure rates and overall device health, which is critical when addressing field performance. 

## Conclusion

Just as with every other design decision on a memory-constrained embedded device, logging is a set of tradeoffs, not a solved problem. A few questions worth revisiting when tuning your own stack:

**Where should logs live?** Most devices need some information in persistent storage to answer "why did this device crash in the field?" but every byte written to disk is a byte counted against your flash's P/E cycle budget. If you're forwarding logs to the cloud for remote debugging, more stored data also means more cost per device, so the "how much" question isn't just a device-longevity question, it's a fleet-cost question too.

**How should logs be stored?** Plain text is the easiest to read and simplest to search, but it grows fast. Binary, compressed formats (journald) can help shrink the physical footprint for the same logical content and query faster, at the cost of needing the right tooling to read them and runs higher risk of corruption if something goes wrong mid-write.

**How much should you actually log?** Severity levels let you dial verbosity up for active debugging and down for production, but the real lever for device longevity isn't the volume of logs you generate, it is how much of that volume actually gets written to flash, how often, and in what size chunks. Concretely, that means filtering relevant logs, rate-limiting logs as early as possible, and moving anything you only need to *count* out of the log stream and into a metric.

None of these are settings you get right once and forget. Rather, they are tradeoffs worth revisiting as a device's fleet, firmware, and failure modes evolve. Getting them right is what separates a logging architecture that helps you debug a fleet in the field from one that quietly wears out the flash underneath it.


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^1]: [Linux Kernel](https://docs.kernel.org/core-api/printk-basics.html)
[^2]: [BusyBox](https://busybox.net/BusyBox.html)
[^3]: [journald](https://man7.org/linux/man-pages/man8/systemd-journald.service.8.html)
[^4]: [Fluent Bit](https://docs.fluentbit.io/manual/4.0/data-pipeline/inputs/systemd)
<!-- prettier-ignore-end -->


