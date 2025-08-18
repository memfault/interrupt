---
title: Practical Zephyr - West workspaces (Part 6)
description: Article series "Practical Zephyr" the sixth part about West workspaces.
author: lampacher
tags: [zephyr, west, practical-zephyr-series, nordic]
---

<!-- excerpt start -->

In the previous articles, we used _freestanding_ applications and relied on a global Zephyr installation. In this article, we'll see how we can use _West_ to resolve global dependencies by using _workspace_ applications. We first explore _West_ without even including _Zephyr_ and then recreate the modified **Blinky** application from the previous article in a _West workspace_.

<!-- excerpt end -->

After this article, you should know _West_ not as a meta-tool for `build` and `flash` commands, but as a powerful dependency manager for your Zephyr applications. Hopefully - after reading this article - you will no longer create _freestanding_ Zephyr applications but will use _West workspaces_ only.

👉 Find the other parts of the *Practical Zephyr* series [here](/tags#practical-zephyr-series).

> 🎬
> [Listen to Martin on Interrupt Live](https://www.youtube.com/watch?v=ls_Y45WsTiA?feature=share)
> talk about the content and motivations behind writing this series.

{% include newsletter.html %}

{% include toc.html %}

## Prerequisites

This article is part of the _Practical Zephyr_ article series. In case you haven't read the previous articles, please go ahead and have a look. This article requires that you're able to build and flash a Zephyr application to the board of your choice and that you're familiar with _Devicetree_.

We'll be using the [development kit for the nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) and the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html), but you can follow along with any target - real or virtual.

> **Note:** This article uses two _manifest repositories_ that are available on GitHub:
> - [practical-zephyr-t2-empty-ws](https://github.com/lmapii/practical-zephyr-t2-empty-ws) contains an empty _manifest_ repository for creating a dummy _West_ workspace that we'll use to explore _West_ without Zephyr.
> - [practical-zephyr-manifest-repository](https://github.com/lmapii/practical-zephyr-manifest-repository) contains the _manifest repository_ of the Zephyr application that we're building in this article.

> **Note:** While writing this article series, I had to switch from STM's Nucleo-C031C6 to the Nucleo-F411RE since I broke my C031C6 development kit. Apologies for the subtle switch - but you should be able to follow along with any board.



## Creating a workspace from scratch

Let's start from scratch, without any tools installed. Ok, that might be a bit much - let's say you have at least [`git`](https://git-scm.com/), [`python`](https://www.python.org/) and [`pipenv`](https://pipenv.pypa.io) installed. You can, of course, use `pyenv` or install directly using `pip`, whatever works best for you!

```bash
$ mkdir workspace
$ cd workspace
```


### Installing west

We've been using [West](https://docs.zephyrproject.org/latest/develop/west/index.html) a lot now to build, debug and flash our project. It comes bundled with the [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk), so until now, we didn't have to install it at all. Without sourcing any paths, e.g., from a `setup.sh` script that we created at the very beginning of this article series, I don't have _West_ installed:

```bash
$ west --version
zsh: command not found: west
```

The first thing to even get started is therefore installing West, which is available as a `python` package. Instead of following the [official documentation](https://docs.zephyrproject.org/latest/develop/west/install.html) and installing _West_ globally, I'll be using `pipenv` to manage an environment for me:

```bash
$ pwd
$ path/to/workspace
$ touch Pipfile
```

In my `Pipfile`, I can simply add `west` as a `dev-package`, since I won't be needing it for any `python` application or extension:

```ini
[[source]]
url = "https://pypi.org/simple"
verify_ssl = true
name = "pypi"

[dev-packages]
autopep8 = "*"
pylint = "*"
west = "1.2"

[requires]
python_version = "3.11"
```

All that's left to do is to install the environment and activate the shell, and we have _West_ available:

```bash
workspace $ pip install pipenv
workspace $ pipenv install --dev
workspace $ pipenv shell
(workspace) workspace $ west --version
West version: v1.2.0
```

In the remainder of this article, I won't explicitly refer to the `python` environment or _West_ installation anymore. It is up to you to decide which approach to use, e.g., installing _West_ globally, using [`pipenv`](https://pipenv.pypa.io/), [`venv`](https://docs.python.org/3/library/venv.html), [`poetry`](https://python-poetry.org/), or maybe you're using one Zephyr's [docker images](https://github.com/zephyrproject-rtos/docker-image) - whatever works best for you!


### Adding a manifest

We now want to use _West_ to handle our dependencies: We really want to move on from using a global installation that may or will change at any time, and instead have all of our dependencies managed in our _workspace_. Zephyr evolves fast and it is therefore very important to have fixed versions of all modules and Zephyr's source code.

_West_ solves this by using a so-called [_manifest_ file](https://docs.zephyrproject.org/latest/develop/west/manifest.html), which is nothing else than a list of external dependencies - and then some. _West_ uses this manifest file to basically act like a package manager for your _C_ project, similar to what [`cargo`](https://doc.rust-lang.org/stable/cargo/) does with its `cargo.toml` files for [_Rust_](https://www.rust-lang.org/).

> **Note:** Please don't pin me down for comparing _West_ with _cargo_. I'm just trying to find some other description than "Swiss Army Knife".

> **Note:** The complete complete manifest repository used in this section is available on GitHub: [practical-zephyr-t2-empty-ws](https://github.com/lmapii/practical-zephyr-t2-empty-ws).

_West_ can also be used if you're _not_ planning to create a Zephyr project. In fact, let's start with a manifest file that doesn't include Zephyr at all. For that, we create the file `app/west.yml` in our `workspace` folder.

```bash
workspace $ mkdir app
workspace $ tree
.
├── app
│   └── west.yml
├── Pipfile
└── Pipfile.lock
```

The minimal content of our _manifest_ file is the following:

`app/west.yml`

```yaml
manifest:
  # lowest version of the manifest file schema that can parse this file’s data
  version: 0.8
```

Zephyr's [official documentation on West basics](https://docs.zephyrproject.org/latest/develop/west/basics.html) calls the `workspace` folder the "_topdir_" and our `app` folder the _manifest **repository**_: In an idiomatic workspace, the folder containing the manifest file is a direct sibling of the "topdir" or workspace root directory, and it is a `git` repository. The folder name `app` is just a personal preference.

> **Note:** I'm using the term "idiomatic" since there are multiple ways of setting up _West workspaces_. Placing your manifest file into a folder that is a direct sibling of the workspace is a _convention_. Nothing stops you from placing your manifest file in a different folder, e.g., deeper within your folder tree. This does, however, have some drawbacks since some of the _West_ commands, e.g., `west init`, are currently not very customizable, e.g., initializing a workspace using a repository and the `-m` argument won't work out-of-the-box.
>
> Feel free to experiment with your version of _West_. I'm sure there will be some changes in the future, but for now, we focus on the intended or "_idiomatic_" way of using _West_.

Let's initialize the repository for our `app` folder containing the manifest files:

```bash
workspace/app $ git init
Initialized empty Git repository in /path/to/workspace/app/.git/

workspace/app $ tree --dirsfirst -a -L 2 ../
../      # topdir
├── app  # manifest _repository_
│   ├── .git
│   └── west.yml  # manifest _file_
├── Pipfile
└── Pipfile.lock

workspace/app $ git remote add origin git@github.com:lmapii/practical-zephyr-t2-empty-ws.git
workspace/app $ git add --all
workspace/app $ git commit -m "initial commit"
workspace/app $ git push -u origin main
```

### Initializing the workspace

Having everything under version control we can relax and start exploring. First, we finally _initialize_ the _West_ workspace using [`west init`](https://docs.zephyrproject.org/latest/develop/west/basics.html#west-init-and-west-update). There are two ways to initialize a _West_ workspace:

- **Locally**, using the `-l` or `--local` flag: This assumes that your manifest repository already exists in your filesystem, e.g., you already used `git clone` to populate it in the _topdir_.
- **Remotely**, by specifying the URL to the manifest repository using the argument `-m`. With this argument, _West_ clones the manifest repository into the _topdir_ for you before initializing the workspace. We'll see how this works in a [later section](#creating-a-workspace-from-a-repository).

There is no difference between the two methods except that _West_ also clones the repository when using the `-m` argument to pass the manifest repository URL. Since our manifest repository is initialized already in the `app` folder, we can initialize the workspace using this local manifest repository:

```bash
workspace/app $ west init --local .
=== Initializing from existing manifest repository app
--- Creating /path/to/workspace/.west and local configuration file
=== Initialized. Now run "west update" inside /path/to/workspace.

workspace/app $ tree --dirsfirst -a -L 2 ../
../
├── .west
│   └── config
├── app
│   ├── .git
│   └── west.yml
├── Pipfile
└── Pipfile.lock
```

When initializing a workspace, _West_ creates a `.west` directory in the _topdir_, which in turn contains a configuration file.

```bash
workspace/app $ cat ../.west/config
```

```ini
[manifest]
path = app
file = west.yml
```

The location of the `.west` folder "marks" the _topdir_ and thus the _West_ workspace root directory. Within this file, we can see that _West_ stores the location and name of the manifest file. Modifying this file - or any file within the `.west` folder -  is not recommended, since some of _West's_ commands might no longer work as expected.

_"west/config"_ sounds familiar, doesn't it? If you've been following this article series, you might remember that we already encountered the `west config` command in the very [first article]({% post_url 2024-01-10-practical_zephyr_basics %}): We used the command "`west config build.board nrf52840dk_nrf52840`" to configure the _board_ for our build so that we didn't have to pass it as an argument to `west build` anymore. This does sound suspiciously similar to _"west/config"_!

Let's try and see how `west config` affects our workspace and `.west/config`:

```bash
workspace/app $ west config build.board nrf52840dk_nrf52840
workspace/app $ cat ../.west/config
```

```ini
[manifest]
path = app
file = west.yml

[build]
board = nrf52840dk_nrf52840
```

Having initialized a _West workspace_, we can now see that `west config` by default uses this _local_ configuration file to store its configuration options. _West_ also supports storing configuration options globally or even system-wide. Have a look at the  [official documentation](https://docs.zephyrproject.org/latest/develop/west/config.html) in case you need to know more.

Let's get rid of the configuration option and finally run [`west update`](https://docs.zephyrproject.org/latest/develop/west/basics.html#west-init-and-west-update) as suggested by the output we got in our call to `west init`:

```bash
workspace/app $ west config -d build.board
workspace/app $ cd ../
workspace $ west update
workspace $ # nothing happens
```

Huh, that was disappointing. `west update` didn't do anything - feel free to check your files, nothing changed! The reason for that is quite simple: With `west init` we already took care of initializing the workspace. Since we don't have any external dependencies to manage, we have no work for `west update`.


### Adding projects

In your manifest file, you can use the `manifest.projects` key to define all Git repositories that you want to have in your workspace. Let's say that we're planning to use a more modern [doxygen](https://doxygen.nl/) documentation for our project and therefore want to add [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css) to our workspace. We can add this external dependency as an entry in the `manifest.projects` key of our manifest file:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  projects:
    - name: doxygen-awesome
      url: https://github.com/jothepro/doxygen-awesome-css
      # you can also use SSH instead of HTTPS:
      # url: git@github.com:jothepro/doxygen-awesome-css.git
      revision: main
      path: deps/utilities/doxygen-awesome-css
```

Every _project_ at least has a **unique** name. Typically, you'll also specify the `url` of the repository, but there are several options, e.g., you could use the [`remotes` key](https://docs.zephyrproject.org/latest/develop/west/manifest.html#remotes) to specify a list of URLs and add specify the `remote` that is used for your project. You can find examples and a detailed explanation of all available options in the [official documentation for the `projects` key](https://docs.zephyrproject.org/latest/develop/west/manifest.html#projects).

With the `revision` key you can tell _West_ to check out either a specific _branch_, _tag_, or commit hash. Without specifying the `revision`, at the time of writing _West_ defaults to the `master` branch. Since [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css) uses `main` as a development branch, we have to specify it explicitly.

The `path` key is also an optional key that tells _West_ the _relative_ path to the _topdir_ that it should use when cloning the project. Without specifying the `path`, _West_ uses the project's `name` as the path. Notice that you're **not** allowed to specify a path that is outside of the _topdir_ and thus _West_ workspace.

With that project added, we finally have some work for `west update`:

```bash
workspace $ west update
=== updating doxygen-awesome (deps/utilities/doxygen-awesome-css):
--- doxygen-awesome: initializing
Initialized empty Git repository in /path/to/workspace/deps/utilities/doxygen-awesome-css/.git/
--- doxygen-awesome: fetching, need revision main
...
HEAD is now at 8cea9a0 security: fix link vulnerable to reverse tabnabbing (#127)
```

Great! Now _West_ populated our dependencies and we have [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css) available in the specified `path`:

```bash
workspace $ tree --dirsfirst -a -L 3 --filelimit 8
.
├── .west
│   └── config
├── app
│   ├── .git  [10 entries exceeds filelimit, not opening dir]
│   └── west.yml
├── deps
│   └── utilities
│       └── doxygen-awesome-css
├── Pipfile
└── Pipfile.lock
```

The folder name `deps` is again just a personal choice. I have borrowed this convention from Mike Szczys' talk ["Manifests: Project Sanity in the Ever-Changing Zephyr World"](https://www.youtube.com/watch?v=PVhu5rg_SGY) from the 2022 Zephyr Developer Summit, and we'll see why it makes sense to collect your dependencies in just a bit.

In our manifest file, we specified that we want to use the `main` branch for the `doxygen-awesome` project. This kind of defeats the purpose of using a manifest to create a stable workspace since we'll always be checking out the latest version of the repository when using `west update`. Instead, you'll typically either specify a _tag_ or even a specific commit in the revision.

At the time of writing, the tag `v2.2.1` was the latest release available for `doxygen-awesome`, pointing at the commit with the shortened hash `df83fbf`. Let's update the manifest to use the latest tag:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  projects:
    - name: doxygen-awesome
      url: https://github.com/jothepro/doxygen-awesome-css
      revision: v2.2.1
      path: deps/utilities/doxygen-awesome-css
```

Running `west update` changes our dependency to the specified revision:

```bash
workspace $  west update
=== updating doxygen-awesome (deps/utilities/doxygen-awesome-css):
Warning: you are leaving 2 commits behind, not connected to
any of your branches:

  8cea9a0 security: fix link vulnerable to reverse tabnabbing (#127)
  00a52f6 Makefile: install -tabs.js as well (#122)

If you want to keep them by creating a new branch, this may be a good time
to do so with:

 git branch <new-branch-name> 8cea9a0

HEAD is now at df83fbf fix rendering error in example class
```

Looks like the `main` branch was already two commits ahead of the specified revision!

_West_ thus takes care of all the projects that we're using. We can, in fact, delete the entire `deps` repository and run `west update` again, and it'll simply put it back into its specified state. This is also the reason why it makes sense to group external dependencies, e.g., into this `deps` folder (full credits to Mike Szczys and [Golioth](https://golioth.io/)): Whenever you're not working on this project anymore, you can simply delete the `deps` and `.west` folders to save disk space (as soon as we'll add Zephyr this becomes a significant amount of disk space). Once you pick it up again, simply run `west init` and `west update` and you're ready to go.

> **Note:** More complex applications include lots of _projects_ in their manifest hierarchy. Having a single dependency folder `deps` can also help in your CI builds: E.g., with GitHub actions you can _cache_ your dependency folder and thereby significantly reduce the time required to run `west update`. The GitHub action used by the [practical-zephyr-manifest-repository](https://github.com/lmapii/practical-zephyr-manifest-repository/blob/main/.github/workflows/ci.yml) that we'll create later in this article uses such a cache for demonstration purposes. Noah Pendleton wrote an [excellent article]({% post_url 2024-01-18-ncs-github-actions %}) about GitHub actions for NCS applications.

Whatever project structure you're using, however, is entirely up to you and always subject to personal preference.



## Creating a workspace from a repository

In Zephyr's official documentation for _West_, the first example call to `west init` uses the `-m` argument to specify the manifest repository's URL. In the previous section, we've seen how we can _create_ such a manifest repository and how to use `west init --local` to initialize the workspace _locally_.

Instead of initializing a workspace _locally_, let's use the manifest file and repository that we've created in the previous section and initialize the workspace from scratch using its remote URL. Let's fire up a new terminal (I'll be installing _West_ again in a virtual environment) and get started right away:

```bash
$ mkdir workspace-m
$ cd workspace-m

workspace-m $ west init -m git@github.com:lmapii/practical-zephyr-t2-empty-ws.git
=== Initializing in /path/to/workspace-m
--- Cloning manifest repository from git@github.com:lmapii/practical-zephyr-t2-empty-ws.git
Cloning into '/path/to/workspace-m/.west/manifest-tmp'...
...
--- setting manifest.path to practical-zephyr-t2-empty-ws.git
=== Initialized. Now run "west update" inside /path/to/workspace-m.

workspace-m $ tree --charset=utf-8 --dirsfirst -a -L 2
.
├── .west
│   └── config
├── practical-zephyr-t2-empty-ws.git
│   ├── .git
│   └── west.yml
├── Pipfile
└── Pipfile.lock
```

Notice that we no longer specify the current directory in the call to `west init` using "`.`". In fact, the directory - optionally passed as the last argument to `west init` - is interpreted **differently** by _West_ when using the `--local` flag or the `-m` arguments:

- With `--local`, the _directory_ specifies the path to the local manifest repository.
- Without the `--local` flag, the _directory_ refers to the _topdir_ and thus the folder in which to create the workspace (defaulting to the current working directory in this case).

Awkward, but this approach is probably used due to legacy reasons. If no `--local` flag is used and no repository is specified using `-m`, _West_ defaults to using the Zephyr repository.

The file tree, however, isn't exactly what we wanted ... `west init` created a folder `practical-zephyr-t2-empty-ws.git` instead of the folder called `app` that we've had before. Well, there's no way for _West_ to know that we want the manifest repository to use the folder name `app`, so it uses the repository's name instead. How can we change that?


### Updating the manifest repository path

The manifest file uses the key `manifest.self` for configuring the manifest repository itself, meaning that all settings in the `manifest.self` key are only applied to the manifest repository. The key `manifest.self.path` can be used to specify the path that _West_ uses when cloning the manifest repository, relative to the _West_ workspace _topdir_.

Let's update the `west.yml` file in the folder that `west init` cloned for us as follows:

`practical-zephyr-t2-empty-ws.git/west.yml`

```yaml
manifest:
  version: 0.8

  # Entries in the `self` key are only applied to the _manifest repository_
  self:
    path: app

  projects:
    - name: doxygen-awesome
      url: https://github.com/jothepro/doxygen-awesome-css
      revision: main
      path: deps/utilities/doxygen-awesome-css
```

Don't forget to commit and push the change: We're passing a URL to `west init` so _West_ obviously won't pick up local changes during the workspace initialization.

To reinitialize the workspace, we **must** remove the `.west` folder, otherwise `west init` throws an error telling us that the workspace is already initialized:

```bash
workspace-m $ west init -m git@github.com:lmapii/practical-zephyr-t2-empty-ws.git
FATAL ERROR: already initialized in /path/to/workspace-m, aborting.
  Hint: if you do not want a workspace there,
  remove this directory and re-run this command:

  /path/to/workspace-m/.west
```

Let's follow the instructions and also remove `practical-zephyr-t2-empty-ws.git` before reinitializing the workspace:

```bash
workspace-m $ rm -rf .west practical-zephyr-t2-empty-ws.git
workspace-m $ west init -m git@github.com:lmapii/practical-zephyr-t2-empty-ws.git
=== Initializing in /path/to/workspace-m
...
--- setting manifest.path to app
=== Initialized. Now run "west update" inside /path/to/workspace-m.

workspace-m $ tree --charset=utf-8 --dirsfirst -a -L 2
.
├── .west
│   └── config
├── app
│   ├── .git
│   └── west.yml
├── Pipfile
└── Pipfile.lock
```

Now we can run `west update` and we end up with the same workspace as we got when initializing it locally:

```bash
workspace-m $ west update
=== updating doxygen-awesome (deps/utilities/doxygen-awesome-css):
--- doxygen-awesome: initializing
Initialized empty Git repository in /path/to/workspace-m/deps/utilities/doxygen-awesome-css/.git/
--- doxygen-awesome: fetching, need revision v2.2.1
...
HEAD is now at df83fbf fix rendering error in example class

workspace-m $ tree --dirsfirst -a -L 3 --filelimit 8
.
├── .west
│   └── config
├── app
│   ├── .git  [10 entries exceeds filelimit, not opening dir]
│   └── west.yml
├── deps
│   └── utilities
│       └── doxygen-awesome-css
├── Pipfile
└── Pipfile.lock
```


### Locally vs. remotely initialized workspaces

What is the difference between a _West workspace_ that has been initialized locally using the `--local` flag, or remotely by passing the URL of the manifest repository? As mentioned already, thankfully, the short answer is _none_.

The only difference is that for remotely initialized workspaces _West_ clones the repository for you and thus you essentially don't need to use `git clone` to obtain the manifest repo used to setup the workspace.

We can also see this in the configuration file `.west/config`:

```bash
workspace-m $ cat .west/config
```

```ini
[manifest]
path = app
file = west.yml
```

Just like for the locally initialized repository, the `[manifest]` section points to the manifest file in the `app` folder. Running `west update` therefore only checks the contents of the local manifest file. It won't try to pull new changes in the manifest repository and it also won't attempt to read the file from the remote.

If there were any changes to the manifest file in the repository, you'll have to `git pull` them in your manifest repository - which is a good thing. In fact, `west update` will never attempt to modify the manifest repository and also states this in the `--help` information for the `update` command:

> "This command does not alter the manifest repository's contents."



## Zephyr with West

Having covered the _West_ basics, let's get back to creating _Zephyr_ applications and put this knowledge into practice.

In this section, we recreate the modified **Blinky** application that we've seen in the previous article, where we failed to compile the application for the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html) since we relied upon the sources from a globally installed [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk) - which doesn't include [STM32 HAL](https://github.com/zephyrproject-rtos/hal_stm32).

> **Note:** You can find the complete _manifest repositories_ on GitHub: [practical-zephyr-manifest-repository](https://github.com/lmapii/practical-zephyr-manifest-repository).


### Application skeleton

Let's zoom through the usual warm-up to create our application. Go ahead and create the following file tree:

```bash
$ tree --dirsfirst -a -L 3
.
└── app
    ├── boards
    │   └── nrf52840dk_nrf52840.overlay
    ├── src
    │   └── main.c
    ├── CMakeLists.txt
    └── prj.conf
```

As an application, we're using a modified version of the *Blinky* sample, where we select the LED node via a newly created `/chosen` node, and output "Tick" and "Tock" each time the LED is turned on or off:

`app/main.c`

```c
/** \file main.c */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 1000U
#define LED_NODE      DT_CHOSEN(app_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void main(void)
{
    int err   = 0;
    bool tick = true;

    if (!gpio_is_ready_dt(&led))
    {
        printk("Error: LED pin is not available.\n");
        return;
    }

    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err != 0)
    {
        printk("Error %d: failed to configure LED pin.\n", err);
        return;
    }

    while (1)
    {
        (void) gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);

        if (tick != false) { printk("Tick\n"); }
        else { printk("Tock\n"); }
        tick = !tick;
    }
}
```

I'll again build the application for my [nRF52840 Development Kit from Nordic](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) and will only later switch to the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html), so, for now, all I need is the matching `nrf52840dk_nrf52840.overlay` that specifies the chosen LED node:

`app/boards/nrf52840dk_nrf52840.overlay`

```dts
/ {
    chosen {
        app-led = &led0;
    };
};
```

The `prj.conf` remains empty, and the `CMakeLists.txt` only includes the same old boilerplate to create a Zephyr application with a single `main.c` source file:

`app/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(
    EmptyApp
    VERSION 0.1
    DESCRIPTION "Modified Blinky application."
    LANGUAGES C
)

target_sources(
    app
    PRIVATE
    src/main.c
)
```

Trying to run `west build` shows us that we're not just missing the Zephyr source code, but we're also still lacking the Zephyr-specific [extension commands](https://docs.zephyrproject.org/latest/develop/west/zephyr-cmds.html), e.g., `build`:

```bash
workspace-m $ west build
usage: west [-h] [-z ZEPHYR_BASE] [-v] [-V] <command> ...
west: unknown command "build"; workspace /path/to/workspace-m does not define this extension command -- try "west help"
```

As we've seen in previous articles, e.g., `build` and `flash` are Zephyr-specific [extension commands](https://docs.zephyrproject.org/latest/develop/west/zephyr-cmds.html) that are not included by a plain _West_ installation. We'll resolve this in the following sections.


### Setting up the manifest repository and file

For our workspace structure, we're using what's known as a ["T2 star topology"](https://docs.zephyrproject.org/latest/develop/west/workspaces.html#topologies-supported): In such a topology, the _manifest repository_ does not only contain the _manifest file(s)_ but also contains all application files.

We have all of our application code ready in the `app` folder, so let's go ahead, create an empty `app/west.yml` and make the `app` folder our manifest repository:

```bash
$ cd app
$ git init
$ touch west.yml
$ git add --all
$ cd ../

$ tree --dirsfirst -a -L 3
.
└── app  # manifest repository
    ├── .git
    ├── boards
    │   └── nrf52840dk_nrf52840.overlay
    ├── src
    │   └── main.c
    ├── CMakeLists.txt
    ├── prj.conf
    └── west.yml  # manifest file
```

With this, we can initialize our workspace as follows:

```bash
$ west init --local app
=== Initializing from existing manifest repository app
--- Creating /path/to/workspace/.west and local configuration file
=== Initialized. Now run "west update" inside /path/to/workspace.
```

Before running `west update`, let's add Zephyr as a project in `manifest.projects` just like what we've done for the [Doxygen-Awesome](https://github.com/jothepro/doxygen-awesome-css) dependency before:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  self:
    path: app

  projects:
    - name: zephyr
      revision: v3.4.0
      url: https://github.com/zephyrproject-rtos/zephyr
      path: deps/zephyr
```

Running `west update` clones the Zephyr repository into `deps/zephyr` and checks out the commit with the tag `v3.4.0`:

```bash
$ west update
=== updating zephyr (deps/zephyr):
--- zephyr: initializing
Initialized empty Git repository in /path/to/workspace/deps/zephyr/.git/
--- zephyr: fetching, need revision v3.4.0
...
HEAD is now at 356c8cbe63 release: Zephyr 3.4.0 release
```

Now, we have a complete "T2 star topology" workspace, where the Zephyr dependency is placed in a separate `deps` folder in the _topdir_ (again following the convention presented by Mike Szczys in his talk ["Manifests: Project Sanity in the Ever-Changing Zephyr World"](https://www.youtube.com/watch?v=PVhu5rg_SGY)):


```bash
tree --dirsfirst -a -L 3
.  # topdir
├── .west
│   └── config
├── app  # manifest repository
│   ├── .git
│   ├── boards
│   │   └── nrf52840dk_nrf52840.overlay
│   ├── src
│   │   └── main.c
│   ├── CMakeLists.txt
│   ├── prj.conf
│   └── west.yml  # manifest file
└── deps
    └── zephyr
```

Since we've used this application already in the previous article, we're pretty sure that it should build. However, building still fails since we didn't tell _West_ about Zephyr's extension commands yet:

```bash
$ west build --board nrf52840dk_nrf52840 app
usage: west [-h] [-z ZEPHYR_BASE] [-v] [-V] <command> ...
west: unknown command "build"; workspace /path/to/workspace does not define this extension command -- try "west help"
```

We can fix this by adding the `west-commands` key to the `zephyr` project: Zephyr's _West_ extensions are provided by the file `scripts/west-commands.yml` in Zephyr's repository. Using the key `west-commands`, we can provide a relative path to _West_ extension commands within the project:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  self:
    path: app

  projects:
    - name: zephyr
      revision: v3.4.0
      url: https://github.com/zephyrproject-rtos/zephyr
      path: deps/zephyr
      # explicitly add Zephyr-specific West extensions
      west-commands: scripts/west-commands.yml
```

With the `west-commands` in place we can re-run `west update` to try and build our application. Several things can go wrong at this point. Best case, you'll see the error message below. Otherwise, the build might fail, e.g., because you don't have [_CMake_](https://cmake.org/) installed or because the _Zephyr SDK_ doesn't match the target:

```bash
west build --board nrf52840dk_nrf52840 app
-- west build: generating a build system
Loading Zephyr default modules (Zephyr base).
...
-- ZEPHYR_TOOLCHAIN_VARIANT not set, trying to locate Zephyr SDK
CMake Error at /path/to/workspace/deps/zephyr/cmake/modules/FindZephyr-sdk.cmake:108 (find_package):
  Could not find a package configuration file provided by "Zephyr-sdk"
  (requested version 0.15) with any of the following names:

    Zephyr-sdkConfig.cmake
    zephyr-sdk-config.cmake

  Add the installation prefix of "Zephyr-sdk" to CMAKE_PREFIX_PATH or set
  "Zephyr-sdk_DIR" to a directory containing one of the above files.  If
  "Zephyr-sdk" provides a separate development package or SDK, be sure it has
  been installed.
```

The error message is quite clear: Zephyr builds require that a toolchain is either installed globally or specified using the `ZEPHYR_TOOLCHAIN_VARIANT` environment variable. I didn't do either.

> **Note:** The `west-commands` is only needed since we've not seen the `import` key yet. I've added it explicitly so that you know about this key and how the West extension commands are handled in a manifest file, but you typically don't need to specify this in your `west.yml`.

The term ["Zephyr SDK"](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html) refers to the toolchains that must be provided for each of Zephyr’s supported architectures. With our manifest, we only add Zephyr's _sources_ to our workspace. The required tools, however, e.g., the GCC compiler for ARM Cortex-M, are **not** included.


### Adding a toolchain

Toolchain management is always a highly opinionated topic, so I'll try to keep that discussion out of this article. The point is this: West workspaces do **not** include the required _toolchain_ needed for building your application. This is something that you need to manage yourself, using whatever approach you feel comfortable with:

- Whether you're installing the toolchain globally,
- or decide to use [NixOS](https://nixos.org/explore),
- or maybe use [Docker](https://www.docker.com/),
- or something like [Nordic's toolchain manager](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation/assistant.html#install-toolchain-manager),

... or any other approach, is entirely up to you. What you need for building Zephyr applications are:

- Host dependencies such as `python`, `cmake`, `ninja`, etc., as explained in Zephyr's [Getting Started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies)
- An installation of the _Zephyr SDK_, containing the architecture specific toolchain. Zephyr's official documentation includes a dedicated section on the [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html) and its installation instructions. The _Zephyr SDK_ does **not** contain Zephyr's _sources_!

> **Note:** Yes, the inconsistent use of the term "SDK" is quite annoying. While Zephyr uses _SDK_ exclusively for referring to the toolchain, I'd claim that an SDK typically also includes source code.

The host tools are typically in your `$PATH` - at least for the executing terminal. For pointing Zephyr's build process to your installed SDK you can use the two environment variables [`ZEPHYR_TOOLCHAIN_VARIANT`](https://docs.zephyrproject.org/latest/develop/env_vars.html#envvar-ZEPHYR_TOOLCHAIN_VARIANT) and [`ZEPHYR_SDK_INSTALL_DIR`](https://docs.zephyrproject.org/latest/develop/env_vars.html#envvar-ZEPHYR_SDK_INSTALL_DIR).

In the first article of this series, we installed the correct SDK for the [nRF52840 Development Kit](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) using [Nordic's toolchain manager](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation/assistant.html#install-toolchain-manager). This installation contains not only the Zephyr SDK but also the host dependencies that I need for building Zephyr applications for nRF devices.

Since both, the STM32 and the nRF52840, are ARM MCUs, the installed Zephyr SDK already also contains the toolchain for building for the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html). All I need to do is provide the required environment variables.

For that, I could, e.g., use a _shell_ script similar to what we've seen in the first article of this series:

`app/setup-sdk-nrf.sh`

```sh
#!/bin/sh

ncs_install_dir="${ncs_install_dir:-/opt/nordic/ncs}"
ncs_bin_version="${ncs_bin_version:-4ef6631da0}"

paths=(
    $ncs_install_dir/toolchains/$ncs_bin_version/bin
    $ncs_install_dir/toolchains/$ncs_bin_version/opt/nanopb/generator-bin
)

# required if dependencies are not installed, see also
# https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies
# e.g., "Ninja"
for entry in ${paths[@]}; do
    export PATH=$PATH:$entry
done

# only export the paths to the SDK, no longer export the path to the zephyr installation.
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=$ncs_install_dir/toolchains/$ncs_bin_version/opt/zephyr-sdk
```

> **Notice:** Zephyr provides [environment scripts](https://docs.zephyrproject.org/latest/develop/env_vars.html#zephyr-environment-scripts) including a `zephyr-env.sh`, which you can source in case you're using [Zephyr's official SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html).

In this script, I'm extending my `$PATH` for the binaries that come with Nordic's toolchain installation. This includes `cmake`, `ninja`, and all other host tools used by Zephyr. In fact, this installation even contains `git` and `python`, but by appending to `$PATH`, my local installations have precedence over Nordic's binaries.

Then, I'm setting the `ZEPHYR_TOOLCHAIN_VARIANT` to `zephyr` and point `ZEPHYR_SDK_INSTALL_DIR` to the full path of the toolchain installation. Since I'm still using my own virtual environment, I need to install all Python packages required by Zephyr. Instead of cloning Zephyr as documented in the [getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies), I can use the locally populated Zephyr installation:

```bash
$ pip install -r deps/zephyr/scripts/requirements.txt
```

> **Note:** Using the above shell script I could actually scrap my virtual python environment, since all dependencies and packages are installed in Nordic's installation.

Having all tools installed, the build should now pass, or shouldn't it? Worth a try!

```bash
$ source app/setup-sdk-nrf.sh
$ west build --board nrf52840dk_nrf52840 app
-- west build: generating a build system
Loading Zephyr default modules (Zephyr base (cached)).
-- Application: /path/to/workspace/app
...
-- Found host-tools: zephyr 0.16.0 (/opt/nordic/ncs/toolchains/4ef6631da0/opt/zephyr-sdk)
-- Found toolchain: zephyr 0.16.0 (/opt/nordic/ncs/toolchains/4ef6631da0/opt/zephyr-sdk)
...
-- Including generated dts.cmake file: /path/to/workspace/build/zephyr/dts.cmake

warning: USE_SEGGER_RTT (defined at modules/segger/Kconfig:12) was assigned the value 'y' but got
the value 'n'. Check these unsatisfied dependencies: HAS_SEGGER_RTT (=n), 0 (=n). See
http://docs.zephyrproject.org/latest/kconfig.html#CONFIG_USE_SEGGER_RTT and/or look up
USE_SEGGER_RTT in the menuconfig/guiconfig interface. The Application Development Primer, Setting
Configuration Values, and Kconfig - Tips and Best Practices sections of the manual might be helpful
too.
...
error: Aborting due to Kconfig warnings

CMake Error at /path/to/workspace/deps/zephyr/cmake/modules/kconfig.cmake:343 (message):
  command failed with return code: 1
Call Stack (most recent call first):
  /path/to/workspace/deps/zephyr/cmake/modules/zephyr_default.cmake:115 (include)
  /path/to/workspace/deps/zephyr/share/zephyr-package/cmake/ZephyrConfig.cmake:66 (include)
  /path/to/workspace/deps/zephyr/share/zephyr-package/cmake/ZephyrConfig.cmake:97 (include_boilerplate)
  CMakeLists.txt:2 (find_package)


-- Configuring incomplete, errors occurred!
FATAL ERROR: command exited with status 1: /opt/homebrew/bin/cmake -DWEST_PYTHON=/path/to/workspace/.venv/bin/python -B/path/to/workspace/build -GNinja -S/path/to/workspace/app
```

The build process was now able to correctly determine the `host-tools` and `toolchains`, but the build still failed with an error message that is quite hard to comprehend. Something seems to be missing!


### Manifest imports

The build fails since we're only adding the _Zephyr_ repository as a dependency. This is not enough: The Zephyr repository has its own dependencies, which it lists as _projects_ in its own `west.yml` file. We can _recursively import_ the required projects from Zephyr using the `import` key as follows:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  self:
    path: app

  projects:
    - name: zephyr
      revision: v3.4.0
      url: https://github.com/zephyrproject-rtos/zephyr
      # the path is no longer needed since we're now using `path-prefix`
      # path: deps/zephyr
      # explicitly adding the Zephyr-specific West extensions is also no longer needed since
      # they are added accordingly with the `import` key.
      # west-commands: scripts/west-commands.yml
      # recursively import Zephyr dependencies
      import:
        path-prefix: deps
        # the `file` key is not strictly needed since `west.yml` is the default value.
        file: west.yml
        name-allowlist:
          - cmsis
          - hal_nordic
```

We got rid of the `path` key, since `import.path-prefix` allows us to define a common prefix for all projects. Using the `import.file` key, we're telling _West_ to look for a `west.yml` file in Zephyr's repository and also consider the projects and West commands listed there. Notice that by default West looks for a `west.yml` file when using `import` and therefore it is not necessary to provide the `import.file` entry.

Instead of adding _all_ of Zephyr's dependencies, we pick the ones we need _by their name_ using the `import.name-allowlist` key.

> **Notice:** Without `name-allowlist` we'd instruct _West_ to clone **all** dependencies, recursively. If you have a quick look at Zephyr's manifest file [`west.yml`](https://github.com/zephyrproject-rtos/zephyr/blob/main/west.yml), you'll see that it has _a lot_ of dependencies. Running `west update` without limiting the dependencies may take several minutes and lots of disk space!

With this, _West_ **recursively** imports all allowed dependencies from the `zephyr` project. The details are explained nicely in the [official documentation on "Manifest Imports"](https://docs.zephyrproject.org/latest/develop/west/manifest.html#west-manifest-import).

Running `west update`, the new dependencies are placed in the `deps/modules` folder: We specified the `deps` prefix, whereas the `module` folder comes from Zephyr's own `west.yml` file:

```bash
$ tree --dirsfirst -a -L 5
.
├── .west
│   └── config
├── app
│   ├── .git
│   ├── boards
│   │   └── nrf52840dk_nrf52840.overlay
│   ├── src
│   │   └── main.c
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── setup-sdk-nrf.sh
│   └── west.yml
├── build  [13 entries exceeds filelimit, not opening dir]
└── deps
    ├── modules
    │   └── hal
    │       ├── cmsis
    │       └── nordic
    └── zephyr  [43 entries exceeds filelimit, not opening dir]
```

Now, our `west build` indeed succeeds:


```bash
$ west build --board nrf52840dk_nrf52840 app
...

[163/163] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       24852 B         1 MB      2.37%
             RAM:        4416 B       256 KB      1.68%
        IDT_LIST:          0 GB         2 KB      0.00%
```

> **Notice:** In addition to the `name-allowlist` you can also instruct _West_ to use shallow clones instead of a complete `git clone` for all its projects. E.g., use `west update -o=--depth=1 -n` to create shallow clones. Have a look at the help output of `west update --help`!


## Switching boards

With our workspace set up and our knowledge about Zephyr's imports, we're ready to add support for the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html). The first thing we need is an overlay file to specify the `/chosen` LED node:

`app/boards/nucleo_f411re.overlay`

```dts
/ {
    chosen {
        app-led = &green_led_2;
    };
};
```

Knowing that we're still missing the STM32 HAL, we can look it up in Zephyr's `west.yml` file:

```bash
$ grep stm32 deps/zephyr/west.yml -A 2 --ignore-case
    - name: hal_stm32
      revision: c865374fc83d93416c0f380e6310368ff55d6ce2
      path: modules/hal/stm32
      groups:
        - hal
```

The STM32 HAL is part of the project `hal_stm32`, which we can now add to our allow-list in the manifest file:

`app/west.yml`

```yaml
manifest:
  version: 0.8

  self:
    path: app

  projects:
    - name: zephyr
      revision: v3.4.0
      url: https://github.com/zephyrproject-rtos/zephyr
      import:
        path-prefix: deps
        name-allowlist:
          - cmsis
          - hal_nordic
          - hal_stm32 # <-- added STM32 HAL
```

Running `west update`, _West_ populates the `stm32` HAL in the expected folder `deps/modules/hal/stm32`:

```bash
$ west update
$ tree --dirsfirst -a -L 5
.
├── .west
│   └── config
├── app
└── deps
    ├── modules
    │   └── hal
    │       ├── cmsis
    │       ├── nordic
    │       └── stm32
    └── zephyr  [43 entries exceeds filelimit, not opening dir]
```

Building the application for the STM32 development kit now succeeds as expected:

```bash
$ west build --board nucleo_f411re app --pristine
...

[159/159] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       18726 B       512 KB      3.57%
             RAM:        4352 B       128 KB      3.32%
        IDT_LIST:          0 GB         2 KB      0.00%
```

Connecting the development kit to my computer, I should now be able to program the application, right? Let's try:

```bash
$ west flash
-- west flash: rebuilding
ninja: no work to do.
-- west flash: using runner openocd
FATAL ERROR: required program openocd not found; install it or add its location to PATH
```

Oh, the STM32 development kit uses a different _runner_ than the nRF52840 development kit: Nordic's development kits use `jlink`, whereas this development kit uses `openocd`.

Thus, you don't only need the host tools and Zephyr SDK for your target, but also all other supporting tools, such as Nordic's command line tools or in this case `openocd`. On my computer, a simple `brew install open-ocd` fixes this problem, and I'm able to flash the application to the device.

In a more serious setup, your entire toolchain should be maintained in a much better way, but - as promised - I won't try to tell how you should do that - it's entirely up to you.



## Zephyr as dependency

When browsing allowed imports, we've seen that Zephyr has dependencies. What we haven't seen yet, is that Zephyr itself can be a dependency: Depending on your application, you might make use of some frameworks or SDKs that in turn depend on Zephyr and thus list it as _project_ in their own `west.yml` file.

E.g., looking at the [GitHub repository the _nRF Connect SDK_](https://github.com/nrfconnect/sdk-nrf/), we find that Nordic uses their own fork of the official Zephyr repository and has it listed as a _project_ in the SDK's [`west.yml`](https://github.com/nrfconnect/sdk-nrf/blob/e464540b74daca7f4a660f04fee0886b04365f70/west.yml#L62) file:

[`https://github.com/nrfconnect/sdk-nrf/blob/main/west.yml`](https://github.com/nrfconnect/sdk-nrf/blob/main/west.yml)

```yaml
manifest:
  projects:
    - name: zephyr
      repo-path: sdk-zephyr
      revision: 90a72daae2c8715d760d974a7d294aa2eb6b38c4
      import:
        name-allowlist:
          - TraceRecorderSource
          - canopennode
          - chre
          - ...
```

Another example is the [Golioth Zephyr SDK](https://docs.golioth.io/firmware/zephyr-device-sdk/), which lists the official Zephyr repository as a project in their [`west-zephyr.yml`](https://github.com/golioth/golioth-zephyr-sdk/blob/4f0fcea1370bfec84f6754caaca8b3f35bbc3573/west-zephyr.yml#L3) manifest:

[`https://github.com/golioth/golioth-zephyr-sdk/blob/main/west-zephyr.yml`](https://github.com/golioth/golioth-zephyr-sdk/blob/main/west-zephyr.yml)

```yaml
manifest:
  projects:
    - name: zephyr
      revision: v3.5.0
      url: https://github.com/zephyrproject-rtos/zephyr
      import: true
```

These dependencies are something that you need to handle yourself since [_West_ ignores projects that have already been defined in other files](https://docs.zephyrproject.org/latest/develop/west/manifest.html#manifest-imports). E.g., if you define _Zephyr_ as a project in your manifest file, and _after_ add [Golioth's Zephyr SDK](https://docs.golioth.io/firmware/zephyr-device-sdk/), then the _Zephyr_ project in the `west-zephyr.yml` file of the Golioth SDK is _ignored_.

It is your responsibility to ensure that the projects are **compatible** - or deal with resolving differences in case they are **not**. _West_ doesn't do that for you.

For some applications, it can therefore be beneficial to maintain multiple manifest files: E.g., when building for devices from Nordic, you may want to use the [`sdk-nrf`](https://github.com/nrfconnect/sdk-nrf) as a _project_ in your manifest file instead of adding "vanilla" _Zephyr_.

This may be tedious but is necessary and not a downside of using Zephyr: If you were to switch MCUs or if you'd be including a big SDK in any other firmware project, you'd also have to ensure compatibility. West manifests simply make this explicit, reproducible, and manageable.



## Working on projects

Even though we mostly ignored the inner workings of _West_, there's one thing that you need to be aware of when working with West workspaces: Except for the _manifest repository_, _West_ creates and controls a Git branch named `manifest-rev` in each _project_ and thus for all dependencies.

E.g., let's have a look at the `deps/zephyr` repository that we cloned using _West_ for the modified **Blinky** application in the section ["Zephyr with West"](#zephyr-with-west):

```bash
$ cd deps/zephyr
$ git status
HEAD detached at refs/heads/manifest-rev
nothing to commit, working tree clean
```

We're not on Zephyr's `main` branch nor are we on a detached head at a specific revision - we're on the `manifest-rev` branch that is maintained by _West_.

This is done for all _projects_ in a manifest. The manifest repository itself is not affected since - as we've seen before - West does not touch the manifest repository. This behavior is also explained in the [official documentation](https://docs.zephyrproject.org/latest/develop/west/workspaces.html#west-manifest-rev).

For all other _projects_, however, West creates and manages its own `manifest-rev` branch. It is important that you **do not modify the `manifest-rev` branch** and that you don't push it to your remote since West recreates and resets the `manifest-rev` branch on each execution of `west update` command. Any changes would be **lost**.

Being aware how West manages projects is especially important if you're using _West_ in a ["T3 forest topology"](https://docs.zephyrproject.org/latest/develop/west/workspaces.html#topologies-supported) or, e.g., if you're using a separate repository for shared code. E.g., we could add the following `shared/dummy` project to our workspace:

```bash
manifest:
  # --snip--
  projects:
    # --snip--
    - name: dummy
      revision: main
      url: git@github.com:lmapii/practical-zephyr-t2-dummy.git
      path: shared/dummy
```

In case you need to update such a shared dependency, make sure to push the changes to a new or existing branch, but **don't commit to the `manifest-rev` branch**. Also, after running `west update`, make sure to switch back to your working branch.

This sounds brittle, but it really isn't. It isn't that easy to lose changes to files with a `west update`. Let's see this in action. In case you want to follow along, add your own dummy repository to the manifest - I'll be using the above project in my `west.yml` - a dummy repository that contains an empty `test.txt` file:

```bash
# after adding the "dummy" project to west.yml, update the project
$ west update
$ tree --dirsfirst -a -L 4 --filelimit 10 -I hal
.
├── .west
│   └── config
├── app
├── deps
│   ├── modules
│   └── zephyr
└── shared
    └── dummy
        └── test.txt
```

West does not checkout the repository's `main` branch as specified in the with the `revision: main` entry in the manifest file, but instead uses the local `manifest-rev` branch mentioned before:

```bash
$ cd shared/dummy
$ git status
On branch manifest-rev
nothing to commit, working tree clean
$ cd ../../ # workspace root
```

Without checking out a new branch, I can modify the contents of `test.txt` and run `west update`. The changes won't be lost and `west update` succeeds since the changes are not in conflict with any incoming change:

```bash
$ echo 123-test >> shared/dummy/test.txt
$ west update
...
=== updating dummy (shared/dummy):
--- dummy: fetching, need revision main
From github.com:lmapii/practical-zephyr-t2-dummy
 * branch            main       -> FETCH_HEAD
M       test.txt
HEAD is now at c8deb58 initial commit
...

$ cat shared/dummy/test.txt
123-test
```

The output of `west update` shows me that I have in fact local changes to `test.txt`, but it doesn't reset the file. The changes are not lost.

Since we're using `main` as `revision` for this dependency (e.g., since we're still very much in development), it _could_ happen that someone else is writing to `test.txt`, e.g., the value `not-a-test.` (let's not discuss using development branches etc., I'm just showing _West's_ behavior). Trying to run `west update` with an incoming change leads to an error - the local changes are still not lost:

```bash
$ west update
...
=== updating dummy (shared/dummy):
--- dummy: fetching, need revision main
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Total 3 (delta 0), reused 3 (delta 0), pack-reused 0
Unpacking objects: 100% (3/3), 229 bytes | 57.00 KiB/s, done.
From github.com:lmapii/practical-zephyr-t2-dummy
 * branch            main       -> FETCH_HEAD
error: Your local changes to the following files would be overwritten by checkout:
        test.txt
Please commit your changes or stash them before you switch branches.
Aborting
...
ERROR: update failed for project dummy
```

OK, time to really try and mess things up: Let's assume we had a long day and we simply forgot that we're changing things in a repository managed by West. We're changing things in `test.txt` and commit the changes to the local `manifest-rev` branch:

```bash
$ cd shared/dummy
$ git add test.txt
$ git commit -m "an honest mistake"
[detached HEAD 8de0853] an honest mistake
 1 file changed, 1 insertion(+)
```

Running `west update` we can indeed see that we're about to lose the commit:

```bash
cd ../../
$ west update
...
=== updating dummy (shared/dummy):
--- dummy: fetching, need revision main
From github.com:lmapii/practical-zephyr-t2-dummy
 * branch            main       -> FETCH_HEAD
Warning: you are leaving 1 commit behind, not connected to
any of your branches:

  8de0853 an honest mistake

If you want to keep it by creating a new branch, this may be a good time
to do so with:

 git branch <new-branch-name> 8de0853

HEAD is now at d22a413 updated test.txt
...
```

Whoops! Git is telling us that we're about to leave behind a commit and at the same time tells us that nothing is lost yet: We can recover the lost change in a new branch by using the proposed command:

```bash
$ cd shared/dummy
$ git branch recover-an-honest-mistake 8de0853
$ git checkout recover-an-honest-mistake
Previous HEAD position was c8deb58 initial commit
Switched to branch 'recover-an-honest-mistake'
$ cat test.txt
this-is-a-test.123-test%
```

Whew! A close call, but even after committing to our local `manifest-rev` branch the changes are not lost an can be restored as instructed by the command line output.

Keep in mind that all _projects_ in a West manifest are managed entirely by _West_. If you're working on those repositories, use dedicated branches. After **every** _West update_, double-check that you're still working on the correct branch and do **not** commit to the `manifest-rev` branch.

Have a look at the official documentation for [`west update`](https://docs.zephyrproject.org/latest/develop/west/built-in.html#west-update). There, you'll find a detailed description of the entire update procedure, including some options that allow you to selectively update projects.



## Conclusion

In this article, we've seen how we can use _West_ to manage our dependencies, including some of its pitfalls and limitations. Exploring the Zephyr application we have also seen, that managing dependencies can still be complicated in case you're dealing with projects that have the same dependencies, e.g., other SDKs that use different versions of Zephyr as dependency.

We also managed to build an application for MCUs from different vendors: This is possible in Zephyr, though it may take a bit more effort than advertised - at least in case you're dealing with a professional application. Personally, I still think that _Zephyr_ is a great step forward: Switching MCUs or supporting multiple targets _always_ includes efforts, but with Zephyr, the efforts are predictable and manageable.

This article marks the **end of the _Practical Zephyr_ series**: I hope that I could get you past the steep section of Zephyr's learning curve, towards a more enjoyable and reasonable experience. There's still a ton to explore!

A big **Thank You** to the team at [Memfault](https://memfault.com/) for their patience, the reviews, the invaluable feedback for these articles, and for maintaining the [Interrupt](https://interrupt.memfault.com/) blog!



## Further reading

We've only covered the surface of what _West_ can do for you. I very warmly recommend at least skimming through the following resources:

- Watch Mike Szczys' talk ["Manifests: Project Sanity in the Ever-Changing Zephyr World"](https://www.youtube.com/watch?v=PVhu5rg_SGY) from the 2022 Zephyr Developer Summit, lots of what you've read here comes from this presentation!
- If you like the `app` and `deps` separation, have a look at [Golioth's Zephyr training repository](https://github.com/golioth/zephyr-training) to see how it is applied in practice.
- Jonathan Beri also has a great [example `.vscode` configuration](https://github.com/beriberikix/zephyr-vscode-example) in case you're working with Visual Studio Code.
- Zephyr's own [example workspace application](https://github.com/zephyrproject-rtos/example-application) is of course also a great resource - and should be much more understandable now that you've read through this article.

Finally, have a look at the files in the example repositories used throughout this article:
- [practical-zephyr-t2-empty-ws](https://github.com/lmapii/practical-zephyr-t2-empty-ws) contains an empty _manifest_ repository for creating a dummy _West_ workspace that we'll use to explore _West_ without Zephyr.
- [practical-zephyr-manifest-repository](https://github.com/lmapii/practical-zephyr-manifest-repository) contains the _manifest repository_ of the Zephyr application that we're building in this article.



<!-- References

[cmake]: https://cmake.org/
[doxygen-awesome]: https://github.com/jothepro/doxygen-awesome-css
[golioth]: https://golioth.io/
[stm-nucleo]: https://www.st.com/en/evaluation-tools/nucleo-f411re.html

[nrf-connect-sdk]: https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk
[nrf-connect-mgr]: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation/assistant.html#install-toolchain-manager
[nordicsemi-nrf52840-dk]: https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk

[zephyr-hal-stm32]: https://github.com/zephyrproject-rtos/hal_stm32
[zephyr-sdk]: https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html

[zephyr-west]: https://docs.zephyrproject.org/latest/develop/west/index.html
[zephyr-west-install]: https://docs.zephyrproject.org/latest/develop/west/install.html
[zephyr-west-cmd-ext]: https://docs.zephyrproject.org/latest/develop/west/zephyr-cmds.html
[zephyr-west-basics]: https://docs.zephyrproject.org/latest/develop/west/basics.html
[zephyr-west-basics-init-update]: https://docs.zephyrproject.org/latest/develop/west/basics.html#west-init-and-west-update
[zephyr-west-manifest]: https://docs.zephyrproject.org/latest/develop/west/manifest.html
[zephyr-west-workspaces]: https://docs.zephyrproject.org/latest/develop/west/workspaces.html
[zephyr-west-config]: https://docs.zephyrproject.org/latest/develop/west/config.html
[zephyr-west-remotes]: https://docs.zephyrproject.org/latest/develop/west/manifest.html#projects
[zephyr-west-manifest-remotes]: https://docs.zephyrproject.org/latest/develop/west/manifest.html#remotes
[zephyr-west-manifest-rev]: https://docs.zephyrproject.org/latest/develop/west/workspaces.html#west-manifest-rev
[zephyr-west-workspace-topologies]: https://docs.zephyrproject.org/latest/develop/west/workspaces.html#topologies-supported

[zephyr-ds22-manifests]: https://www.youtube.com/watch?v=PVhu5rg_SGY

[zephyr-gh-docker]: https://github.com/zephyrproject-rtos/docker-image

-->
