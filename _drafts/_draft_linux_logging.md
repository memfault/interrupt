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
 "A look at embedded Linux logging challenges around collecting relevant kernel and user space informatio for device debugging while balancing flash wear and RAM pressure on resource-constrained devices."
author: grace
---

<!-- excerpt start -->

Embedded Linux offers developers a flexible platform to develop intricate and complex systems. By nature, embedded systems are memory and resource constrained and require more consideration around how to optimize usage of the resources that are available. When it comes to device observability in the field there are lots of questions around how to balance gathering enough critical information to debug systems without consuming too many resources and harming your overall system performance. This article will walk through various types of Embedded Linux logging platforms and consider various configurations to enhance device debugging without eating into your limited resource budget.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Introduction to Embedded Linux Logging 

Log messages are one of the most valuable tools an engineer has when debugging device issues, and they're typically the first place to go look when triaging a problem. As embedded systems are becoming more sophisticated, the logging architecture and design are playing a more critical role in overall system performance. Simply put, as compute is added more at the edge, log verbosity climbs, and memory writes climb with it. If this is not properly managed it can lead to overall degradation of your system's performance.

It is a set of tradeoffs when gathering diagnostics data of your systems. If every single event is logged you will flood your memory with writes and hand the next engineer thousands of lines of logs to dig through to diagnose an issue. Conversely, if you log too little you are now left with nothing useful to further investigate an issue. These challenges are only amplified further when running on resource constrained devices. For embedded Linux devices, flash wear is a major concern to device longevity in the field. Every log line that reaches persistent storage is a write, and on NAND Flash/eMMC that starts to eat into your flash lifetime. 

## Why Logging Can Stress Your Device

Before diving into specific Linux logging mechanics, it's worth understanding how logging can impact flash wear and degrade device performance.

Most embedded Linux systems use eMMC (Embedded MultiMediaCard) for persistent storage. eMMC consists of a flash controller that manages things like wear leveling and block management and a NAND flash memory that is exposed as `/dev/mmcblk0`. Unlike other memory storage devices, flash has a finite life, primarily gated by the amount of program/erase (P/E) cycles it has before it starts to have unpredictable storage behavior. Simply put, the more you read and write to your flash the quicker the flash wears out. To add to the complexity, not all writes to flash memory are the same. How much you write and the size of what you write directly impacts the flash wear. This concept is commonly known as write amplification factor (WAF), which is the ratio of physical bytes written versus the logical bytes your application actually needed to write to flash. Unsurprisingly, log messages, which are small, frequent writes by nature, have a very bad WAF. It is also worth noting the filesystem layer that is implemented also impacts log driven flash wear, but that won't be touched on in detail in this article. 

The other option is to just write all logs to RAM and prevent inducing any flash wear impact. However, if all logs are stored in RAM, none will persist after reboot and then you lose critical debug information when a system malfunctions. Embedded devices also have finite amounts of RAM, and writing to RAM is not free. If the logs start to use up too much of your RAM, the kernel is left to try its best to free up memory which can start to create performance latency in the device. Ultimately if memory pressure gets too tight the kernel will escalate to the OOM (out-of-memory) killer to try to kill off processes consuming too much RAM which can start causing all sorts of unpredictable device behavior. 

In the end, many of the Linux logging daemons that exist today are tasked to try to balance these various tradeoffs to log and store enough information to be useful, but not more than necessary to preserve critical resources for optimal performance. For every log configuration option the most important questions to keep in mind are **How much data is stored? How often does this write to physical flash? And how large is each write?** 

## Kernel Logging

The lowest layer of Linux logging is kernel logging, which is exposed by calling `printk()`. The kernel classifies every log message with one of eight severity levels [^1]:

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

