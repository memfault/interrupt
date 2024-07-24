---
title: Considerations when Building Embedded Databases
description:
  Diving into the challenges of persisting device state to flash memory.
author: chrismerck
---

<!-- excerpt start -->

Persisting to flash is a necessary evil for many embedded devices. Let's take a
look at some of the pitfalls and how they may be avoided.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Why You Should Care About NVS (Non-Volatile Storage)

Nearly every embedded system needs to persist data across power cycle. A smart
fishtank needs to remember the configured salinity and oxygen setpoints, an
e-bike may store a geofence and riding log, and a smart home hub must store
device, group, and schedule information.

Let's assume for now that we're talking about a small embedded system, like an
STM32 with only the built-in flash available for persistent storage. Is there an
obvious solution? What happens if we just write our data to flash and read it
back on reboot?

For the fishtank example, the data we want to persist could be organized into a
single struct which is read from a specific flash address on boot and written to
that address when the user changes the settings:

```c
typedef struct {
    int salinity;
    int oxygen;
} fishtank_config_t;

fishtank_config_t config;

void load() {
  flash_read(CONFIG_ADDR, &config, sizeof(config));
}

void save() {
  flash_erase_page(CONFIG_ADDR);
  flash_program(CONFIG_ADDR, &config, sizeof(config));
}
```

This solution may "just work" for an MVP fishtank, but as the product evolves we
are going to run into a few issues:

1.  The persisted data is not portable due to endianness and integer size
    differences,
2.  The config will be corrupted if power is lost during the `save()` function,
3.  If a `temperature` field is added to the struct, it will be read as `-1`
    upon firmware upgrade, which may be unexpected.

So we see that even for the simplest products, persisting data to flash presents
some maintainability challenges.

## Aside: Portability of Records

Thinking ahead, you may eventually wish to run your fishtank code on different
hardware either because of an upgrade to the product, a new version of the
product, or because you want to be able to load up records from the embedded
system inside a mobile app or a server.

That new processor may be 64-bit processor, or may even use a different
endianness than the microcontroller. Taking a closer look at the
`fishtank_config_t` struct, we see that the basic `int` type is used, which uses
4 bytes on 32-bit chips, but might use 16- or 8- bits on other architectures,
and the endianness (byte order) depends on the processor.

### Integer Size and Alignment

With the definition of `fishtank_config_t` above, `sizeof(fishtank_config_t)`
will be different for different platforms. Compiling on a 64-bit MacBook where
Apple kept `int` as 32-bit with 32-bit alignment, you'll get the same 8-byte
structure as on a 32-bit microcontroller. However, if you compile on an AWS EC2
instance (as you may want to do if there is a cloud component to your fishtank),
then you get a 16-byte structure because `int` there is 8 bytes.

For portability, use the fixed-width definitions in `stdint.h` and instruct the
compiler to pack your structure to remove any padding which would otherwise be
added to encourage alignment to word boundaries:

```c
#pragma pack(push, 1)
typedef struct {
    int32_t salinity;
    int32_t oxygen;
} fishtank_config_t;
#pragma pack(pop)
```

### Much Ado About Endianness

There's one remaining portability issue. There have historically been
"big-endian" processors which represent multi-byte integers with the
most-significant byte first rather than least-significant byte first (so-called
"little endian") which has become the dominant endianness today. This means that
even with the fixed-sized, packed structure above, there may be processors where
a record with `salinity == 2` is interpreted as `33554432` because of the endian
mismatch.

If this sounds confusing, that's because it is. Even when working in a systems
language such as C, we get used to the convenient abstraction of "an integer"
without worrying about how it is laid out in memory. So let's take a look at how
the a 32-bit unsigned integer with the hexadecimal value `0xDEADBEEF` is stored
in memory using either endianness convention:

| endianness    | byte 0 | byte 1 | byte 2 | byte 3 |
| ------------- | ------ | ------ | ------ | ------ |
| little-endian | 0xEF   | 0xBE   | 0xAD   | 0xDE   |
| big-endian    | 0xDE   | 0xAD   | 0xBE   | 0xEF   |

You can see that on little-endian systems, the least significant byte (`0xEF`)
comes in the first memory location, as opposed to big-endian which starts with
the most significant byte (`0xDE`). Now it should be clear that if the same byte
buffer is loaded into a big-endian and a little-endian system, they will be
interpreted as very different integers.

At Bond, we initially took pains to build `hton` (host-to-network) functions for
every structure which would enforce big-endian integer representations, in
keeping with the tradition of TCP/IP networking and numerous file formats.
However, maintaining those `hton` functions is highly error-prone and it is easy
to end up with records that have some of their multi-byte fields in "network
byte order" while others remain in the native byte order. What a mess!

What we learned is that, unless you need to support some very exotic big-endian
architecture, you're better off ignoring endianness and just letting all your
structs be little-endian. This greatly simplifies maintainability, reduces
confusion for developers unfamiliar with byte-order, saves CPU time, and allows
working with constant record data without RAM copies to reverse the byte order
before write to flash.

