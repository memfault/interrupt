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

title: Cross-compilation in various programming languages
description:
  Experimenting with cross-compiling a simple program in various programming languages
author: noah
image: img/<post-slug>/cover.png # 1200x630
---

A little bit of background of why the reader should read this post.

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

Optional motivation to continue onwards

{% include newsletter.html %}

{% include toc.html %}

## Overview

Cross compile a simple "Hello" program for the following targets:

1. Linux x86_64
2. Linux aarch64
3. Windows
4. MacOS x86_64
5. MacOS aarch64

## C

## Go

## Python

## Rust

## Zig

<https://ziglang.org/learn/overview/#cross-compiling-is-a-first-class-use-case>

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

- <https://drewdevault.com/2020/01/04/Slow.html>

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