All kernel messages land in the kernel's ring buffer, a fixed-size in-RAM buffer whose size is set at build time (`CONFIG_LOG_BUF_SHIFT`). The ring buffer is stored in RAM so it is volatile by nature, meaning kernel messages do not survive a reboot. A common way to inspect kernel message ring buffer is by using `dmesg`, which reads the ring buffer via `/dev/kmsg` or leveraging `klogctl()`. Most modern Linux distributions by default are configured to ingest kernel logs and pipe into their own storage mechanism (For journald this is done with: `ReadKMsg=yes` option in `journald.conf`).

Since many logging daemons already hook into `/dev/kmsg` and store that information, it is worth checking that there is no redundancy in copying and storing duplicate kernel messages and the kernel verbosity filtering is set correctly for development vs. production devices.

## System Log Aggregation

Above the kernel sits the user space, which requires its own logging implementation. In Linux there are two main approaches to system log aggregation and collection, the traditional **syslog** and the newer **journald**, which has gained significant adoption on systemd-based systems. 

### Syslog

Syslog historically has been the standard protocol used for log collection on Linux systems. A syslog daemon aggregates logs and pipes them into human-readable text in `/var/log/`. The main advantage of syslog is that the logs are easy to read and can be easily searchable via tools like `grep`. On standard Linux distros `/var/log` is mounted to persistent storage, however, for embedded distros it is more common to mount `/var` on tmpfs (RAM) to help preserve flash writes. Some common syslog daemons are rsyslog, syslog-ng, and for embedded Linux applications in particular, BusyBox syslogd. BusyBox is an open source set of lightweight Unix utilities that is designed specifically for embedded systems. 

BusyBox syslogd, by default, does not hold logs in a large in-memory buffer. Instead, it writes each message straight through to its log file `/var/log/messages` as it arrives. This means where those writes actually go is entirely a function of where `/var/log` is mounted, either on persistent storage, where every log line is pushed to the cache for a flash write, or on tmpfs as a RAM write that won't survive a reboot. 

BusyBox does offer the ability to configure the syslogd to only log messages into a small shared-memory ring buffer in RAM that you can leverage to read back with logread. The required configurations at compile time are shown below: 

```
 CONFIG_FEATURE_IPC_SYSLOG=y
 CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE=16
 CONFIG_LOGREAD=y
```
This can be invoked by running syslogd with -C flag. This capability allows you to clamp your RAM footprint to the buffer size only and gives you flexibility to implement your own log storage and forwarding functionality. The only caveat around this capability is that since it is a ring buffer on highly verbose systems the oldest entries will be lost and overwritten, but for some devices with limited memory space this is the best option available.

BusyBox syslogd also implements simple filter by severity capabilities (-l LEVEL) drops anything less urgent than the given priority. But it has none of the content-based filtering or rate-limiting functionality that some of the larger daemons like rsyslog and syslog-ng provide.

Since syslogd writes continuously to one file, syslogd should always be paired with some sort of log rotation utility to keep `/var/log` from growing without bounds. Most daemons have some level of log rotation included in the configuration but it is critical to set strict caps on maximum log size and age, compress and rotate the logs into persistent storage for debug capabilities. 

syslogd is a good fit for low-to-moderate verbosity systems with tight resource budgets. The limitations show up as verbosity climbs. If `/var/log` is mounted on flash, a noisy system wears the flash. Conversely, with `/var/log` on tmpfs, high volume logs can put pressure on RAM. Either way, if you need finer control over what gets written, you'll want to consider heavier log filtering layers or logs to metrics which can help alleviate some logging storage pressure. [^2]

### journald 

journald has become increasingly popular of the last few years. journald is inherently tied to systemd and can only be used on systemd systems. Its main advantage over syslog is that it's designed for higher log volume. Instead of storing human-readable text, journald stores logs in a binary, structured, indexed format, which is faster to query `journalctl -u <unit>` than grepping through large text files.

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

journald allows you to store logs in a few different places in memory: 

