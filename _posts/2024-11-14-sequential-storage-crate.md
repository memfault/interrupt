---
title: Sequential-storage － Efficiently Store Data in Flash
description:
  A discussion of a sequential storage crate which can be used in Rust projects
  for efficiently storing data in NOR flash.
author: diondokter
tags: [rust, flash]
image: /img/sequential-storage-crate/sequential-storage-main-image.jpg
---

<!-- excerpt start -->

While using a full-blown filesystem for storing your data in non-volatile memory
is common practice, those filesystems are often too big, not to mention annoying
to use, for the things I want to do. My solution? I've been hard at work
creating the
[sequential-storage crate](https://crates.io/crates/sequential-storage). In this
blog post I'd like to go over what it is, why I created it and what it does.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

Sequential-storage implements low-level NOR flash memory management so you can
store fifo queues and key-value maps in memory. This makes the crate very
well-suited for data caches and configs. The special thing about this crate is
that it minimizes page erases and that's where the name comes from.

A naïve implementation would store one item per page. If you then have a config
that is updated every minute or so, you'd have to erase the config every minute
and write out the new state. A 10K erases flash would only last ~7 days and a
100K erases flash only ~70 days. That's far too short for any serious embedded
application.

So instead of storing a value on a page, we should be a bit smarter. In this
implementation multiple items are stored one after another on the same page,
forming a sort of sequence. Hence, sequential-storage.

![Store on page 0 meme](/img/sequential-storage-crate/stored-on-page-0-meme.png)

For example, if you want to update the value of a key in the key-value map data
structure, a new item is appended instead of erasing it and then writing it
again.

After many extra features, lots of fuzz testing, and code hardening we've now
gotten to the current state of the crate.

## Why create it?

It wasn't the first time we needed a crate like sequential-storage. In fact,
this is the second such crate in [our GitHub](https://github.com/tweedegolf).

The first one was called [nvm-log](https://github.com/tweedegolf/nvm-log) and
was initially written by a colleague of mine. We used it to store log messages
from our device in flash so we could send them to the server later.

If you look at the code of nvm-log, you'll see some things that, at least
conceptually, carried over into sequential-storage. There are page states, and
items in flash are written sequentially.

Nvm-log worked fine for the exact application it was written for. However, when
we wanted to reuse nvm-log in a new application, we ran into some issues. For
this new project we needed two things that nvm-log at least in theory could do:
Cache messages to be sent to the server later, and keep a config.

What we found was that the old crate had many assumptions and breaking those
assumptions meant we had to deal with bugs and API choices that didn't fit our
new application. Note however that creating a crate like this in one go is very
hard to do! It's very reasonable that nvm-log wasn't perfect.

Fixing the bugs and APIs wouldn't have been a problem normally, but nvm-log was
written in a way where pretty much everything was implicit. It would need to
deal with implicit page states and item layouts. Because this was all implicit,
you never knew what was already checked for in each part of the library. This
made it difficult to fix the bugs we encountered.

After every bug fixed another popped up a bit later. At some point I got fed up
with it and decided to write something new from scratch. It would benefit from
lessons learned from nvm-log, would be far less opinionated, and would have more
features.

## What does sequential store do?

(I only give some background here. For up-to-date technical details, go to the
[repo](https://github.com/tweedegolf/sequential-storage).)

The main thing I learned from nvm-log is that I wanted sequential-storage to be
explicit and with little runtime state. The biggest change was the way the API
was presented to the user.

Nvm-log was a queue data structure. You would give the crate a reference to your
flash, which it would hold on to. It would then search for the address where the
next log could be stored. After that you'd have an nvm-log instance where you
could store a new item or get an iterator over all items currently stored.

Wanted to do something with your flash while holding the nvm-log instance? Well,
the flash is already borrowed.

Sequential-storage doesn't hold on to your flash, or anything for that matter.
In fact, the crate exposes (almost) all APIs as free-standing functions. This
make the use of the API very flexible at the cost of having to provide some
parameters every time. You won't have to fight the borrow checker (because I've
already defeated it for you in the crate internals 😈).

This style also forces all information to always be stored in flash. This makes
everything very predictable since there are no bugs that can hide in the runtime
state.

There are so many things I like about the current design:

- Page states are now explicit, with many functions to change the states and to
  read them out.
- Items, the things that actually get stored in flash, are very well-defined and
  CRC-protected.
- The user can provide the big ram buffer that is used to read the flash bytes
  into.
- This allows the user to put the buffer in static memory instead of it being
  forced on the stack.
- Users can decide how big it is which allows them to minimize the memory usage
  based on their data.
- The crate has been thoroughly fuzzed with simulated early device shutoff
- Any corruption due to early shutoff can be repaired
- And more...

## Latest Developments

The last thing I've been working on is the implementation of caching.

While it's nice to not hold any volatile runtime state, it isn't great for
performance. Any time you need to fetch something from the flash, it has to be
searched for since the implementation can't possibly know where it was stored.
This means the algorithms have to do a lot of reads, which slows everything
down.

With the new cache functionality, a lot of searching can be skipped. There are
different implementations users can pick from, which offer different performance
and memory use trade-offs.

And yes, this sounds like a huge pile of runtime state, something I was trying
to avoid. However, the cache is optional and you can opt to go without. This
means the code still has to be written as though the cache doesn't exist. The
cache merely serves as an early return if some information is available in
cache.

By making the cache optional, finding bugs remains easy. Simply don't use the
cache and if the bug is still there, the bug is in the main logic. Otherwise,
it's a caching issue.

## Conclusion - Towards 1.0

The crate is already being used in production. Two of our clients and at least
two projects 'out there' (i.e. not mine or Tweede golf's) use it.

As I continue to work on it, my wishlist is getting smaller. There are only a
few things that I think are still missing. Once my wishlist is small enough, it
would make sense to switch the version to a 1.0 release to mark the crate as
'done' (although I'll still be updating and doing releases).

Is there a feature that you're missing that stops you from using
sequential-storage? Let me know! I might be able to implement it.

> **Note**: This article was originally published by Dion on the Tweede Golf
> blog. You can find the original article
> [here](https://tweedegolf.nl/en/blog/115/sequential-storage-efficiently-store-data-in-flash).

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
