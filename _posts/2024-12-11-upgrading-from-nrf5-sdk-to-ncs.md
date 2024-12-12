---
title:
  How to Transition from nRF5 SDK to Zephyr NCS － Lessons from Ultrahuman’s
  Journey
description:
  Ultrahuman's account of doing a complex migration from nRF5 SDK to Zephyr NCS
  on a nRF52 project
author: gauravsingh
---

At [Ultrahuman](https://www.ultrahuman.com/), innovation is at the core of
everything we do. Our health devices, powered by the nRF52840 SoC for BLE
functionality, rely on Nordic Semiconductor’s renowned wireless technology. For
years, the
[nRF5 SDK](https://www.nordicsemi.com/Products/Development-software/nRF5-SDK)
was the cornerstone of firmware development for these chipsets, but in 2018,
Nordic introduced the
[nRF-Connect SDK](https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK)
(NCS), built on Zephyr RTOS, signaling a new era for BLE applications.

As a health tech company delivering cutting-edge metabolic and fitness insights,
we recognized the need to modernize. The challenge? Migrating devices already in
users’ hands from FreeRTOS-based firmware to a Zephyr-based application without
disrupting their experience. This transition wasn’t just a technical upgrade—it
was a critical step toward leveraging advanced BLE capabilities and third-party
algorithms to provide more accurate health data.

<!-- excerpt start -->

In this article, we’ll walk you through our journey, the challenges we faced,
and how this migration is shaping a better future for those who rely on our
technology to improve their health and fitness.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## The Vision Behind the Transition

When Ultrahuman launched its flagship health-tracking devices, FreeRTOS provided
the lightweight real-time capabilities we needed. However, as our technology
matured, so did our aspirations. We wanted to offer:

- **Superior BLE performance**: With Zephyr's robust BLE stack, we could achieve
  more stable connections and seamless interactions across various mobile
  platforms, especially iOS.
- **Improved health insights**: The integration of sophisticated third-party
  algorithms for metrics like heart rate variability, sleep patterns, and
  metabolic tracking demanded a more modular and scalable RTOS.
- **Future-proofing**: Zephyr's growing ecosystem and support for modern
  embedded hardware made it the ideal choice for long-term innovation.

But making the shift wasn’t going to be easy—especially with devices already out
in the wild.

## The Early Days: Challenges Galore

We knew this wouldn’t be a standard firmware update. Here’s why:

### 1\. Complexity of Migration

FreeRTOS and Zephyr have entirely different architectures. From task scheduling
to memory management, porting the application logic was like translating a book
from one language to another—with no dictionary. Our team spent sleepless nights
understanding how to replicate FreeRTOS-based workflows in Zephyr, ensuring no
loss of functionality.

### 2\. Testing Under Real-World Conditions

Our devices were being used by tens of thousands of users in diverse
environments. From intense workouts to restful nights, every scenario had to be
accounted for. Early prototypes showed inconsistencies in BLE connections,
causing data loss—a nightmare for health trackers. Resolving these issues
required countless hours of debugging and field testing.

### 3\. BLE Bonding Crisis

Our most daunting challenge emerged after the initial migration release. After
updating to the Zephyr-based application, devices failed to connect to paired
phones. Users had to unpair the device, restart their phones, and reattempt
pairing. While this worked on some devices, certain Android phones stubbornly
refused to reconnect—even after following these steps.

This was a critical issue, as it not only disrupted user experience but also
risked eroding trust in our brand. We needed a solution that would restore
seamless connectivity across all devices.

## Behind the Solution

We followed the path to change the older bootloader so that it could accommodate
the new image from Zephyr NCS.

The process works like this:

1. Perform a Device Firmware Update (DFU) of an nRF5 SDK bootloader to remove
   protection on the MBR and bootloader regions.

1. Create an NCS image that matches the memory layout of the older image,
   ensuring proper alignment to properly transfer the image with the older ones.

1. Sign the NCS image using the same private and public key as the nRF SDK
   image.

### 1\. Bootloader Update for Migration Support

The following code changes, noted as commented out lines, needed to be made to the relevant bootloader files:

`nrf_dfu_validation.c`

```c
/*
 * @param[in]  sd_start_addr  Start address of received SoftDevice.
 * @param[in]  sd_size        Size of received SoftDevice in bytes.
 */
static bool softdevice_info_ok(uint32_t sd_start_addr, uint32_t sd_size)
{
    bool result = true;

    if (SD_MAGIC_NUMBER_GET(sd_start_addr) != SD_MAGIC_NUMBER)
    {
        NRF_LOG_ERROR("The SoftDevice does not contain the magic number identifying it as a SoftDevice.");

        // Protection removed for bootloader migration
        // result = false;
    }
    else if (SD_SIZE_GET(sd_start_addr) < ALIGN_TO_PAGE(sd_size + MBR_SIZE))
    {
        // The size in the info struct should be rounded up to a page boundary
        // and be larger than the actual size + the size of the MBR.
        NRF_LOG_ERROR("The SoftDevice size in the info struct is too small compared with the size reported in the init command.");
        result = false;
    }
    else if (SD_PRESENT && (SD_ID_GET(MBR_SIZE) != SD_ID_GET(sd_start_addr)))
    {
        NRF_LOG_ERROR("The new SoftDevice is of a different family than the present SoftDevice. Compatibility cannot be guaranteed.");
        result = false;
    }

    return result;
}
```

And of course, we needed to let go of SD magic number:

`nrf_bootloader_fw_activation.c`

```c
static uint32_t sd_activate(void)
{
    uint32_t   ret_val      = NRF_SUCCESS;
    uint32_t   target_addr  = nrf_dfu_softdevice_start_address() + s_dfu_settings.write_offset;
    uint32_t   src_addr     = s_dfu_settings.progress.update_start_address;
    uint32_t   sd_size      = s_dfu_settings.sd_size;
    uint32_t   length_left  = ALIGN_TO_PAGE(sd_size - s_dfu_settings.write_offset);

    NRF_LOG_DEBUG("Enter nrf_bootloader_dfu_sd_continue");

    if (SD_MAGIC_NUMBER_GET(src_addr) != SD_MAGIC_NUMBER)
    {
        NRF_LOG_ERROR("Source address does not contain a valid SoftDevice.")
        
        // Protection removed for bootloader migration
        // return NRF_ERROR_INTERNAL;
    }

    // This can be a continuation due to a power failure
    src_addr += s_dfu_settings.write_offset;

    if (s_dfu_settings.write_offset == sd_size)
    {
        NRF_LOG_DEBUG("SD already copied");
        return NRF_SUCCESS;
    }

    if (s_dfu_settings.write_offset == 0)
    {
        NRF_LOG_DEBUG("Updating SD. Old SD ver: %d, New ver: %d",
            SD_VERSION_GET(MBR_SIZE) / 1000000, SD_VERSION_GET(src_addr) / 1000000);
    }

    ret_val = image_copy(target_addr, src_addr, length_left, NRF_BL_FW_COPY_PROGRESS_STORE_STEP);
    if (ret_val != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Failed to copy firmware.");
        return ret_val;
    }

    ret_val = nrf_dfu_settings_write_and_backup(NULL);

    return ret_val;
}
```

### 2\. Matching Memory Layout

The memory layout should match the below process:

<p align="center">
 <img width="80%" src="{% img_url nrf5-sdk-to-ncs/matching-mem-layout.png %}" alt="Memory layout diagram" />
</p>

<p align="center">
 <img width="50%" src="{% img_url nrf5-sdk-to-ncs/flash-primary.png %}" alt="flash primary layout" />
</p>

### 3\. Signing NCS Image

The signature of the Zephyr-based image should match that of the nRF5 SDK to
ensure the DFU update doesn't fail. Use the following configurations in MCUboot:

```bash
CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P256=y
CONFIG_BOOT_SIGNATURE_TYPE_RSA=n
CONFIG_BOOT_SIGNATURE_KEY_FILE="key/my_key.pem"
```

### 4\. Creating NCS Image and DFU

Create an NCS image using DFUbatch script, replacing the SoftDevice with the
application and the bootloader with MCUboot:

```bash
BOOTLOADER_HEX=../build/mcuboot/zephyr/zephyr.hex
APPLICATION_HEX=../build/zephyr/app_signed.hex

nrfutil pkg generate --hw-version 52 \
--bootloader-version 102 \
--sd-req 0x123
--softdevice ${APPLICATION_HEX} \
--bootloader ${BOOTLOADER_HEX} \
--key-file ../child_image/mcuboot/boards/key/my_key.pem migration_dfu_test.zip
```

Use the nRFConnect application to DFU the new image, and it will be successfully
applied, allowing the NCS image to advertise.

### 5\. Addressing Post-Release Issues

Post-release migration of a DFU from an **nRF5 SDK-based application** to a
**Zephyr-based application** presented significant challenges for iOS and
Android users, primarily in managing BLE connections. The migration required
end-users to unpair their devices and restart their phones to re-establish
connections.

For many Android users, this process was particularly problematic, as some
phones failed to connect even after unpairing and restarting. In such cases,
users were forced to undertake multiple troubleshooting steps, including
toggling Bluetooth, clearing device caches, and sometimes even resetting the
device to factory settings.

These issues were primarily caused by changes in BLE stack handling between the
SDKs, differences in bonding and pairing mechanisms, and inconsistencies in how
Android devices handle BLE caching. Addressing these challenges required
extensive testing, clear user guidance, and improvements to the Zephyr-based
application to ensure smoother reconnection experiences post-migration.

First, to overcome the challenges of BLE connection issues during the migration
from an **nRF5 SDK-based application** to a **Zephyr-based application**, we
implemented a solution that preserved the user’s pairing information across the
transition. This involved migrating a copy of the existing pairing data from the
nRF5 SDK to the Zephyr-based application, ensuring continuity in the bond
between the device and the connected phone.

Next, we leveraged the **Service Changed indication** mechanism to inform the
phone's operating system (iOS and Android) of updates to the BLE GATT services.
This proactive notification forced the phone to refresh its cached data,
preventing connection errors caused by stale BLE attribute caches. By preserving
pairing information and signaling changes in a compliant manner, we
significantly reduced user-facing disruptions, eliminating the need for manual
troubleshooting steps like unpairing, restarting phones, or resetting Bluetooth
settings. This solution improved the migration experience and minimized user
frustration.

To develop the solution, I created a transition firmware based on nRF SDK that
would copy the pairing information and store it in a memory space intended to
remain untouched after the firmware update.

```c
void extract_ltk(uint16_t handle)
{
  const volatile uint32_t *ptr = (const volatile uint32_t *)(0xf7100);
  pm_peer_data_bonding_t bonding_data;
  ret_code_t err_code;
  uint32_t resolved_len = 0;
  uint32_t resolved_len_t = 0;
  pm_peer_id_t peer_id;
  uint32_t peer_count = pm_peer_count();
  if(peer_count>5) peer_count = 0;
  uint8_t per_ltk[16];
  int ret = 0;
  uint32_t erase_address = 0xf7000;
  peer_id = pm_next_peer_id_get(handle);
  err_code = pm_peer_data_bonding_load(handle, &bonding_data);
  if(*ptr != 0xFFFFFFFF)
  {
    memcpy(&per_ltk[0],((uint8_t*)ptr),16);
    ret = memcmp(per_ltk,&bonding_data.peer_ble_id.id_info.irk[0],16);
    if(ret!=0) fstorage_erase_info(erase_address,1);
  }
  else ret = 1;                                       //For safekeeping
  uint32_t total = sizeof(ble_gap_id_key_t)+sizeof(ble_gap_enc_key_t)+1;
  uint8_t aligned_irk[sizeof(ble_gap_id_key_t)] = {0};
  uint8_t aligned_ltk[sizeof(ble_gap_enc_key_t)] = {0};
  memcpy(aligned_keys, &bonding_data.peer_ble_id, sizeof(ble_gap_id_key_t));
  memcpy(aligned_keys+sizeof(ble_gap_id_key_t), &bonding_data.own_ltk, sizeof(ble_gap_enc_key_t));
  resolved_len = sizeof(bonding_data.peer_ble_id)+1;//(sizeof(bonding_data.peer_ble_id)%4);
  if(!err_code && (ret!=0))  {
    nrf_delay_ms(5000);
    fstorage_write_info(pairing_info_addr,&aligned_keys[0],total);
  }
}
```

Then, once the migration update completes, this information is handled in the
Zephyr-based application, as shown below:

```c
void retrieve_pairing_keys(void)
{
  const volatile uint32_t *ptr =
    (const volatile uint32_t
       *)(0xf7100); // Workaround to read back the pairing info from the old SDK
  uint8_t reversed_ltk_ediv[2]; // Array to store reversed bytes
  uint8_t reversed_ltk_val[16];
  uint8_t reversed_ltk_rand[8];
  memcpy(&irk_val[0], ((uint8_t *)ptr), 16);
  memcpy(&keys_addr[0], ((uint8_t *)ptr + 16 + 1), 6);
  memcpy(&ltk_val[0], ((uint8_t *)ptr + 16 + 6 + 1), 16);
  memcpy(&ltk_ediv[0], ((uint8_t *)ptr + 16 + 6 + 16 + 1 + 2), 2);
  memcpy(&ltk_rand[0], ((uint8_t *)ptr + 16 + 6 + 16 + 2 + 1 + 2), 8);

  printk("Successfully copied pairing info from the memory\\n");
}
```

```c
static void pre_shared_bond_set(void)
{
  int err;
  int bond_num = 0;
  struct bt_keys *pairing_info;

  bt_foreach_bond(BT_ID_DEFAULT, bond_cb, &bond_num);

  if (bond_num) {
    return;
  }

  bt_addr_le_t addr = {.type = BT_ADDR_LE_PUBLIC,
           .a.val = {0x43, 0x82, 0x5E, 0xC7, 0xE8, 0xF4}};

  struct bt_ltk ltk = {.val = {0x31, 0x30, 0xca, 0x16, 0x8d, 0x53, 0x73, 0x7c, 0x89, 0xee,
             0x42, 0x58, 0x61, 0x47, 0x03,
             0xb4}, //{0xcb, 0xea, 0xd5, 0xea, 0xe1, 0xab, 0x7c, 0x36, 0x91,
              //0x1b, 0xe7, 0xd3, 0x31, 0xd1, 0xf8, 0xbc},
           .ediv = {0},
           .rand = {0}};

  struct bt_irk irk = {.val = {0xF7, 0xDD, 0x7E, 0xC0, 0xDD, 0x48, 0x69, 0x41, 0x15, 0xF0,
             0x2A, 0x92, 0xA7, 0x1E, 0x27, 0x8A},
           .rpa.val = {0}};

  /**Copying the retrieved values from the memory onto the relevant array to pass the pairing
   * infos*/
  memcpy(&addr.a.val[0], keys_addr, 6);
  memcpy(&irk.val[0], irk_val, 16);
  memcpy(&ltk.val[0], ltk_val, 16);
  memcpy(&ltk.ediv[0], ltk_ediv, 2);
  memcpy(&ltk.rand[0], ltk_rand, 8);

  pairing_info = bt_keys_get_addr(0, &addr);
  if (pairing_info == NULL) {
    printk("Failed to get keyslot\\n");
  }

  memcpy(&pairing_info->periph_ltk, &ltk, sizeof(pairing_info->ltk));
  memcpy(&pairing_info->irk, &irk, sizeof(pairing_info->irk));

  pairing_info->flags = 0;
  pairing_info->enc_size = 16;
  pairing_info->keys = BT_KEYS_IRK | BT_KEYS_PERIPH_LTK;

  unsigned char ff[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (memcmp(keys_addr, ff, 6) != 0) {
    err = bt_keys_store(pairing_info);
    if (err) {
      printk("Failed to store keys (err %d)\\n", err);
    } else {
      printk("Keys stored successfully\\n");
    }
  }
}
```

This completes the transfer of bonding information from the nRF5 SDK-based
application to the Zephyr-based application. However, we still needed to
indicate service changes to ensure seamless transitioning and connectivity. This
was done using the following:

```c
void service_changed_work_handle(struct k_work *item)
{
  int err;
  static struct bt_gatt_indicate_params indicate_params;
  /* Harcode the attribute handle for SC characteristic from nRF5 SDK application (12 in this
   * case). CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION must be disabled in project configuration to
   * allow the indication to be sent, as the Zephyr host has know way of knowing if the client
   * was subscribed to this characteristic.
   *
   * IMPORTANT: consider if disabling CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION
   * has security implications for your application.
   */
  static struct bt_gatt_attr attr = {.handle = 12};
  static uint16_t sc_range[2];

  sc_range[0] = sys_cpu_to_le16(0x0001);
  sc_range[1] = 0xffff;

  indicate_params.attr = &attr;
  indicate_params.func = sc_indicate_rsp;
  indicate_params.data = &sc_range[0];
  indicate_params.len = sizeof(sc_range);

  err = bt_gatt_indicate(local_connection, &indicate_params);
  if (err) {
    printk("Failed to send SC indication. (err %d)\\n", err);
    notify_service_changed(local_connection);
  }
}
```

## Breaking Through: Designing the Impossible Solution

We approached the migration with a clear strategy:

### 1\. BLE Bonding Fix

The BLE bonding issue became our highest priority. After extensive analysis, we
discovered that the bonding information format in FreeRTOS and Zephyr differed
significantly. Bonding data stored on the FreeRTOS firmware was incompatible
with Zephyr’s stack, causing pairing failures.

Our breakthrough solution involved **copying the bonding information** from the
FreeRTOS firmware during the migration. We restructured this data to align with
Zephyr's bonding format and injected it into the Zephyr-based application during
the OTA update. This meant that after migration, the device retained all bonding
information, enabling it to connect seamlessly without requiring users to unpair
or re-pair their devices.

### 2\. Modular Architecture

Zephyr’s component-driven design allowed us to break the application into
smaller, testable modules. This helped in isolating issues during testing and
enabled easier debugging.

### 3\. BLE Enhancements

Leveraging Zephyr's BLE stack, we fine-tuned the connection parameters, reducing
packet loss and improving reconnection times. These changes dramatically
enhanced the BLE experience, especially for iOS users.

### 4\. Rigorous Testing Framework

We developed a hybrid testing setup combining hardware-in-the-loop (HIL)
simulations and real-world scenarios. Our emulation tools, like
[Renode](https://renode.io/), were instrumental in replicating edge cases that
might otherwise go unnoticed.

### 5\. Future Possibilities

The migration wasn’t just about solving today’s problems; it was about paving
the way for future innovations. Zephyr’s modularity and scalability opened new
doors for product development.

## Zephyr and the Future of Health & Fitness

With Zephyr as our foundation, we’re building the next generation of Ultrahuman
devices that will revolutionize how people engage with their health. Here’s
what’s on the horizon:

### 1\. Continuous Health Monitoring

Zephyr’s real-time capabilities and seamless integration with third-party
libraries allow us to develop devices that monitor metabolic health, stress
levels, and sleep patterns with unprecedented accuracy.

The FreeRTOS version of Nordic’s SDK required more workarounds to ensure
compatibility with for third- party libraries compatibility compared to the
native support in Zephyr. RIt requireds manual integration of third-party
libraries, often necessitating copying files into the project and manually
adjusting build scripts (e.g., Makefiles).  
Additionally, Zephyr's **DeviceTree** and **Hardware Abstraction Layer (HAL)**
simplify configuring peripherals and sensors. Developers no longer need to
manually initialize or handle hardware registers — it’s all declaratively
defined in the DeviceTree.  
Zephyr also includes robust stacks for BLE, Wi-Fi, LoRa, Thread, CAN, and more.

Middleware such as **MCUboot**, **File Systems (e.g., FAT, LittleFS)**, and
**Networking Protocols** (MQTT, CoAP) are supported out of the box.

### 2\. Personalized Fitness Recommendations

Leveraging Zephyr's powerful processing capabilities, we’re integrating
AI-driven algorithms that analyze data trends to provide hyper-personalized
fitness plans. From dynamic workout adjustments to nutritional insights based on
real-time metabolic activity, the possibilities are endless.

### 3\. Accessibility and Scalability

Zephyr’s robust architecture allows us to develop cost-effective devices without
compromising on performance. The Zephyr- based stack offersprovides
**standardized, portable APIs**, while FreeRTOS relies onuses
**SoftDevice-specific APIs**., which These APIs are easier to use but less
flexible, limiting **more future-proof capabilities** such as(e.g., LE Audio
support and , open-source extensibility). Zephyr offers **more granular control
over power management.**

This means we can bring health-tracking technology to underserved populations,
promoting health equity and making advanced fitness tools accessible to
everyone.

### 4\. Environmental Impact

Zephyr’s energy-efficient architecture is enabling us to develop devices with
longer battery life and reduced energy consumption. In particular, Zephyr offers
**more granular control over power management.** This not only benefits users
but also reduces the environmental footprint of our products.

### 5\. Changing Lives

The potential societal impact is immense. Ultrahuman devices powered by Zephyr
will help users take control of their health in ways never before possible. By
enabling early detection of health issues, improving fitness outcomes, and
fostering healthier habits, we’re not just building products—we’re building
healthier communities.

## The Results: A Game-Changing Upgrade

Despite the hardships, the migration was a resounding success. Here’s what we
achieved:

- **Improved BLE Stability**: Disconnection rates dropped by 80%, and pairing
  became effortless across devices.
- **Enhanced Health Metrics**: Users began receiving more granular insights into
  their health, thanks to the newly integrated algorithms.
- **Seamless Transition**: The OTA update process was so smooth that most users
  didn’t even realize the complexity behind the transition.
- **Scalable Future**: With Zephyr as the foundation, our devices are now
  equipped to handle future updates and innovations with ease.

## What’s Next?

With Zephyr, we’re not just creating better devices—we’re shaping the future of
health and fitness. From cutting-edge health monitoring to eco-friendly devices,
Ultrahuman is set to transform lives worldwide.

**To all our users who’ve joined us on this journey: thank you. The impossible
was only possible because of you. Together, we’re building a healthier world.**

{% include newsletter.html %}

{% include submit-pr.html %}

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- Nordic’s doc comparing nRF5 SDK \+ nRF-Connect SDK:
  [https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/nrf-connect-sdk-and-nrf5-sdk-statement](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/nrf-connect-sdk-and-nrf5-sdk-statement)
- Brief article describing how to update-in-place the nRF5 bootloader to an NCS
bootloader:
[https://www.embeddedrelated.com/showarticle/1573.php](https://www.embeddedrelated.com/showarticle/1573.php)

<!-- prettier-ignore-end -->
