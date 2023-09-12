---
title: "OTA for Embedded Linux Devices: A practical introduction"
author: tsarlandie
tags: [linux, ota, swupdate, memfault]
---

<!-- excerpt start -->

A core belief of Memfault is that we can ship faster when we have good
infrastructure in place. An essential piece of this infrastructure is tools to
send firmware updates over the air. It enables the team to ship more often and
spend more time building features.

In this article, we look specifically at what is required to ship over-the-air
firmware updates for Linux systems.

<!-- excerpt end -->

A good OTA setup should allow you to quickly prepare updates and ship them with
confidence. In particular:

- It should be predictable: systems should be identical (ideally, exact
  byte-level copies) regardless of whether they are a fresh install or have been
  updated 10 times in their lifespan.
- It should be reliable: interruptions of network or power should not risk
  damaging the device.
- It should be resilient so it can still update devices even when they are in a
  non-nominal state.

> 📺 **Watch the webinar**
>
> This topic was also the subject of a
> [Memfault webinar: OTA for embedded Linux Devices](https://hubs.la/Q01M97730).

{% include toc.html %}

As you will see, many components collaborate to update your embedded Linux
systems. Our goal in this blog post is to walk you through a complete update
cycle, introducing the different components and showing how they interact with
each other.

For today’s discussion, we will focus on a system built with Yocto, booting on
an A/B partition scheme, with [U-Boot](https://u-boot.readthedocs.io/en/latest/)
as the bootloader and [SWUpdate](https://sbabic.github.io/swupdate/index.html)
as the updater. This is an extremely common setup, completely open-source (free
as beer and speech), and the one we use in Memfault Linux SDK examples. The
concepts shown here will apply to a lot of other OTA strategies as well and it’s
important to point out that Memfault for Linux is not limited to this setup.

> **Understanding Yocto and the A/B partitioning scheme**
>
> Yocto is a Linux system image builder. It uses configuration files to identify
> which components are required in the system, builds all the packages, and
> prepares the file system (both a `.tar.gz` and a `.ext3` raw-image).
>
> An A/B partition scheme allocates two system partitions on the device’s
> storage. One is the version that is running. The other one starts empty and
> will be filled with a new system image when we install an update.
>
> This configuration guarantees that we always have a working copy of the system
> because we do not touch the active partition while we prepare the new one. It
> will still be there for a possible rollback after rebooting in the other
> partition.
>
> Doing this protects us from damages that could result from partially installed
> updates: loss of power or connectivity could result in a system where some
> packages have been updated but not all and the system is not usable.
>
> With Yocto and the A/B partition scheme, we can build very predictable
> systems. Firmware engineers love determinism.

## The Big Picture

Let’s start with a general overview of the update process.

- A firmware engineer, using a build system and some form of continuous
  integration system **builds the update**. The result is an artifact that can
  be used to update a system to the latest version.
- The update is uploaded to an **OTA backend** which will announce its
  availability and serve it to eligible devices. This can be as simple as a
  `firmware-latest` file hosted on S3 but having the ability to serve different
  updates to different hardware versions, and serve different firmware versions
  to different groups of customers is a must-have in our opinion.
- Devices will regularly contact the update server to **download the update**.
  We typically use a daemon or cron job to do this.
- The same agent will also **install the update** on the inactive system
  partition.
- Once the update has been installed and verified, the agent can **reboot into
  the freshly updated system partition**. This requires some coordination
  between the old system partition, the new one, and the **bootloader**.
- Finally, the system running from the updated partition will reach out to the
  OTA backend to **confirm** that the update has been installed.

Now that we understand the different steps, let’s see how this is implemented in
practice.

## Diving into the details

> 🪛🔬 **Tag Along!**
>
> We encourage you to follow along on an emulator or a real device. The
> [Memfault Linux SDK example](https://github.com/memfault/memfault-linux-sdk)
> is one way to quickly get a system with all these components configured and
> you can use Memfault backend to host and distribute your update file. It’s
> free under the evaluation license.
>
> Nothing we show here is specific to Memfault though and you can replicate the
> system using a different backend. Follow our QEMU Quickstart guide to get
> started in an emulator, or the Raspberry Pi Quickstart guide to do the same
> thing with real hardware.

### Building the update

The first step to distributing an update over the air is to build and package
it. Your build system should be fully automated (CI) and configured to always
provide, for each revision of your source code, both a complete image to flash
on a new device and an update package.

The update package is the input we provide to the updater. It contains enough
information to update a device running an older version of the firmware to the
new version. With our A/B partitioning scheme, this is easily achieved by
shipping a new system image as the update.

> 📦 Delta updates are also possible with swupdate but we will not discuss them
> in this article. Refer to
> [Delta Update with SWUpdate](https://sbabic.github.io/swupdate/delta-update.html)
> for more information.

A SWUpdate update package is just a [`cpio`][cpio] archive in which the first
file is an update descriptor. The `meta-swupdate` layer for Yocto provides the
`swupdate-image` class to do this easily and the
[Memfault example includes a `swupdate-image` target](https://github.com/memfault/memfault-linux-sdk/blob/1.4.0-kirkstone/meta-memfault-example/recipes-core/images/swupdate-image.bb).

All that is required to build a new update package is to run
`bitbake swupdate-image`.

```bash
$ bitbake swupdate-image
...
```

You can inspect the resulting `.swu` archive. As discussed, it contains the
update descriptor (we will look at it in detail in a minute) and the new
filesystem image.

```bash
$ cpio -vt < tmp/deploy/images/qemuarm64/swupdate-image-qemuarm64.swu
-rw-r--r--   1 build    users        1111 Apr 25 00:53 sw-description
-rw-r--r--   1 build    users    48618608 Apr 25 00:53 base-image-qemuarm64.ext4.gz
```

> ✒ ️ **Conventions for this blog post**
>
> Whenever we are showing outputs and commands typed on the developer machine
> used to prepare the image, we will show a `$` shell prompt. When we are
> showing commands and output typed directly on the target device, we will use
> the `#` prompt.

[cpio]: https://en.wikipedia.org/wiki/Cpio

### Distributing updates

To distribute this new update over the air we will need to make it available to
our fleet of devices. This typically requires two steps:

1. Upload the update file to a CDN so it can be downloaded quickly by a large
   number of devices at a reasonable cost.
2. Indicate that the update is available and provide the link to it on an
   “update server” endpoint that our devices will contact.

> 📢 **Distributing updates is one of the core features of Memfault.**
>
> You can follow along with a free trial account:
> [create a new OTA version and activate it](https://docs.memfault.com/docs/linux/quickstart-raspberrypi#use-memfault-ota-to-update-your-raspberry-pi).
>
> Memfault OTA offers a lot more than just serve the latest update files to your
> customers:
>
> - Control which devices receive which updates so you can have multiple groups
>   of devices (we call them cohorts) “on” different versions at the same time:
>   internal users, beta users, production users, etc.
> - Progressively roll out updates to devices so you have time to detect issues
>   with the new build before too many devices have been updated
> - Serve different versions of the update for different hardware revisions

### Fetching an update

In the SWUpdate world, we use a
[daemon mode called Suricatta](https://sbabic.github.io/swupdate/suricatta.html)
to communicate with the update server and detect when a new version is
available. This daemon mode needs to be
[enabled at build time](https://github.com/memfault/memfault-linux-sdk/blob/1.4.0-kirkstone/meta-memfault-example/recipes-support/swupdate/files/defconfig#L74)
and it adds a few options to the `swupdate` command.

Looking at the `swupdate` configuration, you will find that we provide the
daemon with information about the device’s identity (current image version,
hardware version) and how to fetch updates (server URL, authentication token):

```bash
# cat /tmp/swupdate.cfg
...
suricatta :
{
  ...
  url = "https://device.memfault.com/api/v0/hawkbit";
  gatewaytoken = "1gfiixxx";
};
identify = (
  {
    name = "memfault__current_version";
    value = "0.0.2";
  },
  {
    name = "memfault__hardware_version";
    value = "qemuarm64";
  },
  {
    name = "memfault__software_type";
    value = "main";
  } );
```

This daemon runs in a SystemD unit and one way to monitor its activity is to use
the system journals:

```bash
root@qemuarm64:~# journalctl -u swupdate -f
... SWUPDATE running :  [start_suricatta] : Suricatta awakened.
... SWUPDATE running :  [channel_log_effective_url] : Channel's effective URL resolved to https://device.memfault.com/api/v0/hawkbit/default/controller/v1/mf14
... SWUPDATE running :  [server_get_deployment_info] : No pending action on server.
```

As shown here, it regularly calls the server to see if an update is available.

### Installing updates

When the update becomes available, `swupdate` will download it and write it
directly to the inactive system partition. Writing the update directly in place
directly is an important characteristic of many update systems. It offers the
benefit of not requiring large temporary storage to store the update before
installation.

An interesting question here is how does `swupdate` know where to install the
update? As we have said before there are two system partitions on our device and
we could be running from any of them.

Looking inside the update descriptor (the first file in the update package), we
find that it defines two updates strategy named `copy1` and `copy2`:

```bash
$ cat swupdate-description
software = {
    version = "0.0.2";

    qemuarm64 = {
        hardware-compatibility: [ "1.0" ];
        stable: {
            copy1: {
                images: (
                    {
                        filename = "base-image-qemuarm64.ext4.gz";
                        type = "raw";
                        compressed = "zlib";
                        device = "/dev/vda2";
                    }
                );
                ...
            }

            copy2: {
                images: (
                    {
                        filename = "base-image-qemuarm64.ext4.gz";
                        type = "raw";
                        compressed = "zlib";
                        device = "/dev/vda3";
                    }
                );
                ...
            }
```

And if you run `ps |grep swupdate` you will see that `swupdate` was started with
the `-e stable,copy2`parameter. This parameter is set by
`/usr/lib/swupdate/conf.d/09-swupdate-args`
[which considers which device is mounted as`/`](https://github.com/memfault/memfault-linux-sdk/blob/1.4.0-kirkstone/meta-memfault-example/recipes-support/swupdate/files/09-swupdate-args.in#L3-L7)
to decide if we will need to update `copy1` (in `/dev/vda2`) or `copy2` (in
`/dev/vda3`).

The partition currently mounted is active. The other one is inactive (it’s not
even mounted) and will be overwritten at a raw byte level with the new image.

Once the update is written to disk, a crucial step is needed to activate it. The
remaining content of the file provides it:

```bash
...
  copy1: {
      ...
      images: ({ ... device = "/dev/vda2" ... })
      uboot: (
          {
              name = "rootpart";
              value = "2";
          }
      );
  }
  copy2: {
      ...
      images: ({ ... device = "/dev/vda3" ... })
      uboot: (
          {
              name = "rootpart";
              value = "3";
          }
      );
  }
```

Here we tell SWUpdate to write a configuration variable for the bootloader. This
configuration variable will indicate the partition number that will be used for
the next boot. And as you can see when we write the image to `/dev/vda3` we will
set the variable `rootpart` to the value `3`. More on U-Boot variables in just a
second!

Finally, we need to restart the device. This can be as simple as running the
`reboot` command.

In the case of this Memfault-equipped example device, we want to record why the
device rebooted so we execute `memfaultctl reboot --reason 3` (`3` is the
Firmware Update reason in
[Memfault nomenclature](https://docs.memfault.com/docs/platform/reference-reboot-reason-ids/)).

More complex update scenarios can easily be implemented. For example by setting
a flag on the system to indicate that an update is installed and ready. The user
will reboot when they are ready.

Again, this is defined in SWUpdate's configuration:

```bash
# cat /tmp/swupdate.cfg
globals :
{
  postupdatecmd = "memfaultctl reboot --reason 3";
};
```

### Bootloader

The bootloader is the crucial piece of software that runs first when your device
starts. It plays an essential role in our support for OTA updates because it
provides the switch to run the device on the system partition A or partition B.

U-Boot is a surprisingly capable piece of software and you may be surprised to
learn that the A/B switch is implemented in the form of a script run on every
boot. This script is typically specific to your hardware because it also sets
some essential boot parameters and passes them to the kernel.

Let’s look at
[the RaspberryPi version in our embedded system example](https://github.com/memfault/memfault-linux-sdk/blob/kirkstone/meta-memfault-example/recipes-bsp/rpi-uboot-scr/files/boot.cmd.in)
([the QEMU version is here](https://github.com/memfault/memfault-linux-sdk/blob/1.4.0-kirkstone/meta-memfault-example/recipes-bsp/u-boot/files/0003-memfault_boot-boot-commands.patch#L18-L38)):

```bash
saveenv
fdt addr ${fdt_addr} && fdt get value bootargs /chosen bootargs
if env exists rootpart;then echo Booting from mmcblk0p${rootpart};else setenv rootpart 2;echo rootpart not set, default to ${rootpart};fi
load mmc 0:${rootpart} ${kernel_addr_r} boot/@@KERNEL_IMAGETYPE@@
setenv bootargs "${bootargs} root=/dev/mmcblk0p${rootpart}"
@@KERNEL_BOOTCMD@@ ${kernel_addr_r} - ${fdt_addr}
```

The U-Boot command line allows you to run commands and interact with the
environment variables. It’s a great tool to debug OTA and the boot process in
general:

```bash
U-Boot 2022.01 (Jan 10 2022 - 18:46:34 +0000)

DRAM:  512 MiB
Flash: 64 MiB
In:    pl011@9000000
Out:   pl011@9000000
Err:   pl011@9000000
Net:   eth0: virtio-net#32
Loading Environment from FAT... OK
Hit any key to stop autoboot:  0

=> if env exists rootpart; then echo "rootpart: ${rootpart}"; else echo "NO";fi
rootpart: 3
=> env print -a
... prints all the environment variables
```

First, let’s talk about where the environment variables are stored. U-Boot can
be configured to persist the environment in various locations and this is
something that will most likely be defined by your board vendor in the BSP
package. For example, the `meta-raspberrypi` BSP specifies that U-Boot
[should store the environment in a file called `/boot/uboot.env`](https://github.com/agherzan/meta-raspberrypi/blob/master/recipes-bsp/u-boot/files/fw_env.config).
On some other systems, the environment may be stored at a fixed location in
Flash, or on an EEPROM.

Second, note that the boot script helps with three essential components:

- It loads the kernel from the new root partition. This means we can update the
  kernel, and it’s important to remark here that it works because the kernel is
  part of the system image instead of being in a `/boot` partition ;
- It tells the kernel which system partition to mount as root:
  `root=/dev/mmcblk0p${rootpart}` ;
- Finally, it loads the device tree and passes it to the kernel (with the
  `fdt addr ${fdt_addr}` command). The device-tree is usually
  [appended after the kernel](https://people.kernel.org/linusw/how-the-arm32-linux-kernel-decompresses#decompression-of-the-zimage).
  This little trick is highly specific to the Raspberry Pi and allows
  dynamically loading device tree fragments (we will refer the interested reader
  [to this article](https://dius.com.au/2015/08/19/raspberry-pi-u-boot/) for
  more details). Again, this is something that is specific to your board and
  should be received with your board support package.

### Notifying the server that the update has been received

Our device reboots with the new kernel and the new filesystem. The final step of
our update cycle is to notify the update server that the update has been
installed successfully.

Looking once again at `/usr/lib/swupdate/conf.d/09-swupdate-args`, we notice the
following lines:

```bash
state=$(fw_printenv ustate | cut -f 2 -d'=')
if [ "$state" == 1 ]; then
 # Confirm update Success to server
        SWUPDATE_SURICATTA_ARGS="-c 2"
else
        SWUPDATE_SURICATTA_ARGS=" "
fi
```

The first interesting command here is `fw_printenv` which allows us to read the
U-Boot environment file from our system. For example, you can run
`fw_printenv rootpart` to see on which partition you are currently running.

The second interesting element here is the variable `ustate` that is read by
`swupdate`. This variable was set to `1` by `swupdate` after installing the
update. It is used here to notify the update server that we have successfully
installed the update. It can also be used by more complex bootloader scripts to
identify that an image is “under test” and can be reverted if it does not boot
well.

## Conclusion

We have walked through all the steps required to perform OTA on an embedded
Linux device. All the mysteries and tricks have been revealed. Following the
strong principles of the Unix world, we use simple parts and basic communication
primitives to build a much more complex system.

Knowing now what are the parts and the channels you are equipped to debug issues
and tackle some additional challenges like delta updates and secure boot.

Let us know in the comment below what else you would like to read about in the
world of embedded Linux devices!

# Further Reading

- [The excellent swupdate manual](https://sbabic.github.io/swupdate/swupdate.html)
