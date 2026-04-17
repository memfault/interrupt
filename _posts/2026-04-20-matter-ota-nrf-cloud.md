---
title: OTA Firmware Updates for Matter Devices via nRF Cloud
description:
  Matter devices need firmware updates, but the standard DCL-based approach
  offers limited control and slow iteration. This post walks through
  implementing direct OTA updates using CoAP block transfers over DTLS, with nRF
  Cloud as the OTA backend.
author: francois
tags: [matter, firmware-update, iot, nrf54, ota]
---

In a [previous post]({% post_url 2026-03-24-matter-internet-connectivity %}), we
got a Matter-over-Thread device talking to the internet over UDP. We sent
packets through a HomePod Border Router using NAT64, and received echo responses
from a server on the public internet. That was a proof of concept. Now let's do
something practical with that connectivity: over-the-air firmware updates.

<!-- excerpt start -->

This post walks through implementing OTA firmware updates on a
Matter-over-Thread device, bypassing Matter's standard update mechanism in favor
of a direct CoAP connection to nRF Cloud. We cover the full pipeline: checking
for new versions, downloading firmware in 1024-byte blocks over CoAP, streaming
it to flash, and safely swapping images with MCUboot.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Matter's Built-in OTA

Matter has a built-in mechanism for firmware updates. Here is how it works:

1. The device (OTA Requestor) periodically asks its hub (OTA Provider) whether a
   new firmware version is available.
2. The hub checks the Distributed Compliance Ledger (DCL)[^dcl], a blockchain
   managed by the Connectivity Standards Alliance (CSA).
3. If a new version exists, the hub downloads the firmware binary from the
   manufacturer's server (the DCL stores URLs, not images) and sends it to the
   device using Matter's Bulk Data Exchange protocol.

For each firmware version, the DCL stores the following: a vendor ID, product
ID, software version number, a version string, the certification status, a URL
pointing to the firmware binary on the manufacturer's own server, a firmware
image digest, and a min/max applicable software version range that controls
which existing versions are eligible for this update.

This design has shortcomings:

- **All or nothing.** When you publish a new version to the DCL, every hub in
  the world can see it. There is no way to run experiments, or to update just
  10% to watch for bugs before the full fleet is updated.
- **No cohorts.** You cannot target updates to specific device groups. Internal
  dogfooding, beta testers, regional deployments, hardware revisions that need
  different firmware - the DCL has no concept of any of this. Every device with
  a matching vendor ID and product ID sees the same version.
- **Slow to abort.** If you ship a bad firmware and need to pull it, you cannot
  do it yourself. Revoking a version on the DCL requires the CSA Certification
  Center and vendors cannot act unilaterally. Even after revocation, hubs cache
  update information and only re-check the DCL every 12-24 hours. There is no
  way to recall an update that a hub has already cached. And since Matter does
  not support version rollback, the recovery path is to publish the old firmware
  again under a higher version number.

For any company shipping connected products at scale, the ability to do staged
rollouts, compare firmware performance across cohorts, and react quickly when
something goes wrong is table stakes. The DCL was designed for interoperability
and trust, and it does that well. But it was not designed for fleet management.

```
Standard Matter OTA:

┌────────┐  "any updates?"  ┌─────┐
│ Device │ ───────────────> │ Hub │
└────────┘                  └──┬──┘
                               │
                          check DCL
                               │
                          ┌────┴────┐
                          │   DCL   │
                          │         │
                          └────┬────┘
                               │
                          download from
                          mfr's server
                               │
                          ┌────┴────┐
                          │ Mfr CDN │
                          └─────────┘
```

A manufacturer who wants more control will need to build their own mechanism for
distributing updates to their devices. It turns out, you do not need to build
your own hub to accomplish this.

## OTA Architecture

In the [previous post]({% post_url 2026-03-24-matter-internet-connectivity %}),
we established that a Matter-over-Thread device can reach the internet through
the Border Router using NAT64, and that CoAP over DTLS is the right protocol
stack for constrained devices talking over UDP. Let's build on that foundation.

