---
title: WSL2 for Firmware Development
description: Tutorial for setting up embedded firmware development in WSL2.
author: jphutchins
tags: [zephyr, wsl, wsl2, linux, windows, windows subsystem for linux, usb]
---

<!-- excerpt start -->

This guide provides instructions for setting up an environment for developing,
debugging, and programming embedded systems firmware in the Windows Subsystem
for Linux (WSL2).

<!-- excerpt end -->

WSL2 provides a convenient and stable Linux development environment for working
on embedded systems firmware. If you're curious about toolchain performance,
check out [this comparison of firmware development
environments]({% link _posts/2024-10-10-comparing-fw-dev-envs.md %}), but the
summary is that WSL2 is about 2x the speed of Windows and similar to "bare
metal" Linux. Setup time for WSL2 will vary based on your Windows configuration,
typically taking 10 to 30 minutes.

> This article is based on a living document maintained by
> [Intercreate](https://www.intercreate.io/) and we would welcome feedback and
> contributions at
> [GitHub](https://github.com/intercreate/docs/blob/main/Windows/wsl2.md)!

{% include newsletter.html %}

{% include toc.html %}

## Enabling Virtualization Technology

WSL2 uses a subset of Hyper-V, Microsoft's hypervisor, to virtualize guest
operating systems. Hyper-V uses x86 "Virtualization Technology" to provide guest
operating systems, like Windows or Linux, direct access to the CPU's hardware
virtualization features. These CPU technologies enable the virtualization to run
more efficiently.

When setting up WSL2 for the first time, your computer may not have these x86
virtualization features enabled. To check your settings, go to your laptop or
motherboard's BIOS—
[_probably_ by pressing "Delete" early in the boot](https://www.tomshardware.com/reviews/bios-keys-to-access-your-firmware,5732.html)
—and search for settings related to CPU features. If you are using an Intel
processor, the feature is called
[VT-x](https://en.wikipedia.org/wiki/X86_virtualization#Intel-VT-x), and on AMD
it is called
[AMD-V](<https://en.wikipedia.org/wiki/X86_virtualization#AMD_virtualization_(AMD-V)>).

## Windows Dependencies and Configuration

With hardware virtualization technology enabled, you are ready to configure your
Windows 11 installation for WSL2.

While many of these same techniques are possible in Windows 10, WSL2 in Windows
10 is no longer receiving important feature updates. See the official
[installation documentation](https://support.microsoft.com/en-us/windows/ways-to-install-windows-11-e0edbbfb-cfc5-4011-868b-2ce77ac7c70e).

> Important: Before you continue, run Windows Update(s) until up to date.

> Note: Each task in this section is run from within the Windows 11 environment.

### Windows Terminal

Windows Terminal is a GPU-accelerated terminal host that simplifies access to
the Windows shells, e.g. Command Prompt, PowerShell, PowerShell Core, and WSL2.

Windows Terminal should already be installed on recent versions of Windows 11.
In the Start Menu, begin typing `windows terminal` and select the application if
it is found.

If you don't have Windows Terminal yet, install it using PowerShell. After
installation, Windows Terminal is your interface to PowerShell.

```powershell
winget install --id Microsoft.WindowsTerminal -e
```

You can pin Windows Terminal to the taskbar by right-clicking its icon in the
Start Menu and selecting "Pin to taskbar". Your shells can be accessed by
right-clicking on the Windows Terminal icon and selecting the shell from the
context menu.

> You may like to adjust various settings in Windows Terminal, such as
> increasing the line
> [history size](https://learn.microsoft.com/en-us/windows/terminal/customize-settings/profile-advanced#history-size),
> turning off the
> [audible bell](https://learn.microsoft.com/en-us/windows/terminal/customize-settings/profile-advanced#bell-notification-style),
> setting up a [Nerd Font](https://www.nerdfonts.com/) like FiraCode,
> configuring [Oh My Posh](https://ohmyposh.dev/), etc.

Take a look at the official
[installation documentation](https://github.com/microsoft/terminal#installing-and-running-windows-terminal)
for more info.

### Ubuntu 24.04 LTS on Windows Subsystem for Linux (WSL2)

> These days, WSL2 is the default instead of WSL1. If you are on a fresh
> install, you can assume that WSL2 is being used. If you are worried that you
> might be on WSL1, then
> [upgrade version from WSL 1 to WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install#upgrade-version-from-wsl-1-to-wsl-2).

Open PowerShell as Administrator from Windows Terminal or from PowerShell.

- Terminal
  - Right-click Terminal icon
  - Right-click Terminal
  - Click "Run as administrator"
- Windows PowerShell
  - Right-click Windows PowerShell icon
  - Click "Run as administrator"

> If you're feeling adventurous, you can update your Windows 11 and
> [enable sudo for Windows](https://learn.microsoft.com/en-us/windows/sudo/). So
> happy to have this at last!

To view the Linux distros available from Microsoft:

```powershell
wsl --list --online
```

Of the available Linux distros, I recommend Ubuntu 24.04 for its long-term
support (LTS). Likewise, when it's past April of 2026, I will recommend Ubuntu
26.04, unless something radically changes in the Linux distro landscape. You can
install and use multiple distros simultaneously; because they all run on the
same WSL2 kernel, storage usage is efficient, only using space for each distro's
apps and home folder. For example, I've found that it's convenient to have both
Ubuntu 22.04 and 24.04 instances available because some of my ongoing projects
are locked to Ubuntu 22.04 in CI/CD.

```powershell
wsl --install -d Ubuntu-24.04 --web-download
```

> Although the Microsoft documentation claims this is done automatically, you
> may have to manually enable the virtual machine optional component. In this
> case, you may see the error message "WslRegisterDistribution failed with
> error: 0x80370114". It can be fixed by enabling the Virtual Machine Platform.
>
> Go to Control Panel → Programs → Programs and Features → Turn Windows Features
> On or Off.
>
> Make sure that "Virtual Machine Platform" is checked.

Microsoft maintains
[WSL2 installation documentation](https://learn.microsoft.com/en-us/windows/wsl/install)
that may answer further questions.

### USBIPD Installation using the WSL USB GUI

USB IP Device allows pass-through of USB devices (serial, debugger, etc.) from
Windows to WSL2.

[USBIPD-win](https://github.com/dorssel/usbipd-win) is an open-source project
maintained by Frans von Dorsselaer with support from Microsoft. This provides
the USBIPD client for Windows which will connect to the USBIPD server in the
WSL2 kernel.

You can install USBIPD-win from winget:

```powershell
winget install usbipd
```

`USBIPD` provides a command line interface (CLI) for attaching and detaching
devices to WSL2 from Windows.

Personally, I prefer a GUI for organizing my USB device forwarding.
Specifically, the GUIs implement auto-attach, which is useful for testing device
firmware upgrades done from a bootloader, and for Zephyr Twister runs in which
the device under test (DUT) is providing the USB transport and therefore
attaching/detaching between each test. I've long used Andrew Leech's
[WSL USB GUI](https://gitlab.com/alelec/wsl-usb-gui/-/releases), which I
recommend for its stability and signature verification.

Recently, I've been contributing to Niccolò Betto's
[WSL USB Manager](https://github.com/nickbeth/wsl-usb-manager), and I use it
instead of WSL USB GUI because it's faster, smaller, has a simpler code base. I
prefer the user interface, too. Particularly on
[my branch](https://github.com/nickbeth/wsl-usb-manager/pull/6) that adds
attach/detach capability to the system tray context menu. We don't have signing
and release on Winget or the Microsoft Store yet, but it's on the roadmap, and
we'd welcome more users and contributors!

> While it is possible to forward all USB devices to WSL2, it does not
> necessarily mean that they will "just work". In the firmware development
> domain, an example of this issue is found when forwarding BLE adapters to
> WSL2. The default WSL2 kernel is not compiled with Bluetooth support, so the
> USBIPD community maintains a thread explaining how to compile your own WSL2
> kernel (it's fast!) and get your Bluetooth device working.
>
> The [GitHub thread](https://github.com/dorssel/usbipd-win/discussions/310).

### VSCode

> Not required for developing within WSL2 but it's very well supported.

VSCode can be installed on Windows via winget or by using the
[installer](https://code.visualstudio.com/download#).

```powershell
winget install -e --id Microsoft.VisualStudioCode
```

## Setup WSL2 on First Boot

Now that you have set up the hardware and installed WSL2 in Windows 11, it's
time to launch the WSL2 instance. If you're familiar with Linux, much of this
section will be familiar to you. However, it may still be worth skimming,
especially to understand exactly what is running in Linux compared to running in
Windows.

### Launch and Create a User

Open WSL2 by right-clicking on Terminal and selecting Ubuntu 24.04 LTS. I
recommend choosing a short, simple username, such as your first name in
lowercase. Additionally, I'll offer that the password can be very simple since
your WSL2 instance is already protected by your Windows account login and full
disk encryption. The scary reality is that if a bad actor is at your WSL2 login,
then they already have access to your Windows account and filesystem - one more
barrier at the WSL2 login isn't going to impede them!

> By opening WSL2 in this way, you are interacting with the Linux kernel in a
> Linux filesystem. This is distinct from typing `bash` from a folder on your
> Windows filesystem. `bash` can be a useful tool from within the Windows
> filesystem, but serious Linux development work that will be impacted by
> filesystem performance, such as compilation, should be done from the Linux
> filesystem

### Get Common Dependencies

It's nice to make sure your distro is up to date right away:

```bash
sudo apt update && sudo apt upgrade -y
```

Here are some dependencies that you might want for firmware development. They
are taken from the
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)—
feel free to skip this if you know exactly what you need and want to avoid
bloat.

```bash
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

### Test the WSL2 GUI

To demonstrate launching graphical Linux applications in WSL2, we'll install and
test the Firefox browser.

To install Firefox in WSL2:

```bash
sudo apt install firefox
```

> You can also
> [install Chrome](https://github.com/intercreate/docs/blob/main/Windows/wsl2.md#google-chrome),
> but it's more involved since it's not in the package manager.

And to run it:

```bash
firefox
```

It's important to note that the Firefox application and graphics are running in
WSL2, not Windows. This is in contrast to Terminal or VSCode.

The Terminal application is running in Windows and you can think of its
connection to WSL2 being like an SSH connection to the WSL2 SSH server.

VSCode is a bit more complicated - there's some "syntactic sugar" happening.
When you launch VSCode "from WSL2" - `code .`, for example - the VSCode GUI
opens in Windows, and the shell that called `code` returns from the call. One
thing is happening in each operating system here: in Linux, a VSCode server has
started, including some (but not all) extensions, such as Language Server
Protocol (LSP) processes. As far as I understand it, this is similar to the
VSCode server that you might run on any headless Linux and access through an SSH
session, whether that Linux is on a System On Module (SOM) or a cloud server.
Meanwhile, Windows has launched the VSCode GUI with instructions to connect to
the WSL2 VSCode server.

In both cases, the GUI running in Windows allows for native GPU acceleration and
native latency, combining for an efficient and seamless virtualization user
experience.

### Setup Git

- Set your global git config (same as in Windows):

  ```bash
  git config --global user.name "John Doe"
  git config --global user.email johndoe@example.com
  git config --global init.defaultBranch main
  ```

- Create and add SSH keys to GitHub and BitBucket (as necessary):
  - [Github](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account?platform=linux)
  - [BitBucket](https://support.atlassian.com/bitbucket-cloud/docs/set-up-personal-ssh-keys-on-linux/)

### Install Segger J-Link Software

You may not use Segger J-Link, and in that case you can skip this section.
However, you might not want to since it provides an example workflow for dealing
with graphical End User License Agreements (EULAs) driven by PHP or JavaScript
that are sometimes difficult to manage from headless development environments
like WSL2. This section demonstrates how to work around this by taking advantage
of WSL2's graphics support.

Launch Firefox from your WSL2 terminal: `firefox`. Using WSL2's Firefox,
[download the latest Linux 64-bit DEB installer](https://www.segger.com/downloads/jlink/).

Enter the following commands to install:

```bash
sudo apt install ./snap/firefox/common/Downloads/<name of package>
rm ./snap/firefox/common/Downloads/<name of package>
```

For example, if the latest is `V796m`:

```bash
sudo apt install ./snap/firefox/common/Downloads/JLink_Linux_V796m_x86_64.deb
rm ./snap/firefox/common/Downloads/JLink_Linux_V796m_x86_64.deb
```

You can test the installation by running:

```bash
JLinkExe
```

> Yes, it's really `JLinkExe` - meaning that from Windows Terminal it is invoked
> by `JLink` _or_ `jlink` - because `.exe` is implicit and Windows is generally
> case-insensitive - and in Linux, it's spelled `JLinkExe`, with capital
> letters. What a time to be alive!

## Summary

I hope that this has demystified the process of setting up WSL2 for embedded
systems firmware development. This guide was originally a tutorial maintained by
[Intercreate](https://www.intercreate.io/)
[here](https://github.com/intercreate/docs/blob/main/Windows/wsl2.md), with
contributions from Michael Brust, Alden Haase, JP Hutchins, and Ishani Raha. If
you're running through first-time setup, the step-by-step format of that page
may provide further assistance. Also, if you're starting to dig into some more
complicated use cases, I highly recommend reviewing Craig Buckler's
[Complete Tutorial at sitepoint](https://www.sitepoint.com/wsl2/).

We'd love to hear from you! Do you have any tips or tricks for improving WSL2?

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
