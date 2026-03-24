---
title: Building a Matter Light Bulb from Scratch
description:
  Matter is the new standard for smart home devices, but the spec is dense and
  the examples are complex. In this post, I build a simple Matter light bulb
  on the nRF54LM20 DK from scratch, covering Thread networking, the OnOff and
  LevelControl clusters, and the handful of callbacks needed to make a device
  show up in Apple Home. The implementation is surprisingly small.
author: francois
tags: [matter, thread, iot, nrf54]
---

I've been digging into Matter lately (see [my last post]({% post_url 2026-03-23-matter-internet-connectivity %}) on internet connectivity from Thread devices). It's a neat protocol! The promise of "one standard for the smart home" is compelling, especially considering how clunky WiFi provisioning is today.

In order to learn more about the standard, I decided to write a simple Matter application. Hope you will enjoy coming along for the ride.

<!-- excerpt start -->

In this post, we build a Matter light bulb on the nRF54LM20 DK. We walk through how Matter works over Thread, what clusters are and how they define device behavior, and the actual code needed to make a device appear as a dimmable light in Apple Home.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Setup

### Hardware

For this project, I'm using the nRF54LM20 DK[^lm20dk] from Nordic Semiconductor. The nRF54LM20[^nrf54lm20] has an ARM Cortex-M33 at 128 MHz, 2 MB of RRAM, and 512 KB of RAM. It supports both BLE (for commissioning) and 802.15.4 (for Thread), which makes it a good fit for Matter.

The DK has four LEDs. I'll use LEDs 1 through 3 as the "light bulb" and leave LED 0 for Matter status indication (it blinks during commissioning).

> **Note:** You need to connect header P14 for the LEDs to work. If your LEDs do not light up, check this first. On my preview kit, it was not populated.

For the Matter hub, I have an Apple HomePod mini. It's a Thread Border Router, a Matter controller, and it costs about $99. In my experience it's the best bang-for-buck hub out there if you just want something that works.

```
┌─────────────┐    802.15.4     ┌───────────────────┐                ┌──────────────┐
│ nRF54LM20   │ ── Thread ───> │ HomePod mini      │ ── Wi-Fi ───> │ Apple Home   │
│ DK          │    (mesh)      │ (Border Router +  │                │ (iPhone/iPad │
│ LED1-3 🔆   │ <── Thread ─── │  Matter Controller)│ <── Wi-Fi ─── │  app)        │
└─────────────┘                └───────────────────┘                └──────────────┘
```

### Software