Our OTA pipeline has three components:

**An OTA backend that speaks UDP.** Our Matter device cannot use HTTP, so we
need a backend that supports CoAP over DTLS. I am using nRF Cloud[^nrfcloud]
because it provides this out of the box, along with firmware hosting, version
management, staged rollouts, and cohort targeting. You could also roll your own.

**Application firmware** on the device that checks for updates, downloads the
new image, and writes it to flash. This is the code we will walk through in this
post.

**MCUboot**[^mcuboot] is the bootloader. It manages two firmware slots (primary
and secondary) and can swap between them safely. If a new firmware fails to
boot, MCUboot automatically reverts to the previous version.

Here is how these three interact:

```
Device (nRF54LM20)                       nRF Cloud
       │                                     │
  1.   │──── DTLS handshake ────────────────>│
       │──── POST /auth/jwt ────────────────>│
       │<─── 2.01 Created ───────────────────│
       │                                     │
  2.   │──── GET /version/check ────────────>│
       │<─── 2.05 Content {url: "..."} ──────│
       │                                     │
       │   [ erase secondary flash - ~60s ]  │
       │                                     │
  3.   │──── GET /firmware (Block2 #0) ─────>│
       │<─── 2.05 + 1024 bytes ──────────────│
       │──── GET /firmware (Block2 #1) ─────>│
       │<─── 2.05 + 1024 bytes ──────────────│
       │           ...                       │
       │<─── 2.05 + final block ─────────────│
       │                                     │
  4.   │   boot_request_upgrade(TEST)        │
       │   sys_reboot()                      │
       │                                     │
  5.   │   [ MCUboot swaps images ]          │
       │   boot_write_img_confirmed()        │
       │   ✓ running new firmware            │
```

Steps 1-2 check whether an update is available. Step 3 downloads the firmware in
1024-byte CoAP blocks. Steps 4-5 apply the update safely.

Let's walk through the implementation.

## Implementation

