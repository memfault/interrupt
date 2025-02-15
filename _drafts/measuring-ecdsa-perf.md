---
title: Measuring Embedded TLS 1.3 + ECDSA Performance in the Real World
description: Methodology and results for measuring the performance of TLS 1.3 + ECDSA on embedded devices.
author: noah
---

TLS 1.3 is the latest version of the Transport Layer Security protocol,
published in August 2018 as the successor to TLS 1.2 (6 years old when this
article was written!). It's now more and more common to see embedded devices
using TLS 1.3 for secure communication. Let's dig into what that means for these
devices, and take some measurements to see how it performs in the real world!

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Background: TLS 1.2 + 1.3

## Background: ECDSA vs. RSA

## Methodology

1. set up local TCP server serving a 64 byte payload, configured in 4 different
   ways:

   1. TCP only (no TLS)
   1. TLS 1.2 + RSA
   1. TLS 1.3 + RSA
   1. TLS 1.3 + ECDSA

1. client tests, measuring:

   1. connection handshake time
   1. total bytes sent + received
   1. peak memory usage

   tests repeated 10x on each platform, 2 outliers tossed, and results averaged

1. client: mbedtls on linux (x86_64)

1. client: ESP32 + ESP-IDF v5.3

1. client: ESP32 + Zephyr 3.7

1. client: nrf7002 + NCS 2.7

Results:

| Platform        | Connection Type | Connection Handshake Time | Total Bytes Sent + Received | Peak Memory Usage |
| --------------- | --------------- | ------------------------- | --------------------------- | ----------------- |
| Linux           | TCP             | 1.2ms                     | 128 bytes                   | 1.2MB             |
| Linux           | TLS 1.2 + RSA   | 3.2ms                     | 256 bytes                   | 1.5MB             |
| Linux           | TLS 1.3 + RSA   | 4.2ms                     | 256 bytes                   | 1.5MB             |
| Linux           | TLS 1.3 + ECDSA | 5.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + ESP-IDF | TCP             | 2.2ms                     | 128 bytes                   | 1.2MB             |
| ESP32 + ESP-IDF | TLS 1.2 + RSA   | 4.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + ESP-IDF | TLS 1.3 + RSA   | 5.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + ESP-IDF | TLS 1.3 + ECDSA | 6.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + Zephyr  | TCP             | 2.2ms                     | 128 bytes                   | 1.2MB             |
| ESP32 + Zephyr  | TLS 1.2 + RSA   | 4.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + Zephyr  | TLS 1.3 + RSA   | 5.2ms                     | 256 bytes                   | 1.5MB             |
| ESP32 + Zephyr  | TLS 1.3 + ECDSA | 6.2ms                     | 256 bytes                   | 1.5MB             |
| nrf7002         | TCP             | 3.2ms                     | 128 bytes                   | 1.2MB             |
| nrf7002         | TLS 1.2 + RSA   | 5.2ms                     | 256 bytes                   | 1.5MB             |
| nrf7002         | TLS 1.3 + RSA   | 6.2ms                     | 256 bytes                   | 1.5MB             |
| nrf7002         | TLS 1.3 + ECDSA | 7.2ms                     | 256 bytes                   | 1.5MB             |

## Conclusion

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