Nordic devices come with the excellent nRF Connect SDK (NCS)[^ncs], which includes the Matter stack (connectedhomeip), OpenThread, Zephyr RTOS, and all the board support you need. Follow [Nordic's documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/installation/install_ncs.html) to get NCS installed.

Nordic provides several Matter example applications in NCS. I started from the bare `template` sample, which gives you a minimal Matter device with commissioning support but no application-level clusters:

```
~/ncs/main/nrf/samples/matter/template/
```

### Build Configuration

During development, I got sick of having to re-commission my device in Apple Home every time I reflashed it. The Matter sample stores data in flash, and it was getting over-written by `west flash`.

The trick is to use a static partition map that places the factory data and settings storage in dedicated flash regions that are not touched during a firmware update:

```yaml
# pm_static_nrf54lm20dk_nrf54lm20a_cpuapp.yml
app:
  address: 0xD800
  region: flash_primary
  size: 0x1E2800
factory_data:
  address: 0x1F0000
  region: flash_primary
  size: 0x1000
settings_storage:
  address: 0x1F1000
  region: flash_primary
  size: 0xC000
```

With this layout, `west flash` only erases and rewrites the `app` region. The commissioning data in `factory_data` and `settings_storage` survives across builds, so you only need to commission once.

## How Matter over Thread Works

If you are new to Matter, it is worth understanding the key pieces before diving into code.

### Thread

Thread[^thread_spec] is a low-power mesh networking protocol built on IEEE 802.15.4 (the same radio layer as Zigbee). It provides IPv6 addressing, mesh routing, and encrypted communication. Thread networks are self-healing: if a node goes down, the mesh reroutes automatically.

A Thread network has a few roles:

- **Leader**: One device coordinates the network (distributes addresses, manages routing). This role is elected automatically.
- **Router**: Devices that forward packets for other nodes. Any mains-powered Thread device typically becomes a router.
- **End Device**: Battery-powered devices that talk to a single parent router and sleep between transmissions.
- **Border Router**: A special router that bridges traffic between the Thread mesh and your home IP network (Wi-Fi/Ethernet). In my setup, the HomePod mini is the Border Router.

The DK, being mains-powered, will act as a router.

### Controllers and Fabrics

Matter adds a layer on top of Thread (or Wi-Fi). When you commission a device, it joins a *fabric*, a logical grouping of devices and controllers that share encryption keys. The controller (the HomePod) manages the fabric and orchestrates all communication.

Once commissioned, the controller can read attributes from the device, write attributes to it, and send commands. The device does not initiate communication to the controller, it just responds to requests and pushes subscription updates.

### Clusters

Matter defines device behavior through *clusters*. A cluster is a collection of related attributes, commands, and events. Think of it as an interface definition.

For example:

| Cluster                    | What it does |
|----------------------------|--------------|
| **OnOff**                  | A single boolean attribute (`OnOff`), plus `On`, `Off`, and `Toggle` commands |
| **LevelControl**           | A brightness level (0–254), plus `MoveToLevel`, `Move`, and `Step` commands |
| **ColorControl**           | Hue, saturation, and color temperature attributes |
| **TemperatureMeasurement** | A `MeasuredValue` attribute (read-only) |
| **DoorLock**               | Lock/unlock commands, lock state attribute |

A device exposes one or more *endpoints*, and each endpoint implements a set of clusters. A light bulb, for example, typically has endpoint 1 with the OnOff and LevelControl clusters. A thermostat would have TemperatureMeasurement and Thermostat clusters.

The full list of clusters is defined in the Matter Application Cluster specification[^matter_spec]. There are dozens of them, covering everything from window coverings to air quality sensors.

## The Light Bulb Clusters

Lightbulbs need two Matter clusters:

### OnOff

The OnOff cluster is about as simple as it gets. It has one attribute:

- `OnOff` (boolean): whether the light is on or off.

And three commands:

- `On`: sets `OnOff` to true
- `Off`: sets `OnOff` to false
- `Toggle`: flips `OnOff`

When the controller sends an `On` command, the Matter stack updates the `OnOff` attribute and calls the application callback.

The attribute is also *persistent*. The Matter stack stores it in non-volatile memory, so if you power-cycle the device, it remembers whether the light was on or off.

### LevelControl

Dimming is implemented with the LevelControl cluster. Its key attribute is:

- `CurrentLevel` (uint8, 0–254): the brightness level.

It also has commands like `MoveToLevel` (set to a specific brightness) and `Move` (gradually change brightness over time). The controller handles transition animations. The device just needs to react to the final level value.

With both clusters, the device shows up in Apple Home as a dimmable light with a brightness slider.

## ZAP Files

> **Note:** This is a bit of a side-quest. Feel free to [skip to the implementation](#the-implementation) and simply copy the ZAP file Nordic bundles with their own lightbulb example, which is what I initially did.

Every Matter device needs a *data model* that declares which endpoints exist, which clusters each endpoint implements, and which attributes and commands are enabled. The Matter SDK uses a tool called ZAP (ZCL Advanced Platform) to define this model. You describe your device in a `.zap` file (which is JSON under the hood), and the ZAP code generator produces C++ source files that the Matter stack compiles and links against.

A ZAP file has a few top-level keys:

```json
{
  "featureLevel": 103,
  "creator": "zap",
  "keyValuePairs": [ ... ],
  "package": [ ... ],
  "endpointTypes": [ ... ],
  "endpoints": [ ... ]
}
```

The interesting parts are `endpointTypes` and `endpoints`. Each entry in `endpointTypes` defines a type of endpoint (what clusters it has, what attributes are enabled, what defaults are set). Each entry in `endpoints` binds an endpoint type to an endpoint ID.

For a light bulb, you need at least two endpoint types:

1. **Root Node (endpoint 0)**: mandatory for every Matter device. It includes clusters like Basic Information, General Commissioning, Network Commissioning, and a dozen others that handle device identity, commissioning, and diagnostics.
2. **Dimmable Light (endpoint 1)**: the application endpoint. It includes OnOff, LevelControl, Identify, Groups, and Descriptor clusters.

The root node alone is about 4,000 lines of JSON. A dimmable light endpoint adds another 1,000 lines. Most of this is boilerplate: attribute definitions with default values, min/max ranges, and reporting configuration.

The ZAP files are used to generate two artifacts: a .matter file, which is a human-readable IDL (Interface Definition Language) describing the device's clusters, attributes, and
  commands — used by the Matter SDK's code generator to produce additional C++ scaffolding at build time — and a set of C++ source files (zap-generated/) that define the endpoint
  configuration, attribute tables, and command handlers that get linked into your firmware.

NCS ships pre-generated .matter files and C++ code alongside the ZAP files in their samples, so you can build without needing to use the matter toolchain. But for a custom device with different
  clusters or attributes, you need `zap-cli` to regenerate these artifacts from your modified ZAP file.

I did not find using pre-generated files satisfying, so decided to figure out how to generate them myself.

> **Note**: ZAP files may eventually get replaced with .matter files altogether, but this work is still in progress. Learn more at https://gautesl.github.io/connectedhomeip/docs/code_generation.html.

### Composable ZAP Fragments

Rather than edit a multi-thousand-line JSON file, I wanted a more composable system. The root node is the same for every Matter device, so it should live in one file. Each application endpoint (light, sensor, lock) should be its own file, which are then fed into some build tool and merged to generate the final ZAP file at compile time.

I wrote two small Python scripts to make this work:

- [**`zap_decompose.py`**](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/zap_decompose.py): takes a monolithic ZAP file and splits it into a base fragment (root node + global config) and one fragment per application endpoint.
- [**`zap_compose.py`**](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/zap_compose.py): takes a base fragment and one or more endpoint fragments and produces a valid ZAP file.

It loads the base, appends each fragment's endpoint types, and rebuilds the endpoint list with sequential IDs.

I ended up with two fragments:

- `zap/base.json`: the root node (15 clusters, ~4,000 lines)
- `zap/ma_dimmablelight.json`: the dimmable light endpoint (5 clusters: OnOff, LevelControl, Identify, Groups, Descriptor, ~1,000 lines)

The rest (final ZAP file, but also the auto-generated .matter and C++ files which the matter toolchain spits out from ZAP) are all generated at build.

### Writing Your Own Fragment

The dimmable light fragment I got from Nordic's sample is ~1,000 lines, but most of that is boilerplate. If you wanted to write a minimal on/off light from scratch (no dimming), the structure is straightforward.

A fragment is a JSON file with a single key, `endpointTypes`, containing an array with one entry. That entry has a `name`, a `deviceTypes` array (which tells the Matter stack what kind of device this is), and a `clusters` array:

```json
{
  "endpointTypes": [
    {
      "name": "MA-onofflight",
      "deviceTypes": [
        {
          "code": 256,
          "profileId": 259,
          "label": "MA-onofflight",
          "name": "MA-onofflight"
        }
      ],
      "clusters": [
        ...
      ]
    }
  ]
}
```

The `deviceTypes` code is defined in the Matter spec. 256 is an On/Off Light, 257 is a Dimmable Light, 770 is a Temperature Sensor, and so on. This is how the controller knows what icon to show and what controls to offer.

Each cluster in the `clusters` array has a `name`, a numeric `code`, a `side` (always `"server"` for clusters your device implements), and arrays of `attributes` and `commands`. Here is what a minimal OnOff cluster looks like:

```json
{
  "name": "On/Off",
  "code": 6,
  "side": "server",
  "enabled": 1,
  "attributes": [
    {
      "name": "OnOff",
      "code": 0,
      "side": "server",
      "type": "boolean",
      "included": 1,
      "storageOption": "NVM",
      "defaultValue": "0x00",
      "reportable": 1,
      "minInterval": 0,
      "maxInterval": 65344,
      "reportableChange": 0
    },
    {
      "name": "FeatureMap",
      "code": 65532,
      "side": "server",
      "type": "bitmap32",
      "included": 1,
      "storageOption": "RAM",
      "defaultValue": "0"
    },
    {
      "name": "ClusterRevision",
      "code": 65533,
      "side": "server",
      "type": "int16u",
      "included": 1,
      "storageOption": "RAM",
      "defaultValue": "6"
    }
  ],
  "commands": [
    { "name": "Off",    "code": 0, "source": "client", "isIncoming": 1, "isEnabled": 1 },
    { "name": "On",     "code": 1, "source": "client", "isIncoming": 1, "isEnabled": 1 },
    { "name": "Toggle", "code": 2, "source": "client", "isIncoming": 1, "isEnabled": 1 }
  ]
}
```

A few things to notice:

- The `OnOff` attribute has `"storageOption": "NVM"`, which tells the Matter stack to persist it across reboots. This is why the `emberAfOnOffClusterInitCallback` can read back the stored value at boot.
- `FeatureMap` and `ClusterRevision` are mandatory for every cluster. The Matter stack uses them during commissioning to tell the controller what features are supported.
- Commands have `"source": "client"` and `"isIncoming": 1`, meaning they originate from the controller and are received by the device.

For a complete on/off light, you also need a Descriptor cluster (mandatory for every endpoint) and typically Identify and Groups. But the pattern is the same: declare the cluster, list the attributes you want, and enable the commands.

If you want to go beyond what the spec defines (say, a custom cluster for proprietary functionality), you can add manufacturer-specific clusters with a `mfgCode` field. For standard device types, the attribute codes, cluster codes, and command codes are all in the Matter Application Cluster specification[^matter_spec].

The easiest way to get started is to decompose an existing NCS sample that is close to what you want, then edit the fragment: add or remove clusters, change default values, or flip `storageOption` from `"RAM"` to `"NVM"` for attributes you want persisted.

> **Note:** If editing JSON by hand sounds tedious, there is also a GUI tool. The ZAP project ships an Electron-based editor (`zap-cli gui`) that lets you browse clusters, toggle attributes, and configure endpoints visually. It works on the composed `.zap` file, so you can use it alongside the fragment workflow: compose your fragments, open the result in the GUI to make changes, then decompose it back into fragments.

### Build Integration

The ZAP compose script and code generation runs at CMake configure time. I added a small shell script ([`zap_generate.sh`](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/zap_generate.sh)) that chains the steps:

1. Compose `base.json` + `ma_dimmablelight.json` into `matter_echo.zap`
2. Run the ZAP code generator to produce the C++ cluster implementation (`zap-generated/*.cpp/h`)
3. Run the ZAP code generator again to produce the Matter IDL file (`matter_echo.matter`)

All output goes to `build/zap/`, so nothing generated ends up in the source tree. The generation takes about 8 seconds and only runs on a clean build.

In `CMakeLists.txt`:

```cmake
set(ZAP_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/zap)
if(NOT EXISTS ${ZAP_GEN_DIR}/matter_echo.zap)
  execute_process(
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tools/zap_generate.sh ${ZAP_GEN_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE ZAP_GEN_RESULT
  )
  if(NOT ZAP_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "ZAP generation failed")
  endif()
endif()
```

And in `prj.conf`, I point at the generated output:

```kconfig
CONFIG_NCS_SAMPLE_MATTER_ZAP_FILE_PATH="${APPLICATION_CONFIG_DIR}/build/zap/matter_echo.zap"
```

## The Implementation

At long last, let's write some code. The implementation itself is relatively straightforward.

### Project Configuration

Here is a minimal `prj.conf` for a Matter light bulb:

```kconfig
# prj.conf

# Enable Matter
CONFIG_CHIP=y
CONFIG_CHIP_PROJECT_CONFIG="src/chip_project_config.h"
CONFIG_CHIP_DEVICE_PRODUCT_NAME="Matter Light"
CONFIG_STD_CPP17=y

# ZAP data model -- generated from composable fragments at build time
CONFIG_NCS_SAMPLE_MATTER_ZAP_FILE_PATH="${APPLICATION_CONFIG_DIR}/build/zap/matter_echo.zap"

# PWM for dimmable LEDs
CONFIG_PWM=y
CONFIG_CHIP_DEVICE_PRODUCT_ID=32768

# BLE commissioning
CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART=y
CONFIG_CHIP_BLE_EXT_ADVERTISING=y
CONFIG_CHIP_BLE_ADVERTISING_DURATION=60
CONFIG_BT_DEVICE_NAME="MatterLight"

# DK LEDs/buttons
CONFIG_DK_LIBRARY=y

# Factory data (survives reflash)
CONFIG_CHIP_FACTORY_DATA=y
CONFIG_CHIP_FACTORY_DATA_BUILD=y

# Logging
CONFIG_LOG=y
CONFIG_LOG_MODE_DEFERRED=y
```

The important bits are `CONFIG_CHIP=y` (enables the Matter stack), the ZAP file path (defines the data model), and `CONFIG_PWM=y` (for LED dimming). Everything else is standard NCS configuration.

### Device Tree Overlay and Light Module

The nRF54LM20 DK does not have PWM configured for the LEDs by default, so I added a [device tree overlay](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/nrf54lm20dk_nrf54lm20a_cpuapp.overlay) that maps three PWM channels to LED pins P1.25, P1.27, and P1.28 with a 20 ms period.

The light module itself ([`light.c`](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/light.c), [`light.h`](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb/light.h)) is pure C and exposes a simple API:

```c
void light_init(void);
void light_set(bool on);
bool light_get(void);
void light_toggle(void);
void light_set_level(uint8_t level);
uint8_t light_get_level(void);
```

Under the hood it drives the three PWM channels, mapping the Matter brightness level (0–254) to a pulse width. Nothing fancy.

### Matter Callbacks

This is the glue between the Matter stack and the light module. When the controller changes an attribute (e.g., turns the light on), the Matter stack calls `MatterPostAttributeChangeCallback`. The code below checks which attribute changed and update the LEDs accordingly:

```c
/* src/zcl_callbacks.cpp */

extern "C" {
#include "light.h"
}

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(
    const chip::app::ConcreteAttributePath &attributePath,
    uint8_t type, uint16_t size, uint8_t *value)
{
    ClusterId clusterId = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == OnOff::Id &&
        attributeId == OnOff::Attributes::OnOff::Id) {
        light_set(*value ? true : false);
    }

    if (clusterId == LevelControl::Id &&
        attributeId == LevelControl::Attributes::CurrentLevel::Id) {
        light_set_level(*value);
    }
}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    bool storedValue;
    auto status = OnOff::Attributes::OnOff::Get(endpoint, &storedValue);

    if (status == Protocols::InteractionModel::Status::Success) {
        light_set(storedValue);
    }
}
```

I know, I know, I'm not any happier than you are to see this written in C++. Unfortunately, the matter SDK is written in C++ and provides no C API :-(.

There are two callbacks:

- `MatterPostAttributeChangeCallback`: called whenever an attribute changes. I handle OnOff (turn LEDs on/off) and LevelControl (set brightness).
- `emberAfOnOffClusterInitCallback`: called when the OnOff cluster initializes at boot. I read the persisted value and apply it to the LEDs, so the light comes back in the same state it was in before a power cycle.

### Application Task

The application task initializes the Matter stack, registers a button handler, and runs the event loop:

```c
/* src/app_task.cpp */

#include "app_task.h"

extern "C" {
#include "light.h"
}

#include <dk_buttons_and_leds.h>

#include "app/matter_init.h"
#include "app/task_executor.h"
#include "board/board.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;

CHIP_ERROR AppTask::Init()
{
    ReturnErrorOnFailure(Nrf::Matter::PrepareServer());

    /* Light endpoint ID must match the ZAP file (endpoint 2) */
    static constexpr EndpointId kLightEndpointId = 2;

    auto buttonHandler = [](uint32_t buttonState, uint32_t hasChanged) {
        if (hasChanged & DK_BTN1_MSK) {
            if (buttonState & DK_BTN1_MSK) {
                light_toggle();
                /* Sync to Matter cluster so Apple Home sees the change */
                Clusters::OnOff::Attributes::OnOff::Set(
                    kLightEndpointId, light_get());
            }
        }
    };

    if (!Nrf::GetBoard().Init(buttonHandler)) {
        LOG_ERR("Board initialization failed.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Initialize light AFTER Board init to avoid pin conflicts */
    light_init();

    ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(
        Nrf::Board::DefaultMatterEventHandler, 0));

    return Nrf::Matter::StartServer();
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    while (true) {
        Nrf::DispatchNextTask();
    }

    return CHIP_NO_ERROR;
}
```

The button handler toggles the LEDs and writes the new state back to the Matter attribute. This keeps the controller's UI in sync.

### Entry Point

The entry point is a one-liner:

```c
/* src/main.cpp */

#include "app_task.h"

int main()
{
    return AppTask::Instance().StartApp() == CHIP_NO_ERROR
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

### Building and Flashing

Build with west, targeting the nRF54LM20 DK:

```
west build -b nrf54lm20dk/nrf54lm20a/cpuapp -- -DFILE_SUFFIX=matter
west flash
```

If this is your first flash, you will also need to build and flash the factory data and MCUboot bootloader. Subsequent flashes only need to update the application.

## Trying It Out

After flashing, connect to the UART console and look for the commissioning URL:

```
[00:00:00.140,606] <inf> chip: [SVR]https://project-chip.github.io/connectedhomeip/qrcode.html?data=...
```

Open that URL in your browser to get a QR code, then scan it with the Apple Home app on your iPhone. The device should appear as "Matter Light" and commission into your Thread network.

Once commissioned, you can:

- **Toggle from Apple Home**: tap the light in the Home app. The three LEDs on the DK should turn on/off.
- **Adjust brightness**: long-press the light in Apple Home and drag the brightness slider. The LEDs dim smoothly via PWM.
- **Toggle from the button**: press Button 1 on the DK. The LEDs toggle, and Apple Home updates to reflect the new state.

```
uart:~$ light status
Light is ON, level 254

uart:~$ light level 50
Light level 50

uart:~$ light toggle
Light OFF (level 50)
```

## Conclusion

Phew, I learned a lot about Matter putting this together. In the end, building a Matter light bulb is straightforward! Indeed, had I not gotten sucked into the ZAP file side-quest, it would have taken no more than an hour or two.

This firmware build stands on the shoulders of giants. Below our 200 lines of application code lies tends of thousands of lines of C++ implementing the Matter spec. The resulting firmware binary is almost 1MB! So while this was relatively little work, it is a complex firmware.

I hope you found it informative, and that you understand matter better today than you did yesterday.

As always, I would love to hear about your experience in the comments. The code for this post is available on [GitHub](https://github.com/memfault/interrupt/tree/master/example/matter-light-bulb).

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^matter_spec]: [Matter Specification](https://csa-iot.org/developer-resource/specifications-download-request/)
[^connectedhomeip]: [connectedhomeip - Matter SDK](https://github.com/project-chip/connectedhomeip)
[^thread_spec]: [Thread Group: What is Thread](https://www.threadgroup.org/What-is-Thread)
[^nrf54lm20]: [nRF54LM20 Product Page](https://www.nordicsemi.com/Products/nRF54LM20)
[^lm20dk]: [nRF54LM20-DK Product Page](https://www.nordicsemi.com/Products/Development-hardware/nRF54LM20-DK)
[^ncs]: [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK)
<!-- prettier-ignore-end -->
