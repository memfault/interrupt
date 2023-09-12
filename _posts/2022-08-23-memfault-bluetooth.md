---
title: Integrating Memfault with Blecon Bluetooth Devices
description:
  In this post, I’ll go through how we evaluated Memfault at Blecon and talk through what we liked and why we decided to use it, how we integrated it into the nRF5 platform and carried the Memfault data over to the Memfault cloud via Bluetooth.
author: donatien
tags: [ble, memfault, connectivity, observability]
---

I’m Donatien, co-founder of [Blecon](https://blecon.net/), where we provide device connectivity to the Internet using only Bluetooth. In my previous lives working in the IoT division at Arm and my previous start-up, our teams constantly wrestled with debugging firmware bugs on remote devices.

To mitigate these issues, we would build one-off logging solutions, but we would often end up having to ask a (non-technical) customer to try and connect a debugger to a device (painful for both parties involved) or fly someone to debug the bug onsite (expensive and not environmentally friendly!). I knew we’d face some of these challenges again at Blecon, so I wanted to check out if Memfault could help mitigate or even eliminate them.

<!-- excerpt start -->

In this post, I’ll go through how we evaluated Memfault at Blecon and talk through what we liked and why we decided to use it, how we integrated it into the nRF5 platform and carried the Memfault data over to the Memfault cloud via Bluetooth.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

##  Background

At Blecon, we are enabling SaaS applications to securely integrate IoT devices thanks to Bluetooth Low Energy. Our technology is available to both device builders and web developers. We use smartphones as roaming network access points to avoid the need to deploy new infrastructure on-site. For more background on what we are up to, see [https://blecon.net](https://blecon.net).

On the embedded side, we’re developing modem firmware that can be integrated into a device in order to easily and securely access a Blecon network. The modem allows a host MCU to send and receive data, handling device identity, registration and connecting securely to a network.

<img width=800px src="{% img_url memfault-bluetooth/blecon-architecture.png %}" />

In developing the modem code we wanted to achieve the following:
* Iterate fast on the software we’re producing
* Maintain and improve quality over time

Usually, these two goals conflict for embedded projects, which means that a lot of devices are still built using a waterfall model leading to very long development cycles.

However, this is not a desirable option for us. We want to ship early and learn from our users and customers in order to iteratively build a better product. We also want to make sure device builders using Blecon can focus on building their own products, without our solution getting in the way.

Therefore we are using the following strategy for our modem software:
* Set-up internal automated unit and integration testing in CI (catch bugs as early as possible)
* Leverage an observability platform (catch bugs in the field)
* Add firmware update functionality (allows us to fix bugs and add features to devices in the field)

In this blog, we’re focusing on point #2.

## Why we chose Memfault

We evaluated some platforms that could collect and analyze logs and crash dumps for our modem. There are plenty of options in the mobile and cloud spaces, however, this is quite new to the world of embedded!

Among the options we had, Memfault stood out. Something we were impressed with is the product’s focus on the core functionality (device monitoring) and not being too opinionated elsewhere - this matches the Blecon philosophy well. Especially, Memfault looked easy to integrate with any connectivity transport, which really suited us. We had been reading Interrupt for a while too, so it didn’t hurt having seen that the team there knew what they were doing.

##  Integrating the Memfault SDK

Our reference modem is based on the nRF52840 SoC from Nordic, and we internally use the nRF5 SDK. This is one of the platforms for which the Memfault SDK provides a port, so hooking up the Memfault SDK in our application simply means:
* Pulling in the SDK as a Git Submodule
* Integrating it with our CMake build system
* Adding and customizing the `memfault_platform_*` files
* Enabling the GNU Build ID for each build
* Amending our linker script to dedicate a flash region to Memfault core dumps
* Calling the Memfault initialization routine (`memfault_platform_boot()`) on startup

All of the above is clearly documented in the [Memfault docs](https://docs.memfault.com/docs/mcu/arm-cortex-m-guide).

Something else that the SDK needs is a device identifier. As Blecon devices all automatically get a globally unique device identifier, we can re-use it for Memfault as well. Other information such as the firmware name and version and board name is all set as part of our build system.

```c
#include "memfault/components.h"
#include "blecon/blecon_client.h"

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {

    static char uuid_str[BLECON_UUID_STR_SZ] = "";
    blecon_client_get_uuid(uuid_str);

    *info = (sMemfaultDeviceInfo) {
        // Device ID
        .device_serial = uuid_str,

        // Firmware name
        .software_type = FIRMWARE_NAME,

        // Firmware version
        .software_version = FIRMWARE_VERSION,

        .hardware_version = BOARD_NAME,
    };
}
```

In order to test the integration at this stage, we can add a “feature” that generates a fatal assert in our test application at the press of a button on our NRF52840_DK. This triggers a core dump and a reset of the board.

```c
static void fake_error(void * p_event_data, uint16_t event_size) {
   bsp_board_leds_on();
   MEMFAULT_ASSERT(0); // Create fake error condition
}

/**@brief Function for handling events from the BSP module.
*
* @param[in]   event   Event generated by button press.
*/
void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_1:
            app_sched_event_put(NULL, 0, fake_error);
            break;

        default:
            break;
    }
}
```

There is a [GDB Script](https://docs.memfault.com/docs/mcu/test-data-collection-with-gdb/) that we can use to test data collection and submit these core dumps to the Memfault cloud manually. For this to work, we need to call `memfault_data_export_dump_chunks()`. Something worth noting is that on Nordic platforms the softdevice needs to be disabled at this stage, as well as the scheduler, however, the device will crash before the upload can complete (as GDB will stall the core for too long).

```c
int main(void)
{
    // ...
    memfault_platform_boot();

    // Upload here with GDB before softdevice and timers are started
    memfault_data_export_dump_chunks();

    scheduler_init();
    // ...
}
```

After uploading the test binary’s symbols to Memfault and trying it out, we can see the first coredump appearing in the Memfault app!

##  Adding Comms with Blecon

The Memfault SDK serializes core dumps, traces, and metrics and provides an API (`memfault_packetizer_get_chunk()`) that pulls them as chunks that can be sent to the Memfault cloud. The chunk size is specified by the caller, so we can easily deal with various MTUs depending on the transport used.

The next step is to integrate this Memfault chunks API with the Blecon network client.

When a Blecon device needs to send data, it requests a connection from the infrastructure. Once a nearby access point picks it up, a secure connection is established with our network service. A series of requests/responses can be made, and then the connection is torn down. Any arbitrary binary data can be sent through a Blecon network, which makes it simple to send the Memfault chunks.

At a high level this means:
* Check at startup and then periodically if there is Memfault data that needs to be sent
  * If so, request a Blecon network connection
* When a connection is made, send chunks through the Blecon network infrastructure
* Close the connection once all chunks have been sent

We amend our `main()` with the following:
```c
int main(void)
{
    ...

    // Request connection if we have data to upload
    if(memfault_packetizer_data_available()) {
        blecon_client_request_connection();
    }

    ...
```

And handle the various callbacks from the Blecon client:
```c
static void send_memfault_data_or_disconnect(void) {
    static uint8_t data[64] = {0};
    size_t data_sz = sizeof(data);

    if(!memfault_packetizer_get_chunk(data, &data_sz)) {
        // All done, disconnect
        NRF_LOG_INFO("No more data to send");
        blecon_client_close_connection(); // Nothing left to send/receive
        return;
    }

    NRF_LOG_INFO("Sending %u byte-chunk", data_sz);
    bool r = blecon_client_send_request(data, data_sz);
    if(!r) {
        NRF_LOG_WARNING("Could not send request");
        blecon_client_close_connection();
    }
}

static void blecon_on_connection(void* ctx) {
    NRF_LOG_INFO("Connected, sending request.");
    send_memfault_data_or_disconnect();
}

static void blecon_on_response(void* ctx) {
    NRF_LOG_INFO("Got response");
    send_memfault_data_or_disconnect();
}

static void blecon_on_error(void* ctx) {
    NRF_LOG_INFO("Got error %x", blecon_client_get_error());
	  blecon_client_close_connection(); // Abort
}
```

That’s it!

## Integrating Cloud APIs
Our device can now send Memfault data to the Blecon network infrastructure, however, we still need to route it to the Memfault cloud!

Blecon devices are associated with a particular network. When any of these devices make a request, it is routed to a request webhook that can be configured for the network.

This webhook has a specific data schema that includes data like Device ID, geographic location, plus the raw request data; in this case, the Memfault chunks.

This is an example of a request sent in a Blecon webhook:

```json
{
  "network_headers": {
    "message_type": "DEVICE_REQUEST",
    "account_id": "25YliRsXzU7MBnULDcmT9OzNec5",
    "network_id": "27WCHTAC9zeKqn2G2sxPSUc1YEh",
    "device_id": "172cbb8d-056f-4f7c-88b9-c0b2c755513f",
    "device_location": "52.2089745,0.0896902",
    "device_model_id": "0"
  },
  "request_data": {
    "payload": "4e6963652066696e642120456d61696c2075732061742068656c6c6f40626c65636f6e2e6e6574203a29"
  }
}
```

Memfault chunks would appear in the “payload” field which is hex encoded. A simple nodeJS app can extract the chunks and submit them to the Memfault API.

The app does the following:
* Check that an authentication header is correct
* Extract the device ID and the payload from the request
* Decode the payload from hex encoding to binary
* Set the Memfault project key
* Post to the Memfault chunk API

```javascript
'use strict';

const express = require('express');
const https = require('https');

const PORT = process.env.PORT || 3000;
const MEMFAULT_PROJECT_KEY = process.env.MEMFAULT_PROJECT_KEY;
const BLECON_SECRET = process.env.BLECON_SECRET;
const INDEX = '/index.html';

const server = express()
  .use(express.json())
  .post('/', async (req, res) => {
    if(req.get('Blecon-Secret') != BLECON_SECRET) {
      res.status(401).send('Unauthorized');
      return;
    }

    let device_id = req.body['network_headers']['device_id'];
    let payload = Buffer.from(req.body['request_data']['payload'], 'hex');;
    await sendToMemfault(device_id, payload);
    res.json({ response: "" });
  })
  .listen(PORT, () => console.log(`Listening on ${PORT}`));

function sendToMemfault(device_id, payload) {
  return new Promise((resolve, reject) => {
    console.log("Chunk: ", payload);

    const req = https.request('https://chunks.memfault.com/api/v0/chunks/' + device_id, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/octet-stream',
        'Memfault-Project-Key': MEMFAULT_PROJECT_KEY
      }
    }, res => {
      console.log("Status: " + res.statusCode);
      resolve();
    })
    req.on('error', reject);
    req.write(payload);
    req.end();
  });
}
```

Here is the code on Github: [https://github.com/blecon/blecon-memfault-integration](https://github.com/blecon/blecon-memfault-integration)

We deploy it to Heroku, and configure the Blecon network request handler’s URL and secret within the Blecon console.

<img width=600px src="{% img_url memfault-bluetooth/blecon-edit-request-handler.png %}" />

We then connect our device to the network and can now see the chunks coming into the chunks debug log within Memfault!

Now, when we generate an error on the device we see a proper stack trace in Memfault:

<img width=600px src="{% img_url memfault-bluetooth/memfault-assert-fake_error.png %}" />

## Moving to production
With this success, we’ve decided to integrate Memfault directly into our modem firmware.

As the request/response mechanism above is something we expose to the host MCU the modem is connected to, we’re using the following strategy: every time the modem connects, it first serves requests made by the host MCU application. Once this is done (or on timeout), any pending Memfault chunk is sent to the infrastructure, using an internal channel. These chunks are then forwarded to the Memfault Chunk API by our network service. As all data is authenticated and encrypted between the modem and cloud, the Memfault chunks are sent securely and we can guarantee that the chunks came from a specified device.

We’re also leveraging the other features provided by the platform (Heap allocation tracking, Compact logs, metrics, etc) which gives us a lot of visibility into any issue which arises. For instance, one of our reference devices seemed to stop working within 30 minutes of each charge. By looking at its metrics, we could see straight away it was likely to be a hardware issue as the battery voltage would drop significantly (and eventually below our SoC’s minimum voltage) once disconnected from external power.

## Conclusion

Adding Memfault to our Blecon modem implementation gives us the confidence to iterate fast, knowing that if something goes wrong in the field we will be able to find out what and issue a fix quickly.

Device observability is something that you don’t go back from once you’ve tried it. We remember the pain of dealing with customers facing issues that were hard to reproduce. Is it the hardware? Some specific environmental condition? The specific way the customer is using the device? All of these questions were difficult to answer, and in extreme cases, they would lead to someone being sent to the customer’s site to understand what was going on. They always led to a lot of time being spent that the engineering team would have rather used to improve the product.

As an engineer, along with firmware update capabilities, device observability gives you an amazing toolbox to build great products, fast.

I hope that this blog post gives you some good insight into how Memfault can be integrated with a non-IP transport like Blecon, and the benefits it can bring to your product development flow.

I’m also intrigued by the comments we’ve had that Blecon could be an easy way to enable Memfault for devices that don’t have connectivity. Adding a BLE module to your design won’t hurt your BOM or power consumption much, so could be an easy way to add wireless diagnostics to a product.

_If you are building products and interested in learning more about using Blecon with Memfault, drop us an email at [hello@blecon.net](mailto:hello@blecon.net). We are also currently running an early access program, so if you want to get a modem breakout board to try it yourself, you can apply at [https://www.blecon.net/early-access](https://www.blecon.net/early-access)_


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}
