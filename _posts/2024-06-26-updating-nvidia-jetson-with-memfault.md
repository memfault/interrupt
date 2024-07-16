---
title: Updating NVIDIA Jetson devices with Memfault OTA
description:
  How to use Memfault to update NVIDIA Jetson devices over-the-air (OTA).
author: tsarlandie
tags: [linux, ota, nvidia, jetson, memfault]
---

<!-- excerpt start -->

NVIDIA offers one of the most comprehensive SDKs for developers of AI-heavy products. It includes a development kit that can emulate other devices in the lineup (Jetson AGX Orin DK), a simpler development kit for “entry-level” products (Jetson Orin Nano DK), a ton of exciting software libraries, AI models and even more examples of how to use them. It’s truly outstanding and out of the box shows up as a Ubuntu workstation which will feel very familiar.

However, it can be a bit daunting to figure out how to take this workstation experience and turn it into a headless unit that you can ship to customers far away and update remotely.

<!-- excerpt end -->

In this article, I will walk you through building a customized image of NVIDIA
Jetson Linux and updating it via Memfault OTA service.

## Pre-requisites

To use NVIDIA SDK, you need a Ubuntu machine running Ubuntu 20.04 or 22.04. You will need one NVIDIA developer kit, we use the Jetson Orin Nano Developer Kit because it is widely available. You will also need to install an NVMe drive on the bottom of the kit because the OTA system requires it for image-based OTA (an SDCard will not work).

Testing and validation for this article is based on [NVIDIA Jetson Linux
36.3](https://developer.nvidia.com/embedded/jetson-linux) (latest
at the time of publication) and specifically the following files:

- [Jetson Linux BSP 36.3](https://developer.nvidia.com/downloads/embedded/l4t/r36_release_v3.0/release/jetson_linux_r36.3.0_aarch64.tbz2)
- [OTA Tools](https://developer.nvidia.com/downloads/embedded/l4t/r36_release_v3.0/release/ota_tools_R36.3.0_aarch64.tbz2)

> If your developer kit is brand new, you may need to update your firmware to support Jetson 36. [We found these instructions easy to follow.](https://www.jetson-ai-lab.com/initial_setup_jon.html#__tabbed_1_1)

## Preparing the image

NVIDIA provides a pre-built root filesystem which includes a graphical filesystem and lots of examples and tools. It’s great for initial experimentation but before going into production you will want to reduce the size of the image and only include software that is used.

Generating your root filesystem is very easy.

### Generate a minimal root filesystem with the packages you need

First, download and decompress the BSP:

```bash
$ mkdir ~/nvidia && cd ~/nvidia
$ wget https://developer.nvidia.com/downloads/embedded/l4t/r36_release_v3.0/release/jetson_linux_r36.3.0_aarch64.tbz2
$ tar xpf jetson_linux_r36.3.0_aarch64.tbz2
```

Then, customize the list of packages that are included in the image:

```bash
$ cd ~/nvidia/Linux_for_Tegra/tools/samplefs
$ cp nvubuntu-jammy-minimal-aarch64-packages nvubuntu-jammy-interrupt-aarch64-packages
$ vi nvubuntu-jammy-interrupt-aarch64-packages
# ... customize the list of packages as needed

# For OTA, we will use curl and jq
$ echo "curl" >> nvubuntu-jammy-interrupt-aarch64-packages
$ echo "jq" >> nvubuntu-jammy-interrupt-aarch64-packages
```

Finally, use the `nv_build_samplefs.sh` script to generate a tarball of the root filesystem with all the packages installed:

```bash
$ sudo ./nv_build_samplefs.sh --abi aarch64 --distro ubuntu --flavor interrupt --version jammy
```

> This script only works on Ubuntu 22.04. If you are using a more recent
> version, [NVIDIA provides a Docker image](https://docs.nvidia.com/jetson/archives/r36.3/DeveloperGuide/SD/RootFileSystem.html#execute-the-script-on-a-non-ubuntu-22-04-host) that you can use to
> generate the root filesystem.

At this point, you have a `sample_fs.tbz2` file. You need to extract it in the `rootfs` folder of the BSP and run the NVIDIA `apply_binaries.sh` script.

```
# Decompress in the `rootfs` folder
$ cd ~/nvidia/Linux_for_Tegra/rootfs
$ sudo tar xpf ../tools/samplefs/sample_fs.tbz2
$ cd ..

# Install NVIDIA binary packages into the rootfs
$ ./apply_binaries.sh
```

### Skip `oem-config` and add a default user

For a production image, you will need to add a user account. This will also skip
the `oem-config` program that is otherwise run automatically by Jetson Linux.

```bash
$ sudo tools/l4t_create_default_user.sh -u mf -n orin --accept-license
Creating: Username - mf, Password - OTk5MGE0, Autologin - false
Creating: Hostname - orin
```

> A password is automatically generated for you by default. You can also set one
with the `-p` option. Either way, it would not be a good idea to ship an image
with a password that someone could possibly find and compromise all your
devices.
>
> For a better alternative, look at our list of [EOSS talks recap](/blog/eoss-recap#engineering-secure-ssh-access-for-engineers) which includes "Engineering Secure SSH Access for Engineers".

### Install Memfault monitoring

Now would also be a great time to add Memfault monitoring to your image (make sure to replace `<YOUR_PROJECT_KEY>` with a valid [Memfault Project Key](https://docs.memfault.com/docs/platform/data-routes)).

```bash
curl -sSf https://app.memfault.com/static/scripts/linux-quickstart.sh | sudo  MEMFAULT_PROJECT_KEY="<YOUR PROJECT KEY>" chroot rootfs/ sh
```

> This script will show some errors because it is not designed to run in a
chroot environment. However, it will install the necessary packages and
configuration files.

We want the version number reported to Memfault to include the NVIDIA `/etc/user_release_version` file. So we adjust `/usr/bin/memfault-device-info`:

```bash
#!/bin/bash

source /etc/os-release
USER_VERSION=$(awk '{print $NF}' /etc/user_release_version)
MACHINE_ID=$(cat /etc/machine-id)

echo "MEMFAULT_DEVICE_ID=$MACHINE_ID"
echo "MEMFAULT_HARDWARE_VERSION=orin"
echo "MEMFAULT_SOFTWARE_TYPE=ubuntu"
echo "MEMFAULT_SOFTWARE_VERSION=${VERSION_ID}.${USER_VERSION}"
```

### Add Memfault OTA Agent

We use the agent described in the [Generic OTA Guide](https://docs.memfault.com/docs/linux/generic-ota). Let's add it to our image:

Add `/usr/bin/memfault-ota` to the image with the following content (and make sure it is executable):

```bash
#!/bin/sh

PROJECT_KEY=$(cat /etc/memfaultd.conf | jq -r '.project_key')
MEMFAULT_SOFTWARE_TYPE=$(cat /etc/memfaultd.conf | jq -r '.software_type')
MEMFAULT_SOFTWARE_VERSION=$(cat /etc/memfaultd.conf | jq -r '.software_version')
eval "$(memfault-device-info)"

url=$(curl -f -s --get \
    --data-urlencode "hardware_version=${MEMFAULT_HARDWARE_VERSION}" \
    --data-urlencode "software_type=${MEMFAULT_SOFTWARE_TYPE}" \
    --data-urlencode "software_version=${MEMFAULT_SOFTWARE_VERSION}" \
    --data-urlencode "device_serial=${MEMFAULT_DEVICE_ID}" \
    --header "Memfault-Project-Key: ${PROJECT_KEY}" \
    "https://api.memfault.com/api/v0/releases/latest/url")
exit_code=$?

if [ "$exit_code" -eq 0 ] && [ -n "$url" ]; then
    # The following line will need to be updated for the specific update installer
    # you are using.
    update-installer $url
    if [ $? -eq 0 ]; then
      memfaultctl reboot --reason 3
    else
        logger -p user.err "OTA install failed"
    fi
else
    if [ "$exit_code" -ne 0 ]; then
        logger -p user.err "OTA status check request failed"
        logger -p user.err "curl returned exit code: ${exit_code}"
    fi
fi
```

`memfault-ota` will call the command `update-installer` with the URL of the OTA package to install. We have to provide this command.

In the `rootfs`, create a file `usr/bin/update-installer` with the following content (and make sure it is executable):
```bash
#!/bin/sh

set -ex

rm -fr /ota
mkdir -p /ota/package

echo "Downloading..."
wget -O /ota/package.tar $1

echo "Preparing..."
tar -C /ota/package -xf /ota/package.tar
tar -C /ota/package -pxf /ota/package/ota_tools.tar.tbz2

echo "Installing..."
cd /ota/package/Linux_for_Tegra/tools/ota_tools/version_upgrade/
./nv_ota_start.sh /ota/package/ota_payload_package.tar.gz
```

Finally, to automatically look for OTA updates at regular intervals, we use a SystemD unit and timer. Create the following files in the rootfs:

`/lib/systemd/system/memfault-ota.service`
```
[Unit]
Description=Memfault OTA Service
Requires=network.target
After=network.target

[Service]
Type=oneshot
ExecStart=/bin/sh /usr/bin/memfault-ota

[Install]
WantedBy=multi-user.target
```

`/lib/systemd/system/memfault-ota.timer`
```
[Unit]
Description=Launch %i service every 6 hours

[Timer]
OnBootSec=3min
OnUnitInactiveSec=6h
Persistent=true

[Install]
WantedBy=timers.target
```

### Configure your image

Now is a great time to apply further customization to the image. You can add
your programs, configuration files, etc. Hopefully, the examples above will provide some good inspiration.

## Flash your device (first-time)

At this point, our image is ready to be packaged and flashed via a direct USB
link to a device.

This is the initial preparation of the device, the one you would do at
the factory:

```bash
# Generate images for QSPI
$ sudo ROOTFS_AB=1 ./tools/kernel_flash/l4t_initrd_flash.sh --showlogs -p "-c bootloader/generic/cfg/flash_t234_qspi.xml" --no-flash --network usb0 jetson-orin-nano-devkit internal

# Generate images for external storage device
$ sudo ROOTFS_AB=1 ./tools/kernel_flash/l4t_initrd_flash.sh --showlogs --no-flash --external-device nvme0n1p1 -c ./tools/kernel_flash/flash_l4t_t234_nvme_rootfs_ab.xml --external-only --append --network usb0 jetson-orin-nano-devkit external

# Flash images into the both storage devices
$ sudo ./tools/kernel_flash/l4t_initrd_flash.sh --showlogs --network usb0 --flash-only
```
([Reference documentation](https://docs.nvidia.com/jetson/archives/r36.3/DeveloperGuide/SD/FlashingSupport.html#using-initrd-flash-with-orin-nx-and-nano))

Your system will reboot after the last command, and is ready to be used!

## Prepare an update

Now without changing anything else (but feel free to edit the configuration or add other programs on the system), let’s increment the version in the `nv_tegra/user_version` file and build an OTA update payload:

```bash
$ cd ~/nvidia/Linux_for_Tegra
$ sudo -E ROOTFS_AB=1 ./tools/ota_tools/version_upgrade/l4t_generate_ota_package.sh --external-device nvme0n1 jetson-orin-nano-devkit R36-3
```

This will generate the `bootloader/jetson-orin-nano-devkit/ota_payload_package.tar.gz` file.

Our `update-instaler` also needs the OTA tools so we will include them in each release.

```bash
$ cd ~/nvidia/Linux_for_Tegra/bootloader/jetson-orin-nano-devkit
$ cp ~/nvidia/ota_tools_R36.3.0_aarch64.tbz2 ./ota_tools.tar.bz2
$ source ../../nv_tegra/user_version
$ tar -cf update-22.04.$USER_VERSION.tar ota_tools.tar.tbz2 ota_payload_package.tar.gz
```

This `update-0.2.tar` file is our complete update payload. It's what will be distributed by Memfault OTA service. We are not compressing it because it's content is already compressed (it's one tar containing two compressed files).

## Deploy the update

To upload the update you could use the Memfault web interface but it's generally easier to do it from the command line:

```bash
$ pip install memfault-cli
$ memfault --org $MEMFAULT_ORG --org-token $MEMFAULT_OAT --project $MEMFAULT_PROJECT upload-ota-payload --hardware-version orin --software-type ubuntu --software-version 22.04.$USER_VERSION update-22.04.$USER_VERSION.tar
```

To activate this update, navigate to Memfault OTA and activate the release.

## Watch the update!

You can wait for the device to check for updates every 6 hours or you can manually trigger the update by running the `memfault-ota` command:

```bash
$ sudo memfault-ota
```

## Conclusion

We have built a custom image for an NVIDIA Orin Nano Developer
Kit with Memfault OTA and Memfault Monitoring. We are now able to update this
device (and all its replicas!) over the air **reliably**.

This is a great starting point for building a production-ready device.

We left out signing and verifying the image. From an OTA distribution
perspective, it makes no difference if the image is signed and encrypted. For a
real deployment, you should do a threat analysis and implement appropriate
security measures. NVIDIA includes a suite of tools to sign and verify
updates.

If you have any questions or need help, feel free to reach out to us!

## References

- [NVIDIA guide on image based update](https://docs.nvidia.com/jetson/archives/r36.3/DeveloperGuide/SD/SoftwarePackagesAndTheUpdateMechanism.html#updating-jetson-linux-with-image-based-over-the-air-update)

