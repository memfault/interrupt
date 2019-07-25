---
title: "A BLE Primer"
author: mafaneh
---

## Introduction
It's hard to imagine our lives nowadays without ...

I used to be obsessed with audio quality and finding the best sounding headphones (in-ear sound isolating earphones). I was a Westone customer for many years (a US-based company known for their high quality products in different applications including: hearing aids, music, hearing protection, and military). In fact, if you're looking for great quality sounding earphones, I would highly recommend you check them out (no affiliation, just a happy customer!): www.westone.com.

When Apple released the AirPods in December of 2016, I jumped on the bandwagon and pre-ordered them right off the bat. Apple sold me on the simplicity of connectivity, the relatively long battery life, and reliability.

Fast forward 2.5 years and I've never been happier. There were tradeoffs for sure, but the benefits outweighed the drawbacks (at least for me, personally). I sacrificed quality in favor of convenience and practicality. I suspect there are others who would do the same and still be happy!

So you may be asking, where am I going with all this? 

IMO, this is what will help wireless succeed in the marketplace (especially for consumer products). For industrial, enterprise, and commercial, it will have to prove its robustness, reliability, and more importantly its ROI.

People are willing to sacrifice some quality and aspects they wouldn't consider sacrificing when they try wireless. The convenience factor is huge! Unfortunately, it sometimes comes at a steep price (like in the AirPods example). Over time, however, wireless will only get cheaper and more affordable.

Wireless is changing the world, and it's providing many benefits that help make our lives easier. I'm not talking just about the consumer applications, but rather applications that make your life simpler and easier and completely in the background. Technology is best utilized when it happens in the background without the user ever being aware of its existence.

Wireless provides the following benefits:

- Removing wires
- Freedom and flexibility
- New use cases. Objects that cannot be connected with a wire, due to design factors or physical size limitations 
- Convenience, especially with remote or long-distance access
- It has the potential of saving lives! Think of a use case where an array of sensors are placed on the top of an electric pole, on the edge of a cliff, or even in a dam. Having the ability to collect data and control devices placed in these unsafe areas can reduce the safety risk significantly.  
- 

Nothing is perfect, so wireless has its drawbacks too:

- Sacrificing quality in some cases (compression, limited bandwidth).
- Batteries and the need to recharge devices.
- Reliability concerns.
- Security (easier to sniff packets and listen in on communication when you don't need physical access to the device).
- From a developer's perspective, it's much harder to develop and test wireless systems.
- High power consumption (in some use cases).
- Lack of interoperability in many cases (especially across different protocols and standards). This leads to more complicated integration between wireless networks.

## Wireless Technologies and the Internet of Things (IoT)
You've probably heard of the term IoT (Internet of Things), and likely seen the hype in the media surrounding this somewhat confusing term. In basic terms, IoT refers to connecting devices to each other and to the Internet/Cloud (whether it's private or the mass Internet). It also implies wireless connectivity for devices and potentially adding connectivity to a slew of devices that were never connected before (even [flip-flops](https://www.geek.com/tech/smart-flip-flops-keep-brand-buyers-connected-1694504/)!).

Wireless technologies are a dime a dozen, each with their own quirks and features. Some have been around for a long time, and some have only popped up recently. We'll spare the comparison between the different ones to a different article, but some of most interesting and applicable wireless technologies to embedded developers are:

- Wi-Fi
- Bluetooth Classic & Bluetooth Low Energy
- Zigbee
- Z-Wave
- LoRaWAN
- ANT/ANT+
- NFC
- Infrared (IR)

Most of these focus on the low-power aspect and are suitable for embedding into devices that need to run on small batteries for long periods of time. Applications include: industrial sensors, medical devices, wearables, and more.

In today's post, we'll be covering one of the most popular low-power IoT technologies: Bluetooth Low Energy (BLE). We'll cover:

- The basics of Bluetooth Low Energy
- A comparison between BLE and Bluetooth Classic
- BLE Peripherals and Centrals
- Advertising
- Connections
- The Generic Access Profile (GAP)
- The Generic Attribute Profile (GATT), including Services and Characteristics
- Implementation of a simple BLE Peripheral application on the nRF52 development kit
- Interacting with the BLE Peripheral from a mobile phone



## Simple BLE example on the nRF52 platform
Now that we've gone through the basics of BLE, we can take that knowledge and apply it to practice. In this section, we will take on an implementation of a simple BLE Peripheral running on the nRF52840 development kit.

Rather than relying on an IDE, we'll be using the command line for almost everything, and the steps should pretty much work on any Linux or macOS machine.

Let's go through the different software packages needed. If you've read and followed the steps in our previous post on [*Debugging Firmware with GDB*](https://interrupt.memfault.com/blog/gdb-for-firmware-1), you would already have all the necessary software packages installed and set up.

## Software required:
Here's a list of the software packages required for following the steps in this tutorial:

- [Nordic Semiconductor's nRF5 SDK](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK/Download#infotabs) (we'll be using SDK version 15.3.0).
- [Nordic Semiconductor's nRF5 Command Line Tools for macOS](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF5-Command-Line-Tools/Download#infotabs) (we'll be using version 9.8.1).
- [SEGGER J-Link Software for macOS](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) (we'll be using version V6.44g).
- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) (we'll be using version 7-2017-q4-major which is recommended by Nordic per [this article](https://devzone.nordicsemi.com/b/blog/posts/using-gdb-with-nordic-devices)).
- [CoolTerm serial terminal program](https://freeware.the-meiers.org/CoolTermMac.zip) (or any other serial terminal program).
- Xcode Command Line Tools – [For installation steps, I found this tutorial quite helpful and easy to follow](https://www.embarcadero.com/starthere/berlin/mobdevsetup/ios/en/installing_the_xcode_command_line_tools_on_a_mac.html).

## Setup
Let’s go through the different steps for downloading the necessary software packages.

For each of these downloads, you can place them anywhere on your machine. I recommend having them all in a single folder, to make it easier to locate later on.

Download SDK
Download nRF command line tools
Navigate to examples/ble_peripheral/

nrfjprog -f NRF52 --eraseall
nrfjprog -f NRF52 --program /Users/mafaneh/Projects/nRF5_SDK_15.3.0_59ac345/components/softdevice/s140/hex/s140_nrf52_6.1.1_softdevice.hex --chiperase --verify
nrfjprog -f NRF52 --program _build/nrf52840_xxaa.hex --verify
nrfjprog -f NRF52 --reset

Start with ble_app_template

Add code to handle button press (button 1 on dev board)

```
//Configure 1 button with pullup and detection on low state
#define NUM_OF_BUTTONS 1
static const app_button_cfg_t app_buttons[NUM_OF_BUTTONS] =
{
    {BUTTON_1, false, BUTTON_PULL, button_event_handler},
};
```

Add button event handler

```
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case BUTTON_1:
            NRF_LOG_INFO("Button 1 %s\r\n", button_action == 1 ? "pressed":"released");
            button_1_characteristic_update(&m_simple_service, &button_action);
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }

}
```

Use APP_Button instead of BSP button
Change:

```
/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
```

To

```
/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init()
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    //init app_button module, 50ms detection delay (button debouncing)
    err_code = app_button_init((app_button_cfg_t *)app_buttons,
                                   NUM_OF_BUTTONS,
                                   APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}
```