All the code shown here runs on an nRF54LM20 DK[^nrf54lm20dk] using the nRF
Connect SDK[^ncs]. The complete project is available on
[GitHub](https://github.com/memfault/ncs-example-matter-over-thread).

### Configuration

We need a few Kconfig options on top of the base Matter application from the
previous post:

```
# prj.conf - additions for OTA

# OTA firmware download
CONFIG_STREAM_FLASH=y
CONFIG_IMG_MANAGER=y
CONFIG_MCUBOOT_IMG_MANAGER=y

# Memfault SDK (for OTA version check via Release Management API)
CONFIG_MEMFAULT=y
CONFIG_MEMFAULT_NCS_PROJECT_KEY="your-project-key"

# DTLS (already enabled for nRF Cloud connectivity)
CONFIG_OPENTHREAD_COAPS=y
CONFIG_MBEDTLS_SSL_DTLS_CONNECTION_ID=y
```

`CONFIG_STREAM_FLASH` provides a buffered flash write API that handles page
alignment for us. `CONFIG_IMG_MANAGER` and `CONFIG_MCUBOOT_IMG_MANAGER` give us
the MCUboot APIs to request an image swap and confirm the new image after boot.

The nRF54LM20's partition layout places the MCUboot primary slot in internal
RRAM (the nRF54LM20 uses RRAM, not traditional flash) and the secondary slot on
external SPI NOR flash (a MX25R6435F). This means the secondary slot erase is
slow (~60 seconds for 2 MB), but the primary slot benefits from RRAM's fast
write speeds.

### Checking for Updates

The update check is a CoAP GET to nRF Cloud. Under the hood, nRF Cloud uses a
CoAP proxy architecture: the device sends a request to `/proxy` with a
`Proxy-Uri` option, and nRF Cloud translates it to HTTPS and returns the
response. Here is what the request looks like:

```c
static int memfault_ota_check(const struct shell *sh,
                              struct nrfcloud_session *s,
                              char *url_out, size_t url_out_size)
{
    static uint8_t coap_buf[600];
    char proxy_uri[256];

    /* Build the Memfault Release API URL with device identifiers */
    snprintf(proxy_uri, sizeof(proxy_uri),
             "https://%s/api/v0/releases/latest/url"
             "?device_serial=%s"
             "&hardware_version=%s"
             "&software_type=%s"
             "&current_version=%s",
             MEMFAULT_RELEASE_API_HOST, DEVICE_ID,
             MEMFAULT_HW_VERSION, MEMFAULT_SW_TYPE,
             MEMFAULT_FW_VERSION);

    /* Build CoAP GET with 8-byte token (required by nRF Cloud proxy) */
    uint8_t token[8];
    sys_csrand_get(token, sizeof(token));

    int pos = coap_start_request(coap_buf, sizeof(coap_buf),
                                 COAP_CODE_GET, 0);

    coap_buf[0] = 0x48; /* Ver=1, Type=CON, TKL=8 */
    memcpy(coap_buf + 4, token, 8);
    pos += 12;

    uint16_t prev_opt = 0;

    /* Option 11: Uri-Path = "proxy" */
    pos = coap_append_option(coap_buf, pos, sizeof(coap_buf),
                             &prev_opt, COAP_OPT_URI_PATH,
                             "proxy", 5);

    /* Option 35: Proxy-Uri = full Memfault API URL */
    pos = coap_append_option(coap_buf, pos, sizeof(coap_buf),
                             &prev_opt, COAP_OPT_PROXY_URI,
                             proxy_uri, strlen(proxy_uri));

    /* Option 2429: Memfault project key */
    const char *project_key = CONFIG_MEMFAULT_NCS_PROJECT_KEY;
    pos = coap_append_option(coap_buf, pos, sizeof(coap_buf),
                             &prev_opt, COAP_OPT_MEMFAULT_KEY,
                             project_key, strlen(project_key));

    /* Payload marker (required by proxy, even with no body) */
    coap_buf[pos++] = 0xFF;

    nrfcloud_send(s, coap_buf, pos);

    /* Receive response, filtering by token to skip stale packets */
    /* ... (token matching loop omitted for brevity) ... */

    if (resp.code == 205 && resp.payload_len > 0) {
        /* 2.05 Content - update available, payload is JSON */
        memcpy(url_out, resp.payload, resp.payload_len);
        return 1;
    }
    /* 2.03 Valid - no update available */
    return 0;
}
```

A few things worth noting:

- **8-byte tokens.** nRF Cloud expects an 8-byte token matching the NCS SDK
  convention. We generate a random token for each request and use it to filter
  responses, discarding any stale packets (such as retransmitted authentication
  ACKs) that do not match.
- **Option 2429.** This custom CoAP option carries the project key for
  authentication.
- **Proxy-Uri.** nRF Cloud translates our CoAP GET into an HTTPS GET to the URL
  in this option. The device never needs TCP or TLS.

If an update is available, the response payload is JSON containing a CDN
download URL:

```json
{ "data": { "url": "https://cdn.memfault.com/..." } }
```

### Downloading Firmware Block by Block

We now need to download the image into flash (after we've erased the flash). The
firmware image is too large to fit in a single CoAP response (our images are
~250 KB), so we use RFC 7959 Block2[^block2_rfc] blockwise transfers.

Block2 works like this: the client includes a `Block2` option in its request
specifying which block number it wants and the block size. The server responds
with that block and a flag indicating whether more blocks follow. The client
repeats until the flag says "no more."

We use a block size of 1024 bytes (SZX=6, meaning 2^(6+4) = 1024). This is the
largest size that fits within Thread's effective MTU after DTLS and IPv6
headers.

Note that the code below is simplified and is not production ready. For example,
we do no error checking.

```c
/* Build a Block2 GET request for firmware download */
static int build_block_request(uint8_t *buf, size_t buf_size,
                               const char *download_url,
                               uint32_t block_num, uint8_t szx,
                               const uint8_t *token8)
{
    int pos = coap_start_request(buf, buf_size, COAP_CODE_GET, 0);

    buf[0] = 0x48; /* TKL=8 */
    memcpy(buf + 4, token8, 8);
    pos = 12;

    uint16_t prev_opt = 0;

    /* Uri-Path: "proxy" */
    pos = coap_append_option(buf, pos, buf_size, &prev_opt,
                             COAP_OPT_URI_PATH, "proxy", 5);

    /* Block2: (block_num << 4) | szx */
    unsigned int block2_val = (block_num << 4) | (szx & 0x07);
    uint8_t block2_buf[3];
    int block2_len = coap_encode_uint(block2_buf, block2_val);
    pos = coap_append_option(buf, pos, buf_size, &prev_opt,
                             COAP_OPT_BLOCK2, block2_buf, block2_len);

    /* Proxy-Uri: CDN download URL */
    pos = coap_append_option(buf, pos, buf_size, &prev_opt,
                             COAP_OPT_PROXY_URI, download_url,
                             strlen(download_url));

    buf[pos++] = 0xFF;
    return pos;
}
```

The download loop requests each block, writes it to flash via
`stream_flash_buffered_write`, and repeats:

```c
static uint8_t stream_buf[4096] __aligned(4);
struct stream_flash_ctx stream;

stream_flash_init(&stream, fa->fa_dev, stream_buf,
                  sizeof(stream_buf), fa->fa_off, fa->fa_size, NULL);

uint8_t dl_token[8];
sys_csrand_get(dl_token, sizeof(dl_token));

uint32_t block_num = 0;
size_t total_bytes = 0;
static uint8_t coap_buf[1200];

while (true) {
    int pkt_len = build_block_request(coap_buf, sizeof(coap_buf),
                                      download_url, block_num,
                                      COAP_BLOCK_SZX_1024, dl_token);

    /* Send request with retry (up to 3 attempts per block) */
    struct coap_response blk;
    for (int retries = 0; retries < 3; retries++) {
        nrfcloud_send(&s, coap_buf, pkt_len);
        int ret = recv_block_matching(&s, coap_buf,
                                      sizeof(coap_buf), dl_token);
        if (ret > 0) {
            coap_parse_response_full(coap_buf, ret, &blk);
            if (blk.code == 205) break;
        }
    }

    /* Write block payload to flash (4KB buffered) */
    stream_flash_buffered_write(&stream, blk.payload,
                                blk.payload_len, false);
    total_bytes += blk.payload_len;

    if (!blk.has_block2 || !blk.more) break;
    block_num++;
}

/* Flush final partial page */
stream_flash_buffered_write(&stream, NULL, 0, true);
```

`stream_flash_buffered_write` is doing important work here. External SPI NOR
flash typically requires writes aligned to page boundaries (often 4 KB). Our
CoAP blocks arrive in 1024-byte chunks. The stream flash API accumulates these
into a 4 KB buffer and writes to flash in page-sized chunks, handling alignment
transparently.

We also retry each block up to 3 times. Thread's mesh routing occasionally drops
a packet, and a transient recv timeout should not abort a 250 KB download.

### MCUboot Swap and Confirmation

With the firmware written to the secondary slot, we tell MCUboot to swap it in
on the next boot:

```c
boot_request_upgrade(BOOT_UPGRADE_TEST);
sys_reboot(SYS_REBOOT_COLD);
```

`BOOT_UPGRADE_TEST` tells MCUboot to swap the images, but treat the new image as
a **test**. If the new firmware does not explicitly confirm itself, MCUboot will
revert to the previous image on the next reboot. This is the safety net for
remote devices: a buggy firmware that crashes before confirming will be rolled
back automatically.

The image is confirmed early in `main()`, before the application starts:

```c
// src/main.cpp
if (!boot_is_img_confirmed()) {
    boot_write_img_confirmed();
    LOG_INF("OTA: new firmware confirmed");
}
```

Once confirmed, the new image is permanent.

> _Note_ you may chose to confirm the image after e.g. a connection was
> successfully established.

## Trying It Out

Before the device can check for updates, we need to upload the firmware image to
nRF Cloud. The file to upload is the MCUboot-signed binary,
`build/zephyr/app_update.bin` - not the `.elf` or the unsigned `.hex`. This is
the image that MCUboot knows how to verify and swap.

You can upload it through the Memfault web UI or the CLI[^memfault_ota]. From
the command line, it looks like this:

```bash
memfault upload-ota-payload \
  --hardware-version nordic-device \
  --software-type nordic-device-software \
  --software-version 1.1.0 \
  build/zephyr/app_update.bin
```

Once uploaded, create a release and activate it for your cohort. See the
Memfault OTA documentation[^memfault_ota] for details on staged rollouts and
cohort targeting.

With the image uploaded, let's see it in action. First, a quick update check:

```
uart:~$ nrfcloud ota_check
Resolving coap.nrfcloud.com ...
Connecting to [64:ff9b::xxxx:xxxx]:5684 ...
DTLS handshake OK
Sending POST /auth/jwt ...
Auth: 2.01 Created
Checking for OTA update...
OTA response: 2.05
Update available: {"data":{"url":"https://cdn.memfault.com/..."}}
```

Now the full update - check, erase, download, and reboot:

```
uart:~$ nrfcloud ota
Resolving coap.nrfcloud.com ...
Connecting to [64:ff9b::xxxx:xxxx]:5684 ...
DTLS handshake OK
Auth: 2.01 Created
Checking for OTA update...
OTA response: 2.05
Erasing secondary slot (1978368 bytes)...
Erase complete.
Connecting to [64:ff9b::xxxx:xxxx]:5684 ...
DTLS handshake OK
Auth: 2.01 Created
Downloading firmware...
  0 KB downloaded
  100 KB downloaded
  200 KB downloaded
Downloaded 245760 bytes (240 blocks)
Rebooting to apply update...
```

After the reboot, MCUboot swaps the images and boots the new firmware:

```
*** Booting MCUboot v2.1.0-ncs1 ***
Swap type: test
Starting swap using move algorithm.
[00:00:02.140,000] <inf> app: OTA: new firmware confirmed
[00:00:02.145,000] <inf> chip: [SVR]Server initializing...
```

The whole process takes about 5 minutes.

## Conclusion

We built a complete OTA pipeline for a Matter device: version checks, blockwise
firmware download over CoAP, streamed flash writes to the MCUboot secondary
slot, and safe image swap with automatic rollback. The entire path runs over
CoAP/DTLS through the Thread Border Router - the same UDP connectivity we
established in the [previous
post]({% post_url 2026-03-24-matter-internet-connectivity %}), now doing real
work.

Block2 over Thread is reliable but not fast. At ~4 KB/s, a 250 KB image takes
about a minute. For typical firmware sizes this is fine, but very large images
would benefit from a faster transport.

nRF Cloud provides the fleet management features that Matter's DCL lacks: staged
rollouts, cohort targeting, and version management with a fast development loop.
It offers a free tier for up to 10 devices, and pay-as-you-go pricing after
that[^nrfcloud_pricing].

The complete project is available on
[GitHub](https://github.com/memfault/ncs-example-matter-over-thread). We would
love to hear about your experience in the comments.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^dcl]: [CSA Distributed Compliance Ledger](https://webui.dcl.csa-iot.org/)
[^nrfcloud]: [nRF Cloud](https://nrfcloud.com/)
[^memfault_ota]: [Memfault OTA Releases Integration Guide](https://docs.memfault.com/docs/mcu/releases-integration-guide)
[^mcuboot]: [MCUboot](https://docs.mcuboot.com/)
[^block2_rfc]: [RFC 7959: Block-Wise Transfers in the Constrained Application Protocol](https://datatracker.ietf.org/doc/html/rfc7959)
[^nrf54lm20dk]: [nRF54LM20-DK Product Page](https://www.nordicsemi.com/Products/Development-hardware/nRF54LM20-DK)
[^ncs]: [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK)
[^nrfcloud_pricing]: [nRF Cloud Pricing](https://nrfcloud.com/pricing)
<!-- prettier-ignore-end -->
