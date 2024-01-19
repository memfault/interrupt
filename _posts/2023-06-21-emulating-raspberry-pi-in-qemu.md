---
title: "Emulating a Raspberry Pi in QEMU"
description: Emulating a Raspberry Pi computer on your desktop in Docker using QEMU - an open source emulator
author: stawiski
tags: [raspberry-pi, qemu, linux, emulator]
---

The Raspberry Pi, a compact single-board computer, is widely used for DIY projects to industrial applications. These devices ship with a customized Linux distribution that differs from standard Linux, adding a layer of complexity for developers trying to troubleshoot application problems and dependencies.

With many available versions and flavors of Raspberry-compatible Linux, it's challenging to debug issues related to dependencies of each Linux version, and reflashing the Raspberry SD card each time with a different image is time-consuming. Emulating a Raspberry Pi with a tool such as QEMU could offer a solution to some of these challenges. While emulation is severely limited, and most hardware interactions will probably lead to application crashing, it is enough to debug dependency issues across Raspberry Linux versions.

This article will delve into the challenges and opportunities of emulating a Raspberry Pi using QEMU. If you don't care about the details, you can jump straight to the [Docker image](#dockerfile) section to get it running.

<!-- excerpt start -->

This article dives into QEMU, a popular open-source emulator, and how to use it to emulate a Raspberry Pi on your desktop. At the end of the article, we will put the whole environment in Docker, so you will be able to emulate a Raspberry Pi by just using a Docker container.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What is QEMU?