-  `persistent` writes to flash in file path: `/var/log/journal`
-  `volatile` writes to tmpfs/RAM in file path: `/run/log/journal`
-  `auto` will use persistent storage if `/var/log/journal` already exists, otherwise falls back to volatile. 
-  `none` disables the on-disk/in-memory journal entirely and only forwards the logs to whatever `ForwardTo*=` targets are enabled. 

As mentioned before, for embedded Linux `/var` is most likely mounted to `tmpfs` so it is always worth confirming your mounting scheme is configured as desired. Storing logs in volatile storage is the strongest wear-reduction option available if you don't need logs to survive a reboot as RAM writes cost zero flash P/E cycles. However, for the majority of logging use cases this won't gather meaningful insights on what your system is doing and defeats the purpose of including logs in the program in the first place. If you are leveraging other log forwarding to cloud storage, or logs to metrics then you may be able to leverage either `none` or `volatile` storage since most of your logs will be stored off device. 

Enforcing size limits of the log files and how far they can increase in size will directly impact how much you are storing. Things like **`Compress=`** will reduce the physical bytes written for the same logical log volume. Utilizing configurations like **`SystemMaxUse=` / `RuntimeMaxUse=`** provide hard caps on the overall size impact in Flash + RAM. There are other configurations that can be used to tweak the overall storage strategy of these logs that can be found in journald.conf. 

journald also includes rate limiting and write amplification optimizations. The **`SyncIntervalSec=`** batches writes and doesn't `fsync()` on every message, except for the CRIT/ALERT/EMERG messages. A longer sync interval typically means on average fewer, larger physical writes for the same log volume, which will correlate to less flash wear impact, at the cost of losing more of the tail of the log if power is lost between syncs. The **`RateLimitIntervalSec=` / `RateLimitBurst=`** cap how many messages per interval will get accepted by journald at all, so a single noisy service can't turn into flooding your logs with useless information [^3].

## Reducing What You Store

### Log Centralization and Processing

It is worth thinking beyond common logging tools to consider log centralization and processing. What if everything landing in journald (or syslog) could be tweaked to only save critical information? This is where tools like `Fluent Bit` can come in and start to filter out logs from certain processes of importance. Fluent Bit or a similar application can listen to log streams you configure to apply filters and forward only messages you care about.

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

This current configuration ingests all the kernel logs hitting systemd, listens for logs that discuss things like power and battery channels, and pushes any relevant log to a tcp port. This is a very simple use of filtering, but can be quite powerful when compounded to different application logs entering the system. The various output configuration gives flexibility for other applications to ingest this information, who could do further log processing and/or send the critical logs up to the cloud for device monitoring purposes [^4]. 

### Logs to Metrics

The last note to consider around gathering critical information from your devices in the field is the idea of Logs to Metrics. What if, instead of storing hundreds of lines of logs for thousands of deployed devices in the field, you counted occurrences of the critical states the device enters? Instead of persisting every Ethernet TX failure as a log line that has to be searched and aggregated on the backend, you could just increment a counter each time this event occurs. Now you can answer questions like "How often is this issue happening?" and gauge issue severity without needing to store numerous log lines.

Converting log events to metrics can allow you to listen and filter out textual noise to gain meaningful insights. Most of the log to metrics tools are simple to implement and require you to generate regex against incoming log lines. There are applications that exist that will hook into your logging daemon, and increment local metrics of critical events instead of (or in addition to) passing the log through. At this point, you can count the resulting metric that can be scraped or forwarded to whatever time-series/observability backend you use, giving a quick easy way to view failure rates without ever having stored the underlying text long-term.

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
[^2]: [ BusyBox ](https://busybox.net/BusyBox.html)
[^3]: [ journald ](https://man7.org/linux/man-pages/man8/systemd-journald.service.8.html)
[^4]: [ Fluent Bit ](https://docs.fluentbit.io/manual/4.0/data-pipeline/inputs/systemd)
<!-- prettier-ignore-end -->