## Challenges of Flash Memory

Let's consider a few of the ways that flash differs from RAM from the
C-programmer's perspective:

1. First off, flash must be **erased** before it can be **programmed**. Erasing
   changes all bits to `1` while programming changes some bits to `0`. And
   although single bits may be programmed, only entire **pages** of flash may be
   erased.

2. These pages vary in size between flash chips and microcontrollers, from 512
   bytes to 32k, with 4k being a typical size.

3. Flash operations must be **aligned** to specific offsets, usually 32 bytes
   but this varies. The page size, alignment, and possibly restricted read/write
   lengths together are referred to as the flash **geometry**.

4. Flash has a limited number of program-erase cycles, typically 10k to 100k.
   So, to avoid premature failure, values that change many times per day must be
   distributed across different flash pages, a scheme known as "wear leveling".

5. Flash reading, programming, and especially erasing, is much slower than RAM
   operations. So frequently-accessed data must at least be cached in RAM.

These challenges mean that for non-trivial applications such as the smart home
hub where there are many small pieces of information to persist, there is a
clear need for a well-organized scheme for managing the use of the flash to save
and load this data. This is what we refer to as an **embedded database**.

## Considering your Requirements for an Embedded Database

Now that it's clear why you need an embedded database, let's consider the
different constraints which may lead you in one direction or another when
shopping or building a solution:

#### How much data do you have to store? And how much flash is available?

You should start with a back-of-the-envelope calculation of the total amount of
data that your device needs to store. This is harder than it sounds because
you've got to also anticipate space for future features.

My approach is usually to be fairly greedy and allocate a lot of flash to a new
product line, both to the code space and database. This way we can add features
freely without concern for running out of space if the product is successful
enough to experience a decade of feature creep.

#### How important is resilience to power loss during write operations?

Data loss and corruption is incredibly frustrating for customers and expensive
in terms of customer support and reputation. So, your embedded database
should not lose data if power is lost at an inopportune time (such as during a
program or erase operation).

#### Are you storing just one record, or many records?

Some simple applications can get away with a single record structure. However,
if you need arbitrarily-sized enumerations as in a smart home hub that needs to
track schedules for and of several devices, you'll need a database that can
store multiple copies of multiple record types.

#### Do the records form a natural hierarchy?

It can be helpful to take advantage of hierarchical relationships if only for
visualization and detection of orphaned records.

#### How often are records being re-written?

It's important to make a calculation of the number of times any flash page is
expected to be erased in the lifetime of the product and ensure that it is under
the specified endurance in the flash manufacturer's datasheet.

These calculations, although simple, can defy intuition. The highest flash
endurance of 1 million cycles seems like a big number, but if you don't have any
wear leveling and write a record every 10 seconds, you will pass a million
cycles in just 4 months. And the flash built into microcontrollers usually has
much lower endurance, around 10k cycles.

#### How big are your records? Do you tend to write a whole record at once? Or do you often append to a record?

Things get much more complicated when you've got records which need to be able
to be appended to or partially written. If you need these complexities, it's
best to select a filesystem or at least you will want a file-like API.

Keeping things simple means having just a get(), set(), and possibly list() API
where whole records must be read and written at once.

#### When can you tolerate garbage collection delays?

At some point deleted records will need to be cleaned up to make space for more
data. How often this happens depends on the application, but when something like
a device name is included in the device record, any renaming of a device will
result in a dead record.

When those dead records eventually are compacted, a delay for flash page erasure
is incurred. This could be designed to happen on demand (practical and easy), or
to run in the background when the device is idle (sounds better, but hard to
pull off).

#### Do you need a backup and restore function?

Effectively backing up and restoring from backups can be a really powerful user
story. However, doing this well will require some support at the embedded
database level. Simply copying the entire database partition may be excessively
large.

Databases can also be designed to stream records out as part of a backup
mechansism.

#### Is the database read-write or write-only?

In many applications the data you are storing may be logs or sensor data that
never needs to be read by the embedded system, but only recorded for later
analysis by a larger application. Usually this kind of data forms a time-series.
If you know the records to be of fixed size or small maximum size, you could
simply write each record to flash at regular offsets. However, you still should
consider what happens when reaching the end of the allotted flash space. If
continued writes are needed after the partition is full, then a scheme for
handling the erasure of full pages is required which again complicates life for
the implementor.

## A Brief Look at a Few Existing Solutions

Now that we've discussed the main challenges of flash memory for data storage,
let's take a very brief look at a few specific implementations and their design
choices:

### Espressif's NVS Flash

If you are building a product using Espressif SoCs like ESP32, then you have an
full-featured built-in choice called
[nvs_flash](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html).
Some salient features are:

