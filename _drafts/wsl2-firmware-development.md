---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
#
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
#
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md
#
# It will now show up in the front page when running Jekyll locally.

title: WSL2 for Firmware Development
description:
  Article and tutorial about using WSL2 for embedded firmware development.
author: JP Hutchins
tags: [zephyr, wsl, wsl2, linux, windows, windows subsystem for linux]
mermaid: true
---

<!-- excerpt start -->

The Windows Subsystem for Linux 2 (WSL2) provides a convenient, fast, and stable
development environment for embedded firmware.  This article discusses
the pros and cons of WSL2 and presents a complete tutorial for setting up your WSL2
environment for compiling, flashing, and on-target debugging.

<!-- excerpt end -->

About a year and a half ago, in March of 2023, I took the leap: beginning a new project
for one of Intercreate's clients, I decided to set up a Zephyr environment in
WSL2 instead of my trusty VMWare Workstation Linux VM.  Not only did everything go
smoothly, but since then, many colleagues have also moved from
native Linux or traditional VMs to WSL2 and have seen excellent outcomes.

{% include newsletter.html %}

{% include toc.html %}

## Features and Limitations

This is a minimal summary intended to guide you on the trade-offs that come with
the use of various development environments.  It is by no means exhaustive and I'd
welcome further feedback in the comments!

|                          | Ubuntu 24.04 (Kernel 6.8) | Windows 11 | VMWare Workstation Pro | WSL2 2.2.4.0 (Kernel 5.15) |
| ------------------------ | ------------------------- | ---------- | ---------------------- | -------------------------- |
| Graphics                 | ✅                         | ✅          | ✅*                     | ✅                          |
| Network                  | ✅                         | ✅          | ✅                      | ✅                          |
| USB                      | ✅                         | ✅          | ✅                      | ✅                          |
| BLE                      | ✅                         | ✅          | ✅                      | ✅**                        |
| Performance***           | 100%                      | ~50%       | ?                      | ~88%                       |
| Developer Experience**** | ⭐⭐⭐⭐                      | ⭐⭐         | ⭐⭐⭐                    | ⭐⭐⭐⭐                       |

> \* VMWare Workstation Pro, when run headless, has the typical "X forwarding" setup
curve.  You can choose to emulate the graphics instead of forwarding
if you are OK with the overhead and latency.