[QEMU](https://www.qemu.org/) is an open-source software tool that provides hardware virtualization capabilities. It allows you to run operating systems and programs designed for one architecture on a different architecture, thereby enabling cross-platform emulation. QEMU can emulate various CPUs, including x86, ARM, and more. It can also emulate a variety of devices such as network cards, and storage devices. QEMU is a great tool for testing software on different architectures without having to buy the hardware.

## Environment setup

Since I'm on a Mac, and the purpose is to get a working Docker image, I set up an Ubuntu 20.04 Virtual Machine in Parallels. If you want to follow along, I recommend you to do the same and get onto a fresh Ubuntu 20.04 with QEMU installed which is as simple as
```bash
apt-get install -y qemu-system-aarch64
```

If you want to skip to the working Docker image just go to [Docker image](#dockerfile) section and make sure you have Docker installed on your system (if not go to [https://docs.docker.com/engine/install/](https://docs.docker.com/engine/install/)).

## Getting the Raspberry Pi image

You can find all available Raspberry Pi images on [https://www.raspberrypi.com/software/operating-systems/](https://www.raspberrypi.com/software/operating-systems/). Today we will be using the latest 64-bit image as of the time of writing, which is [Raspberry Pi OS with desktop released on May 3rd 2023](https://downloads.raspberrypi.org/raspios_arm64/images/raspios_arm64-2023-05-03/2023-05-03-raspios-bullseye-arm64.img.xz).

Let's get that image first and unpack it:
```bash
cd ~
wget https://downloads.raspberrypi.org/raspios_arm64/images/raspios_arm64-2023-05-03/2023-05-03-raspios-bullseye-arm64.img.xz
xz -d 2023-05-03-raspios-bullseye-arm64.img.xz
```

Now, let's inspect the image:
```bash
$ fdisk -l ./2023-05-03-raspios-bullseye-arm64.img
Disk ./2023-05-03-raspios-bullseye-arm64.img: 4.11 GiB, 4412407808 bytes, 8617984 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x3e247b30

Device                                   Boot  Start     End Sectors  Size Id Type
./2023-05-03-raspios-bullseye-arm64.img1        8192  532479  524288  256M  c W95 FAT32 (LBA)
./2023-05-03-raspios-bullseye-arm64.img2      532480 8617983 8085504  3.9G 83 Linux
```

## Kernel and device tree

We've got two partitions in the image. The first partition is the boot partition, let's mount it to see what's inside. We need to know the offset of the partition, which is the start sector multiplied by the sector size. In our case, the sector size is 512 bytes, so the offset is 8192 * 512 = 4194304.
```bash
$ sudo mkdir /mnt/image
$ sudo mount -o loop,offset=4194304 ./2023-05-03-raspios-bullseye-arm64.img /mnt/image/
```

And list the mounted directory:
```bash
$ ls -ls /mnt/image/
total 30244
  30 -rwxr-xr-x 1 root root   30390 Apr  5 21:32 bcm2710-rpi-2-b.dtb
  32 -rwxr-xr-x 1 root root   32142 Apr  5 21:32 bcm2710-rpi-3-b.dtb
  32 -rwxr-xr-x 1 root root   32753 Apr  5 21:32 bcm2710-rpi-3-b-plus.dtb
  30 -rwxr-xr-x 1 root root   30285 Apr  5 21:32 bcm2710-rpi-cm3.dtb
  32 -rwxr-xr-x 1 root root   31318 Apr  5 21:32 bcm2710-rpi-zero-2.dtb
  32 -rwxr-xr-x 1 root root   31318 Apr  5 21:32 bcm2710-rpi-zero-2-w.dtb
  52 -rwxr-xr-x 1 root root   52682 Apr  5 21:32 bcm2711-rpi-400.dtb
  52 -rwxr-xr-x 1 root root   52593 Apr  5 21:32 bcm2711-rpi-4-b.dtb
  52 -rwxr-xr-x 1 root root   53202 Apr  5 21:32 bcm2711-rpi-cm4.dtb
  38 -rwxr-xr-x 1 root root   38182 Apr  5 21:32 bcm2711-rpi-cm4-io.dtb
  50 -rwxr-xr-x 1 root root   50504 Apr  5 21:32 bcm2711-rpi-cm4s.dtb
  52 -rwxr-xr-x 1 root root   52476 Apr  5 21:32 bootcode.bin
   2 -rwxr-xr-x 1 root root     193 May  3 13:23 cmdline.txt
   4 -rwxr-xr-x 1 root root    2109 May  3 12:53 config.txt
  20 -rwxr-xr-x 1 root root   18693 Apr  5 21:32 COPYING.linux
...
```

We can see classic Raspberry Pi `/boot` files here such as `cmdline.txt` and `config.txt`. Have in mind, if we make any modifications here to the partition, the changes will be saved to the image.

To run QEMU we will need the kernel and device tree, so let's copy them out:
```bash
$ cp /mnt/image/bcm2710-rpi-3-b-plus.dtb ~
$ cp /mnt/image/kernel8.img ~
```

## Setting up SSH

While we have the image mounted, let's sort out the SSH connection as well. Raspberry changed its default password policy so we can no longer use the well-known `pi:raspberry` combination to log in. We'll need to create a new password first and put it in `userconf` file in the boot partition. `userconf` expects a username and a hashed password. Storing passwords in plaintext is considered a bad practice, so we'll use `openssl` to generate a secure hashed version of our new password.

Now, let's recreate the password `raspberry` using `openssl`:
```bash
$ openssl passwd -6
Password:
Verifying - Password:
$6$rBoByrWRKMY1EHFy$ho.LISnfm83CLBWBE/yqJ6Lq1TinRlxw/ImMTPcvvMuUfhQYcMmFnpFXUPowjy2br1NA0IACwF9JKugSNuHoe0
```

Openssh asks the user to type in the password twice, so I just typed `raspberry`. We got the hash, which we can now put in the image alongside with the username `pi`:
```bash
$ echo 'pi:$6$rBoByrWRKMY1EHFy$ho.LISnfm83CLBWBE/yqJ6Lq1TinRlxw/ImMTPcvvMuUfhQYcMmFnpFXUPowjy2br1NA0IACwF9JKugSNuHoe0' | sudo tee /mnt/image/userconf
```

And let's not forget to enable SSH by creating an empty file `ssh`:
```bash
$ sudo touch /mnt/image/ssh
```

## Running QEMU

Now we have everything we need. To run QEMU one last thing to sort out is the image size. QEMU will complain if the image size is not a power of 2, so let's fix it by resizing our image to 8G:
```bash
$ qemu-img resize ./2023-05-03-raspios-bullseye-arm64.img 8G
WARNING: Image format was not specified for './2023-05-03-raspios-bullseye-arm64.img' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
Image resized.
```

Great! Now we are ready to start QEMU. This is what we'll use:
- our resized image
- extracted kernel and device tree
- Raspberry Pi 3B+ with 1GB of RAM and 4 cores
- configure serial console
- configure networking to connect to SSH via emulated USB network adapter

Putting all of these together, we get the following command:
```bash
qemu-system-aarch64 -machine raspi3b -cpu cortex-a72 -nographic -dtb bcm2710-rpi-3-b-plus.dtb -m 1G -smp 4 -kernel kernel8.img -sd 2023-05-03-raspios-bullseye-arm64.img -append "rw earlyprintk loglevel=8 console=ttyAMA0,115200 dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2 rootdelay=1" -device usb-net,netdev=net0 -netdev user,id=net0,hostfwd=tcp::2222-:22
```

After a couple of seconds, you should see the output from the kernel bootup. Eventually, you will see the login prompt:
```bash
         Starting Light Display Manager...
[  OK  ] Started OpenBSD Secure Shell server.
[  OK  ] Finished Turn on SSH if /boot/ssh is present.
[  OK  ] Finished Rotate log files.

Debian GNU/Linux 11 raspberrypi ttyAMA0

raspberrypi login:
```

In a separate shell, you should be able to SSH to the emulated Raspberry now:
```bash
$ ssh -p 2222 pi@localhost
The authenticity of host '[localhost]:2222 ([127.0.0.1]:2222)' can't be established.
ED25519 key fingerprint is SHA256:Rjgd9NJvyQKYIisy7gPwcDop2hrk8BXC9IajVNqWVvE.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '[localhost]:2222' (ED25519) to the list of known hosts.
pi@localhost's password:
Linux raspberrypi 6.1.21-v8+ #1642 SMP PREEMPT Mon Apr  3 17:24:16 BST 2023 aarch64

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Wed May 17 12:17:55 2023

SSH is enabled and the default password for the 'pi' user has not been changed.
This is a security risk - please login as the 'pi' user and type 'passwd' to set a new password.

pi@raspberrypi:~ $
```

It works! This wasn't too bad, but it's still a couple of steps to follow. Let's see if we can put this in a Docker container.

## Putting it all in Docker

Now, let's put this in a Docker image, so emulating our Raspberry Pi will be as simple as pulling an image and calling `docker run`. We'll need to automate a couple of tricky things:
1. resize the image to the next power of 2 size
1. mount the image to extract the kernel and device tree from the image
1. enable SSH and set the password

### Resizing the image

To resize the image we need to know the current image size, which is easy:
```Dockerfile
RUN CURRENT_SIZE=$(stat -c%s "2023-05-03-raspios-bullseye-arm64.img")
```

To find the next power of two we can use this little Python script:
```python
import math
print(2**(math.ceil(math.log(CURRENT_SIZE, 2))))
```

Altogether, we get this snippet:
```Dockerfile
RUN CURRENT_SIZE=$(stat -c%s "2023-05-03-raspios-bullseye-arm64.img") && \
    NEXT_POWER_OF_TWO=$(python3 -c "import math; \
                                    print(2**(math.ceil(math.log(${CURRENT_SIZE}, 2))))") && \
    qemu-img resize "2023-05-03-raspios-bullseye-arm64.img" "${NEXT_POWER_OF_TWO}"
```

Which will neatly resize any image to the next power of two size.

### Mounting the image

To mount the image in Dockerfile, we won't be able to use standard `mount`. Instead, we'll use mtools. We'll also need to figure out the offset of the partition, as we don't want to hardcode it to `4194304`. We can do this by using `fdisk` and `awk`:
```Dockerfile
RUN OFFSET=$(fdisk -lu 2023-05-03-raspios-bullseye-arm64.img | awk '/^Sector size/ {sector_size=$4} /FAT32 \(LBA\)/ {print $2 * sector_size}')
```

And to mount the image and extract the kernel and device tree:
```Dockerfile
RUN echo "drive x: file=\"2023-05-03-raspios-bullseye-arm64.img\" offset=${OFFSET}" > ~/.mtoolsrc && \
    mcopy x:/bcm2710-rpi-3-b-plus.dtb . && \
    mcopy x:/kernel8.img .
```

This will work for any image that contains a FAT32 partition, and specified files in the root of that partition, which are all current Raspberry Pi images.

### Enabling SSH and setting the password

This is the easiest part. We'll hardcode the password to the previously generated `raspberry` hash. We just need two files that we put into the mounted drive `x:` from the previous step:
```Dockerfile
RUN mkdir -p /tmp && \
    touch /tmp/ssh && \
    echo 'pi:$6$rBoByrWRKMY1EHFy$ho.LISnfm83CLBWBE/yqJ6Lq1TinRlxw/ImMTPcvvMuUfhQYcMmFnpFXUPowjy2br1NA0IACwF9JKugSNuHoe0' | tee /tmp/userconf && \
    mcopy /tmp/ssh x:/ && \
    mcopy /tmp/userconf x:/
```

That's it!

## Dockerfile

Here's the full Dockerfile, based on Ubuntu 20.04: [link](https://github.com/memfault/interrupt/tree/master/example/emulating-raspberry-pi-in-qemu/Dockerfile)

If you don't want to build this, feel free to pull my image from Docker Hub:
```bash
docker pull stawiski/qemu-raspberrypi-3b:2023-05-03-raspios-bullseye-arm64
```

Running the container is as simple as:
```bash
docker run -it --rm -p 2222:2222 stawiski/qemu-raspberrypi-3b:2023-05-03-raspios-bullseye-arm64
```

And you can SSH into it in via port 2222 using the `pi` user and `raspberry` password:
```bash
ssh -p 2222 pi@localhost
```

## Conclusion

We Dockerized the latest Raspberry Pi 64-bit OS and exposed its SSH port. This is a great starting point for any Raspberry Pi development, as it allows you to run it on any machine that supports Docker. You can use this to run your CI/CD pipelines, provided you don't need any hardware access or are willing to emulate the hardware as well.

As for myself, I used this to figure out Linux package dependencies for a particular Raspberry Pi OS version, and it wasn't much longer than installing the OS on a real Raspberry Pi. However, having put it in Docker, I can reuse this setup to quickly emulate any other Raspberry Pi OS version.

Hope this helps!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- <https://github.com/qemu/qemu>
- <https://github.com/raspberrypi/firmware>
- <https://docs.docker.com/engine/reference/builder/>
- <https://www.gnu.org/software/mtools/manual/mtools.html>
- <https://raspberrypi.stackexchange.com/questions/138548/unable-to-login-in-qemu-raspberry-pi-3b>
- <https://stackoverflow.com/questions/61562014/qemu-kernel-for-raspberry-pi-3-with-networking-and-virtio-support>
- <https://raspberrypi.stackexchange.com/questions/117234/how-to-emulate-raspberry-pi-in-qemu>
- <https://blog.grandtrunk.net/2023/03/raspberry-pi-4-emulation-with-qemu>

<!-- prettier-ignore-end -->
