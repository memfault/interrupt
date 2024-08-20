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
author: jphutchins
tags: [zephyr, wsl, wsl2, linux, windows, windows subsystem for linux]
---

<!-- excerpt start -->

This guide provides instructions for setting up an environment for developing,
debugging, and programming embedded systems firmware in the Windows Subsystem
for Linux (WSL2).

<!-- excerpt end -->

WSL2 provides a convenient and stable Linux development environment for working
on embedded systems firmware. If you're curious about toolchain performance, see
[this article]({{ site.baseurl }}{% link _drafts/zephyr-development-environments.md %}), 
but the summary is that WSL2 is about 2x the speed of Windows and similar to 
"bare metal" Linux.  Depending on the state of your Windows host, setting up
WSL2 for firmware development should take between 10 and 30 minutes.

This tutorial is a living document maintained by
Intercreate and we would welcome feedback and contributions at
[GitHub](https://github.com/intercreate/docs/blob/main/Windows/wsl2.md)!

Contributions by Michael Brust, Alden Haase, JP Hutchins, and Ishani Raha.

{% include newsletter.html %}

{% include toc.html %}

## Windows Dependencies and Configuration

Each task in this section is run from within the Windows 11 environment.

### Windows 11

While many of these same techniques are possible in Windows 10, WSL2 in Windows
10 is no longer receiving important feature updates. It is recommended to run
Windows Update until up to date.

[Installation Documentation](https://support.microsoft.com/en-us/windows/ways-to-install-windows-11-e0edbbfb-cfc5-4011-868b-2ce77ac7c70e)

### Windows Terminal

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

#### Configuration

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

### Ubuntu 24.04 LTS on Windows Subsystem for Linux (WSL2)

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

#### About Distros

See `wsl --list --online` for more info.

#### About Hardware Configuration

You will need to enable virtualization support in your system BIOS if it is not
already enabled.  On Intel this is called VT-d and an AMD it is called AMD-V.
See your laptop/motherboard manufacturer's documentation.

Further reading:

- [Set up a WSL development environment](https://learn.microsoft.com/en-us/windows/wsl/setup/environment)
- [Run multiple distros/instances](https://learn.microsoft.com/en-us/windows/wsl/install#ways-to-run-multiple-linux-distributions-with-wsl)

### USBIPD Installation using the WSL USB GUI

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

### VSCode

> Not required for developing within WSL2 but it's very well supported.

```powershell
winget install -e --id Microsoft.VisualStudioCode
```

Or installer: https://code.visualstudio.com/download#

## Setup WSL2 on First Boot

### Create a User

- Open WSL2 by right-clicking on Terminal and selecting Ubuntu 24.04 LTS
  - Or, in an already open Terminal, from the top bar, dropdown V to display
    shells
  - You can set defaults from the Terminal Settings
- Choose a short and normal username, such as your first name, all lowercase
- Choose a short and easy password 
  > OK to use `admin` or similar - if a bad actor has gotten this far they
  > already have access to your entire Windows OS!

### Get Common Dependencies

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

### Setup Git

- Set your global git config (same as in Windows):
  ```
  git config --global user.name "John Doe"
  git config --global user.email johndoe@example.com
  git config --global init.defaultBranch main
  ```
- Create and add SSH keys to GitHub and BitBucket (as necessary):
  - [Github](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account?platform=linux)
  - [BitBucket](https://support.atlassian.com/bitbucket-cloud/docs/set-up-personal-ssh-keys-on-linux/)

### Install Segger J-Link Software

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

### Test VSCode

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

### Google Chrome

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