> \*\* Not supported by default, discussed [below](#ble).

> \*\*\* % of bare metal Ubuntu 24.04 performance, based on a Zephyr benchmark, discussed [below](#performance).

> \*\*\*\* The author's personal opinion, discussed [below](#developer-experience).

### Graphics

If you've previously setup X forwarding in WSL2, you'll be happy to learn that this is no longer
necessary.  Linux GUIs "Just Work" in WSL2!

### Network

It’s no surprise that WSL2 can do anything you need over the network, since one of its
primary use cases is web development.

### USB

[usbipd-win](https://github.com/dorssel/usbipd-win) is
[officially recommended by Microsoft](https://learn.microsoft.com/en-us/windows/wsl/connect-usb)
and provides a stable foundation for USB forwarding including automatic attach
by both port and device.

### BLE

Even laptop BLE chips use USB, so you can easily forward an integrated or
dongled BLE device to WSL2.  However, the default WSL2 kernel does not include
`CONFIG_BT=y` and other modules that may be required for your specific chip. You
will need to compile your own WSL2 kernel (only takes about 10 minutes!) and see
the [discussion](https://github.com/dorssel/usbipd-win/discussions/310) over
at usbipd-win.  Please [let Microsoft know](https://github.com/microsoft/WSL/issues/242)
that this feature would be appreciated!

### Performance

There is a lot of confusion about WSL2 performance that's caused
by experiences with earlier versions of WSL or use of the Windows filesystem.  Regarding
the filesystem, it's important to understand that WSL2 creates a complete Linux
filesystem, from `/` up, and that you want to do your work there, probably under
`/home/<user/`, just like you would on Linux -- because you are on Linux!  While it's nice
that you can use `bash` (or `zsh`, or whatever) and Linux apps from anywhere on your Windows
filesystem, there is a performance cost for doing so.

The performance differences between WSL2 and bare metal
Linux are [small](https://www.phoronix.com/review/windows11-wsl2-zen4/5),
[will continue shrinking](https://www.phoronix.com/news/Microsoft-WSL2-Linux-6.6-Kernel)
, and are not likely to meaningfully impact your workflow.

#### Zephyr Benchmark

<div class="mermaid">
xychart-beta 
    title "Zephyr Benchmark"
    x-axis ["Ubuntu 24.04 6.8", "WSL2 Ubuntu 24.04 5.15", "WSL2 Ubuntu 24.04 6.1", "Windows 11"]
    y-axis "Duration in seconds (lower is better)" 0 --> 1440
    bar [664.44, 754.642, 723.323, 1393.901]
</div>

Recently, Intercreate upgraded my work laptop to the best I could find: a
Framework 16.  This laptop put me in a uniquely convenient position to run a
benchmark due to how easy it is to run both Windows and Linux.  Personally, I don’t enjoy
partitioning disks for dual booting.  While I appreciate improvements to
security like TPM, UEFI, Secure Boot, and Full Disk Encryption, they make the
dual booting process nerve-wracking enough that I am no longer willing do it on a
machine that I depend on at work.

Not only does the Framework have two (relatively accessible) M.2 drive slots,
it can be configured with
[SSD expansion cards](https://frame.work/products/storage-expansion-card-2nd-gen?v=FRACCFBZ0A-2)
that can boot an OS.  This means
that several of these cards with different operating systems and plugged into the
laptop without the performance issues
associated with something like a Live USB, and without the headaches caused by
disk partitioning and GRUB.

So, I set out to quantify the performance trade-offs between developing Zephyr
on Native Ubuntu 24.04, WSL2 (Ubuntu 24.04), and Native Windows 11.

#### The Test Fixture

Hardware: Framework 16
- CPU: AMD Ryzen 7 7840HS
  - Geekbench 6: 11,134
    - similar to an Intel Ultra 9 185H (12,110) or Apple M3 @ 4.1GHz
      8 CPU/10 GPU (11,704)
- RAM: 32GB 5600 MHz DDR5 (2x CT16G56C46S5.M8G)
- Windows (and WSL2) Storage: 2TB NVME (CT2000T500SSD8)
- Ubuntu Storage: 250GB USB 3.2 Gen 2 (FRACCFBZ02-2)

Zephyr Toolchain
- Zephyr @ 40810983ead23c954bce113cb7ace50592451da4 (3.7.0-rc2)
- Zephyr SDK 0.16.8
- cmake 3.28.3
- Python 3.12.4
- ninja 1.11.1

Benchmark

A benchmark shouldn't take too long, yet it should be long enough that it
smooths over some run-to-run issues you might see—_what are
Windows Defender and Windows Update up to during the run🔍?_.  A build-only twister
run of 244 builds representing 4 common platforms from ST, Nordic, Espressif, and NXP,
was chosen in the hope that it overlaps with many developer's everyday Zephyr workflows.  The `drivers` test
folder was chosen because most projects would use some drivers and the build
times weren't too long.

```
./scripts/twister \
  -p esp32c3_devkitm \
  -p nrf52840dk/nrf52840 \
  -p mimxrt1060_evk \
  -p nucleo_g474re \
  -T tests/drivers
```
244 Builds
- 55 nucleo_g474re (ST)
- 108 nrf52840dk (Nordic)
- 32 esp32c3_devkitm (Espressif)
- 49 mimxrt1060_evk (NXP)

Problems

- A new reason not to use Windows as your development environment!  While
  path length limitations have been mitigated by Microsoft, toolchains have not caught
  up.  I gave up trying to fix it and I'm not sure whether it's a CMake or Ninja
  problem.  Even after I used a workaround where I shortened the paths by assigning the
  root build path to a driver letter, still 48 of the 244 tests errored due to
  some object in the build exceeding the max path length.  This is an open
  [issue](https://github.com/zephyrproject-rtos/zephyr/issues/36358) at Zephyr.
- The native Ubuntu run is on a different disk than the Windows and WSL2 tests.
  While the Windows NVME drive is faster than the USB 3.2 Gen 2 drive, the test is
  expected to be primarily CPU bound instead of being bound by the 1,000MB/s read
  and 800MB/s write speeds of the SSD expansion card.
- Operating systems.  They do stuff.  It's hard to know why and when they do
  that stuff!  All tests were run with a single Chrome instance open on a
  default tab and the system set to "maximum performance".  Time permitting, the
  benchmark would be run many times.
- Test selection could be refined to cover a wider variety of build scenarios.

#### Native Ubuntu 24.04

```
Operating System: Ubuntu 24.04 LTS                  
          Kernel: Linux 6.8.0-36-generic
    Architecture: x86-64
```

All other platforms are compared to the Ubuntu results because Ubuntu is the
fastest.

| Metric                  | Result     | Compared to Native Ubuntu |
| ----------------------- | ---------- | ------------------------- |
| Duration                | 11m 4.444s | -                         |
| Average build duration  | 2.723s     | -                         |
| Workload multiplier     | 1.000      | -                         |
| Duration of a 40s build | 40.000s    | -                         |


![Screenshot of the native Ubuntu benchmark run]({% img_url wsl2-firmware-development/ubuntu-twister-run.png %})

#### WSL2 Ubuntu 24.04

```
  Virtualization: wsl
Operating System: Ubuntu 24.04 LTS
          Kernel: Linux 5.15.153.1-microsoft-standard-WSL2
    Architecture: x86-64
```

| Metric                  | Result                    | Compared to Native Ubuntu |
| ----------------------- | ------------------------- | ------------------------- |
| Duration                | 12m 34.642s               | + 1m 30.642s              |
| Average build duration  | 3.092s                    | + 0.369s                  |
| Workload multiplier     | 754.642s / 664.444s       | * 1.136                   |
| Duration of a 40s build | 40.000s * 1.136 = 45.440s | + 5.440s                  |

![Screenshot of the WSL2 5.15 benchmark run]({% img_url wsl2-firmware-development/wsl2-twister-run-2.png %})

I also compiled Kernel 6.1 and 6.6 for WSL2 and saw that Kernel 6.1 shrank the
performance gap by about 4%.  In my opinion, that's not worth it, but it shows
that the WSL2's upcoming upgrade to Kernel 6.6 may come with some more performance
gains.

```
  Virtualization: wsl
Operating System: Ubuntu 24.04 LTS
          Kernel: Linux 6.1.21.2-microsoft-standard-WSL2+
    Architecture: x86-64
```

![Screenshot of the WSL2 6.1 benchmark run]({% img_url wsl2-firmware-development/wsl2-twister-run-kernel6.1.png %})

#### Windows 11

> Tip: One of the main things that slows down build systems in Windows is Windows Defender.  Build
systems do a ton of file IO and Windows Defender scans go wild during the build.  For
this test, I disabled Windows Defender entirely, which you should never do.  Instead,
you should painstakingly ignore your build folders and toolchains in
Defender settings and pray for better support from Microsoft someday!

> Note: Windows 11 wasn't able to properly complete the benchmark due
to issues with long paths generated by the build system.  I've compiled Zephyr
projects on Windows many times without (too many) problems, but this issue
does give me pause. Well, that, and it being half the speed of WSL2, if we're
being GENEROUS by disabling Windows Defender!

> Note: I made a few patches to improve Twister’s Windows support which is why
a diff is shown in the prompt.

| Metric                  | Result                                                      | Compared to Native Ubuntu |
| ----------------------- | ----------------------------------------------------------- | ------------------------- |
| Duration                | 23m 13.901s (includes a 274.201s penalty for errored tests) | + 12m 9.457s              |
| Average build duration  | 5.713s                                                      | + 2.990s                  |
| Workload multiplier     | 1,393.901 / 664.444                                         | * 2.098                   |
| Duration of a 40s build | 40.000s                                                     | + 43.920s                 |

![Screenshot of the Windows benchmark run]({% img_url wsl2-firmware-development/windows-twister-run-without-defender.png %})

### Developer Experience

This is a subjective 5-star rating of my personal Firmware Developer Experience with
the platforms discussed in this article.  I'd love to hear about other people's
experiences!

#### Linux

⭐⭐⭐⭐

Pros:
- Control: choice of distro, desktop environment, kernel, and more.
- Zephyr performance: fastest of the platforms tested
- It’s free!
- 🐧 Cute penguin mascot!

Cons:
- Security:
  - Full disk encryption is not default in major desktop distros like
  Ubuntu and Fedora, leaving it up to IT departments or employees to enable
  and support.
  - Many companies and developers distribute software and scripts that are unsigned,
  leaving users open to attacks that impersonate their tools.  Install scripts may
  require sudo and users that have become used to running scripts like this may be
  unaware that they are willfully disabling Linux’s security model.  Heck, I’ve
  even seen inline binary in unsigned bash scripts - a hacker’s dream!
- Hardware Support: Linux desktops tend to be much slower to receive driver
  support from manufacturers -- if they receive driver support at all.  Sometimes
  you have to wait for a community developed solution.
- Software Support: many tools in the Hardware and Electrical Engineering fields
  do not have Linux support.
- Your clients
  [probably don't use Linux desktop](https://gs.statcounter.com/os-market-share/desktop/worldwide).
  So, when you're developing tooling for them, you'll probably want to test on
  MacOS or Windows anyway.

In a bubble, booting Linux provides the perfect experience for development.
However, considering the wider ecosystems, I chose to dock one star for the
baggage that comes with Linux desktop: no agreed upon FDE, hardware and software
compatibility issues, security hubris, and minimal market share.

#### Windows 11

⭐⭐

Oh, Windows.  WTF.  Microsoft has made a lot of progress towards making software
development a first-class experience, but there's still a lingering _je ne sais
quoi_ when using Windows.

Well, now we know one thing: it runs Zephyr builds at half the speed of
Linux!

Pros:
- Security: 
  - full disk encryption (BitLocker) is default and your recovery key is backed
    up to your Microsoft account
  - unsigned applications and scripts produce a warning (this is also a con)
- Hardware Support: if your product doesn't support Windows on release then it's
  dead on arrival.
- Software Support: mechanical and electrical engineering tools have long been
  built for Windows.
- Your clients probably use it.  Developing and testing software on the target
  platform is generally a good idea.
- Stability.  Freezes and crashes are all but nonexistent.  Of course, that’s
  largely anecdotal, as are the instabilities associated with desktop Linux.

Cons:
- Zephyr performance: half the speed of Linux.
- Security:
  - Developer mode removes warnings about unsigned scripts and applications.
  - Microsoft requires yearly payment — previously, to an approved certificate
  authority, now you have the option of paying MS directly — in order to sign scripts
  and applications.  This has resulted in the widespread use of unsigned scripts
  and applications.  Even huge projects like Rye and WezTerm will trigger false
  positives for Trojans in Windows Defender and require the user to click through
  “scary warnings”.  The global software ecosystem lives and dies by OSS and
  creating barriers to signing and authentication of OSS is dangerous for
  Microsoft’s customers and business.
- It’s $200, but you might never notice that when purchasing new laptops.
- It’s Windows.  This is more of a “gut feeling” thing that I can best explain 
  with an anecdote.
  
  As I was setting up my new laptop, I used my Microsoft
  account when installing Windows, something that I had long avoided, not really
  for any specific reason.  It felt like I could potentially avoid “Windows problems”
  by being compliant here.  It went OK, except that it never prompted me for the
  name of my home (AKA user) folder and then it named it incorrectly: “jphut” instead of
  “jp”.  Now, many people would have left it there.  And perhaps I should have. 
  But “jphut” is _wrong_.  Home is at `/home/jp`, or `c:/users/jp`, or `/users/jp` -
  this is how it has _always been_.  “No big deal”, I thought. “I’ll fix it!” 
  
  It took me 3 hours.  RegEdit.  Reviewing scripts left on Windows forums. 
  Seriously breaking my install.  Adding symlinks for OneDrive.  It’s been working
  fine for a few months now, but for all I know I’m out of spec in a weird way that
  will cause problems.  So, to quote Linus Torvalds, “F@&# you, \[Microsoft\]!”
- 📎Unsettling paperclip mascot!

#### VMWare Workstation Pro

⭐⭐⭐


Pros:
- With a fully featured VM, you get most of the pros of Linux with access to Window’s strong points when you need them.
- USB forwarding, including BLE, works well because you can run a full Linux distro of your choice with all the kernel modules that you need.
- SSH to a headless VM allows for a snappy, low-latency, experience in your
  editor of choice.

Cons:
- Setup and maintenance.  VMs are complicated, and setting up and maintaining the VM takes time and research.  A common example is the allocation of disk space.  Generally, you will need to allocate the space for your guest OS up front.  It can be hard to predict exactly how much space you really need, so you may end up needing to expand the virtual disk at some point, requiring steps to be taken from both the host and guest.

#### WSL2

⭐⭐⭐⭐

Pros:
- You get most of the pros of a Linux environment and all of the pros of the Windows host.
- Resource allocation is automatic.  You can fiddle with CPU and RAM allocation, but you shouldn’t need to.  Storage is automatically resized as needed, so you never need to worry about running out of space for your WSL2 environment.
- Setup is very simple — you can follow Intercreate’s tutorial below — and supported by Microsoft.

Cons:
- Minimal choice of distros and fewer options for customizing your installation and kernel.  That’s not to say that you _can’t_, but rather that you will stray outside of the Microsoft support umbrella by doing so.
- BLE, and probably other kernel modules that I haven’t thought of, are not enabled in the default WSL2 configuration.
- Performance is not as good as running Linux on bare metal.

### Conclusion

Ultimately, choosing a development environment depends on what helps you to
provide the most value to your clients.  Decisions about security, performance,
features, and stability must be weighed distinctly within each organization or
project.  Personally, I’ve found a nice balance by using WSL2 on Windows 11. 
I get all of the security, stability, compatibility, and support of Windows while
being able to use my preferred tools in a Linux environment.  My primary concern,
performance, has been eased by quantifying that a WSL2 Zephyr build is only ~12% slower
than bare metal Linux.  If you compile big Zephyr projects with 40s build times - 
_pristine instead of incremental_ - 20 times a day (I don’t), WSL2 costs you an extra 1m 30s
of compile time.  But what does maintaining desktop Linux cost each day?  How
much time do you have for fixing ALSA, compiling a tool that didn’t have a binary
release, fiddling with WINE, or setting up a Windows VM?

If you’re interested in trying out WSL2 for firmware development, continue reading below for a complete tutorial.  This is a living tutorial document maintained by Intercreate and we would welcome feedback and contributions at [GitHub](https://github.com/intercreate/docs/blob/main/Windows/wsl2.md)!
Depending on the state of your Windows host, setting up WSL2 for firmware
development should take between 10 and 30 minutes.

## Tutorial

This guide provides instructions for setting up an environment for developing,
debugging, and programming embedded systems firmware in the Windows Subsystem
for Linux (WSL2).

Contributions by Michael Brust, Alden Haase, JP Hutchins, and Ishani Raha.

### Windows Dependencies and Configuration

Each task in this section is run from within the Windows 11 environment.

#### Windows 11

While many of these same techniques are possible in Windows 10, WSL2 in Windows
10 is no longer receiving important feature updates. It is recommended to run
Windows Update until up to date.

[Installation Documentation](https://support.microsoft.com/en-us/windows/ways-to-install-windows-11-e0edbbfb-cfc5-4011-868b-2ce77ac7c70e)

#### Windows Terminal

Windows Terminal is a GPU accelerated terminal host that simplifies access to
the Windows shells, e.g. Command Prompt, PowerShell, PowerShell Core, and WSL2.

Windows Terminal should already be installed on recent versions of Windows 11.
In the Start Menu, begin typing `windows terminal` and select the application
if it is found.

If you don't have Windows Terminal yet, you'll need to use PowerShell to install
it.  After this, Windows Terminal is your interface to PowerShell.

```powershell
winget install --id Microsoft.WindowsTerminal -e
```

##### Configuration

- Pin Windows Terminal to the taskbar by right-clicking on its icon in the Start
  Menu and selecting "Pin to taskbar".
  - Tip: your shells can be quickly accessed by right-clicking on the Windows
    Terminal icon and selecting your shell from the list.

> You may like to adjust various settings in Windows Terminal, such as
> increasing the line [history size](https://learn.microsoft.com/en-us/windows/terminal/customize-settings/profile-advanced#history-size),
> turning off the [audible bell](https://learn.microsoft.com/en-us/windows/terminal/customize-settings/profile-advanced#bell-notification-style),
> setting up a [Nerd Font](https://www.nerdfonts.com/) like FiraCode,
> configuring [Oh My Posh](https://ohmyposh.dev/), etc.

[Installation Documentation](https://github.com/microsoft/terminal#installing-and-running-windows-terminal)

#### Ubuntu 24.04 LTS on Windows Subsystem for Linux (WSL2)

> These days, WSL2 is the default instead of WSL1.  If you are on a fresh
> install, you can assume that WSL2 is being used.  If you are worried that you
> might be on WSL1, then [upgrade version from WSL 1 to WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install#upgrade-version-from-wsl-1-to-wsl-2).

Open PowerShell as Administrator from Windows Terminal or from PowerShell.

```powershell
wsl --install -d Ubuntu-24.04 --web-download
```

> Note: Although the Microsoft documentation claims this is done automatically,
> you may have to manually enable the virtual machine optional component. In
> this case you may see the error message "WslRegisterDistribution failed with
> error: 0x80370114". It can be fixed by enabling the Virtual Machine Platform.
>
> Go to Control Panel &rarr; Programs &rarr; Programs and Features &rarr; Turn Windows Features On or Off.
>
> Make sure that "Virtual Machine Platform" is checked.

[Installation Documentation](https://learn.microsoft.com/en-us/windows/wsl/install)

##### About Distros

See `wsl --list --online` for more info.

##### About Hardware Configuration

You will need to enable virtualization support in your system BIOS if it is not
already enabled.  On Intel this is called VT-d and an AMD it is called AMD-V.
See your laptop/motherboard manufacturer's documentation.

Further reading:

- [Set up a WSL development environment](https://learn.microsoft.com/en-us/windows/wsl/setup/environment)
- [Run multiple distros/instances](https://learn.microsoft.com/en-us/windows/wsl/install#ways-to-run-multiple-linux-distributions-with-wsl)

#### USBIPD Installation using the WSL USB GUI

USB IP Device allows pass through of USB devices (serial, debugger, etc.) from
Windows to WSL2. Install the WSL USB GUI
[most current release](https://gitlab.com/alelec/wsl-usb-gui/-/releases).

Further documentation for USB forwarding setup without using the GUI found below:
- [usbipd-win source](https://github.com/dorssel/usbipd-win)
- [wsl-usb-gui source](https://gitlab.com/alelec/wsl-usb-gui)
- [Microsoft write-up](https://learn.microsoft.com/en-us/windows/wsl/connect-usb)

> While it is possible to forward all USB devices to WSL2, it does not
> necessarily mean that they will "just work".  In the firmware development
> domain, an example of this issue is found when forwarding BLE adapters to 
> WSL2.  The default WSL2 kernel is not compiled with Bluetooth support, so the
> USBIPD community maintains a thread explaining how to compile your own WSL2
> kernel (it's fast!) and get your Bluetooth device working.
>
> The [THREAD](https://github.com/dorssel/usbipd-win/discussions/310).

#### VSCode

> Not required for developing within WSL2 but it's very well supported.

```powershell
winget install -e --id Microsoft.VisualStudioCode
```

Or installer: https://code.visualstudio.com/download#

### Setup WSL2 on First Boot

#### Create a User

- Open WSL2 by right-clicking on Terminal and selecting Ubuntu 24.04 LTS
  - Or, in an already open Terminal, from the top bar, dropdown V to display
    shells
  - You can set defaults from the Terminal Settings
- Choose a short and normal username, such as your first name, all lowercase
- Choose a short and easy password 
  > OK to use `admin` or similar - if a bad actor has gotten this far they
  > already have access to your entire Windows OS!

#### Get Common Dependencies

- Update packages: 
  ```
  sudo apt update && sudo apt upgrade -y
  ```
- Install suggested packages:
  ```
  sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache device-tree-compiler wget build-essential tio python3-dev \
  python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 firefox clang-format
  ```
  - Test WSL2 GUI:
    ```
    firefox
    ```
    > Firefox is running in WSL2!
- Additional information on USB/IP client tools can be found [here](https://github.com/dorssel/usbipd-win/wiki/WSL-support#usbip-client-tools).

#### Setup Git

- Set your global git config (same as in Windows):
  ```
  git config --global user.name "John Doe"
  git config --global user.email johndoe@example.com
  git config --global init.defaultBranch main
  ```
- Create and add SSH keys to GitHub and BitBucket (as necessary):
  - [Github](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account?platform=linux)
  - [BitBucket](https://support.atlassian.com/bitbucket-cloud/docs/set-up-personal-ssh-keys-on-linux/)

#### Install Segger J-Link Software

> Ideally, packages are obtained from package managers or wget. However, vendors
> will occasionally put the download links behind EULAs that require a GUI to
> accept. This section demonstrates how to work around this by taking
> advantage of WSL2's graphics support!

- From WSL - launch Firefox:
  ```
  firefox
  ```
- Using Firefox, [download the latest Linux 64-bit DEB installer](https://www.segger.com/downloads/jlink/).
- Enter the following commands to install:
  ```
  sudo apt install ./snap/firefox/common/Downloads/<name of package>
  rm ./snap/firefox/common/Downloads/<name of package>
  ```
  For example, if latest is `V796m`:
  ```
  sudo apt install ./snap/firefox/common/Downloads/JLink_Linux_V796m_x86_64.deb
  rm ./snap/firefox/common/Downloads/JLink_Linux_V796m_x86_64.deb
  ```
- Test:
  ```
  JLinkExe -nogui 1
  ```
  > Note that the GUI also will work!

#### Test VSCode

- If you are using VSCode, enter `code .` from a WSL2 Terminal to understand how
  to open VSCode from within WSL2.  Typically this command would be used after
  `cd` to a repository root.

  > VSCode is not running in WSL2!  The VSCode GUI (frontend) is running in
  > Windows while the VSCode server runs in WSL2.  This is akin to establishing
  > an SSH connection to a remote server or local virtual machine.
  >
  > Running `code` from a WSL2 terminal is a special case - a convenient
  > shortcut to launch the VSCode server in Linux and launch a VSCode frontend in
  > Windows that connects to the Linux server.
  >
  > This is similar to how Windows Terminal is interacting with WSL2. In both
  > cases you benefit from the native GPU acceleration of the frontend while
  > interacting with a Linux "server" on the backend.

#### Google Chrome

- Install Google Chrome:
  ```
  cd ~
  wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
  sudo apt install ./google-chrome-stable_current_amd64.deb
  rm google-chrome-stable_current_amd64.deb
  ```
- Test:
  ```
  google-chrome
  ```

## Closing

We'd love to hear from you!  Do you have any tips or tricks for improving WSL2?

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}