- key-value store with string-based keys
- namespace system avoids conflicts between components
- can efficiently enumerate all records in a given namespace
- binary blobs are allowed to exceed page size
- highly efficient for small entries such as integers
- supports resuming of a compaction operation interrupted by power loss
- only officially available for Espressif chips

### Zephyr's NVS

The Zephyr operating system also offers a key-value
[database implementation](https://docs.zephyrproject.org/latest/services/storage/nvs/nvs.html),
which is characteristically lightweight. Two points to note are:

- keys are only 16-bit, so careful planning is required to avoid conflicts
- unlike other solutions, the data CRC is optional, so you can live dangerously
  and save on flash space

### SPIFFS (SPI Flash File System)

[SPIFFS](https://github.com/pellepl/spiffs) is way more ambitious than the
key-value stores discussed above, seeking to provide a POSIX-like filesystem
interface featuring `open`, `write`, `read`, and `seek` functions so that code
written for SPIFFS could potentially execute unmodified on a major OS like
Linux. However, trying to pack full filesystem-like capabilities into the RAM
available in a small embedded system leades to steep tradeoffs in performance
and safety. Reading the [SPIFFS FAQ](https://github.com/pellepl/spiffs/wiki/FAQ)
shines some light on these challenges.

### Bond's _Beau_ Embedded Database

When we were building the Bond Bridge, we looked around for embedded database
solutions but none of the free software solutions available at the time were up
for the task. ESP32 nvs*flash was in early days and the venerable FAT12 format
was too inefficient with small records. So after shipping an MVP with a very
limited database that we affectionately dubbed "UglyDB", we rolled our own
embedded database that we call \_Beau*.

We've used Beau on both smart ceiling fans with 128 kB storing schedules, group,
and scenes and 4 MB for Bond Bridge (basically a smart home hub) which needs to
store similar information about possibly hundreds of devices plus recorded RF
and IR signals.

Our database stores records like this:

```
FFFF BEEF [key] [type] [parent] [data_len] [data_crc] [header_crc]
[data ... ]
```

The idea is that the first word `FFFF BEEF` is a sentinel that signals that a
record follows. The header contains a 64-bit unique record key, 16-bit record
type, the 64-bit key of the parent record (useful for building hierarchical
relationships), the length of the record data, a checksum of the record data,
and finally a checksum of the header.

At boot, the microcontroller can efficiently scan through the database, loading
the key and type metadata, memorizing the physical address (flash offset) of
every record. This scan is efficient because only the headers need be read. The
contents can be skipped over. Things only slow down in the case of corruption,
in which case it is necessary to scan through the database looking for another
valid header.

The `FFFF` at the beginning of the sentinel is important. These `1` bits are
"unprogrammed", so we are able to delete a record by simply programming some of
them so the sentinel reads `DEAD BEEF`. The header is still intact so it's still
possible to efficiently scan through the database.

We always reserve one page (keeping it empty) so that when it is time to compact
a page, all the live records from the old page can be copied to the reserved
page. The old page can then be erased, freeing up the space occupied by dead
(deleted) records.

A killer feature of Beau is backup and restore. To back up, we simply stream the
database records out using HTTP chunked transfer. To restore, we again use
chunked transfer and this time write each record to the database. This technique
has unlocked efficient transfer of smart home devices between hubs, and for an
easy path for upgrades to newer models of hubs, and of course for recovery after
hardware failure or catastrophic firmware issues.

## Remote Monitoring of Database Statistics

Lastly, building and deploying an embedded database requires some way of
monitoring performance. [Memfault's
metrics](https://memfault.com/customers/bond-home-case-study/) can be used for
this purpose.

These are a few things you may want to keep an eye on:

1. Number of operations since boot. Track `gets`, `sets`, `deletes`, and
   `compactions`.

2. How much free space is available? And how much space is occupied by dead
   records?

3. What's the largest headroom available in a page? This is similar to
   `largest_free_block` in heaps.

4. Error due to running out of space in the database due to no page large enough
   to hold the record.

5. Any CRC errors. These happen more often than you may expect, and will surely
   degrade customer experience in some way. So it's a good idea to be able to
   correlate them to specific units and support tickets.

Here's what one heavily-used unit's statistics look like for Bond Bridge Pro. As
you see, our allocation of 4 MB for the database has left us lots of room for
expansion:

```json
{
  "live_records": 235,
  "dead_records": 1516,
  "page_size": 4096,
  "total_dead": 648704,
  "total_free": 4121024,
  "max_free": 4096,
  "total_head": 3472320,
  "max_head": 4096,
  "compactions": 0,
  "empty_pages": 190,
  "size": 4194304,
  "dirty": false,
  "landmines": -3,
  "sets": 4,
  "gets": 2708,
  "deletes": 0
}
```

Happy building! And remember, persistence is not futile. Just never
underestimate the limitations of flash memory.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

<!--## References-->

<!-- prettier-ignore-start -->
<!--[^reference_key]: [Post Title](https://example.com)-->
<!-- prettier-ignore-end -->
