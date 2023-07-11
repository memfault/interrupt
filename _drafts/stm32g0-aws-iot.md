---
title: Integrating Memfault in a Device-to-Cloud Embedded Design using STMicroelectronics STM32G0 and AWS IoT ExpressLink
description:
  Post Description (~140 words, used for discoverability and SEO)
author: dangross
---

In the dynamic realm of embedded systems, the right combination of hardware
and software components can transform the development process and empower
engineers to build robust and efficient solutions. This article explores
a streamlined device-to-cloud embedded design utilizing the STM32G0
Nucleo board from STMicroelectronics, an AWS IoT ExpressLink module from
Espressif, AWS IoT Core for secure MQTT communication, and Memfault for
remote debugging. We will delve into the seamless integration of these
technologies and highlight the benefits they offer in the context of IoT
applications.

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

First, let's take a look at the high-level architecture
this article will cover with the primary components identified along
with how they fit together. The device side is primarily hardware, but
will also include your custom firmware as well as any specific device
configuration. The cloud side leverages AWS services as a bridge to
Memfault.

![]({% img_url stm32g0-aws-iot/arch_diagram.png %})

In this design, the STM32G0 Nucleo board serves as a versatile
development platform based on the STM32G0 series of microcontrollers.
The STM32G0 is a low-cost, low-power, 32-bit MCU based on the Arm Cortex-M0+.
It offers a solid foundation for prototyping and developing IoT applications.
Peripherals such as sensors and actuators can be easily attached to suit the
intended behavior of the device. STMicroelectronics has a rich and extensible
development environment with STM32CubeMX and STM32CubeIDE that can be used
for the STM32G0. In conjunction with these tools, STMicroelectronics also
developed the I-CUBE-ExpressLink expansion to make developing firmware
with AWS IoT ExpressLink connectivity modules incredibly easy. The board
comes complete with Arduino headers, which is a perfect interface to
plug in the AWS IoT ExpressLink development kit from Espressif.

Before we dive into the specifics of the aforementioned connectivity module
available from Espressif, let's cover some basic information about AWS
IoT ExpressLink. AWS IoT ExpressLink is a hardware and software solution
available through qualified AWS partners to provide connectivity to AWS
with very little coding and very few dependencies. This comes in the
form of a hardware module with software built in that can connect to AWS
IoT Core securely with pre-provisioned keys and certificates. The
interface to the module is a serial UART and implements a small AT
command set. In this way, the module acts like a "cloud modem." To date,
several AWS IoT ExpressLink modules are in production with cellular and
Wi-Fi connectivity available from these AWS partners: Espressif,
Infineon, Realtek, Telit Cinterion, and u-blox.

In this article, we are using the Espressif dev kit with Wi-Fi connectivity.
You can follow the links above to discover various procurement options.
Configuration of the dev kit is straightforward by following the steps in
this guide from Espressif. At a minimum, AWS IoT Core must be provided the
device certificate and name, and the module must be provided the MQTT endpoint
along with the Wi-Fi credentials. Once you finish the configuration,
simply plug in the dev kit to the STM32G0 Nucleo board as shown in the
setup below.

![]({% img_url stm32g0-aws-iot/board_picture.png %})

To program the firmware of the device, you can leverage the
I-CUBE-ExpressLink expansion mentioned earlier in the article. To set up
your development environment, follow the instructions from
STMicroelectronics in this guide. The steps include the installation of
several packs in your development environment. There are examples and
code provided so you can get your device connected to AWS IoT Core and
publish data over MQTT topics. Of course, you can connect different
kinds of sensors to the device and capture the readings within your code
to send as data to the cloud for processing as well.

![]({% img_url stm32g0-aws-iot/aws_iot_screenshot.png %})

To ensure the reliability and stability of IoT applications, effective
debugging and error monitoring are crucial. Memfault comes to the rescue
with its comprehensive set of tools for remote debugging, crash
reporting, and real-time error monitoring. By integrating Memfault into
the STM32G0 Nucleo and AWS IoT ExpressLink combination, developers gain
valuable insights into application crashes, exceptions, and faults. This
integration enables proactive error detection, efficient bug fixing, and
improved system performance, resulting in more robust and reliable IoT
applications. You can get started with a free Memfault account.

On the device side, the Memfault SDK can be used to incorporate
Memfault into the firmware code to enable the remote debugging
capabilities. When an application encounters an error, the Memfault SDK
captures relevant information and splits it into smaller chunks. These
chunks are then sent securely to the Memfault API via AWS IoT Core and
MQTT. By dividing the data into manageable chunks, developers can ensure
efficient and reliable transmission, even in resource-constrained
environments. This approach enables comprehensive error reporting,
aiding in quicker issue resolution and enhancing the overall stability
of IoT applications.

However, how does the MQTT broker forward the data
to the Memfault API? This is where the Rules Engine in AWS IoT Core
comes into play along with a custom AWS Lambda function for calling the
Memfault API directly. The Memfault docs provide an example of the
recommended rule setup which looks like this:

```
SELECT encode(\*,'base64') AS data, topic() AS topic FROM 'prod/+/+/memfault/\#'
```

Another example of this configuration and setup can be found here from
Nordic Semiconductor as an asset tracking use case.

At this point, we have covered all the components of the architecture
in the diagram at the beginning of the article. You should have a complete
picture of how the hardware, software, connectivity, and services all
fit together in this device-to-cloud embedded design. This design allows
you to interact with the Memfault dashboard in both development and
production to gain insights into the device's behavior.

![]({% img_url stm32g0-aws-iot/memfault_screenshot.png %})

By harnessing the combined power of the STM32G0 Nucleo board, AWS IoT
ExpressLink, and Memfault, developers can unlock new frontiers in IoT
application development. This integration empowers engineers to
prototype, debug, monitor, and securely connect their devices while
facilitating efficient data processing and analysis. Leveraging cloud
services and scalable infrastructure, IoT applications built using this
combination can revolutionize domains like home automation, industrial
monitoring, smart agriculture, and beyond. The fusion of these
technologies propels innovation, enabling the creation of robust and
future-proof IoT solutions.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}
