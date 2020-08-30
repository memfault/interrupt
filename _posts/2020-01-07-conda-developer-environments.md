---
title: "Managing Developer Environments with Conda"
description:
  "How to use Conda for managing build, test, and debug environments and
  dependencies for local development and continuous integration in the context
  of embedded software projects"
tag: [python, better-firmware]
author: tyler
image: /img/conda-developer-environments/cover.png
---

We've all had the experience of checking out an old release branch to debug an
issue, only to find out that it depends on toolchains and packages that are no
longer installed. This can be a major pain, and people have come up with several
ways around it.

In previous posts,
[we have used virtualenv]({% post_url 2019-07-23-using-pypi-packages-with-GDB %}#virtual-environment)
to manage our Python packages. What if you would like to easily manage non
Python dependencies? This is where Conda comes into play.

<!-- excerpt start -->

This post gives an introduction to what Conda is, explains why you should care
about keeping your developer environments in sync, and finally provides a
walk-through on getting started with Conda to set up a GCC based developer
environment.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Who Is This Post For?

Every developer should build, debug, and test the firmware using the **exact
same versions** of the compiler, build system, and other tools. The specific
tools used should also be in lock-step with the code base revision being worked
on. This ensures that there is one way to do all everyday tasks and they
**always work**. It's also a critical step towards having [reproducible
builds]({% post_url 2019-12-11-reproducible-firmware-builds %}).

If you or your teammates have experienced any of the following more than once,
it may be time to consider using Conda or similar environment manager.

- After coming back from a 2 week vacation, you find that getting a functional
  development environment again takes hours or even days.
- Updating a package required by the firmware build is a monumental effort and
  requires communication in all channels, including email, slack, and various
  wiki pages.
- You find that a build from your machine works correctly but not the one from
  CI or your co-workers machine.
- You check out a 6 month-old revision of your project and realize it's probably
  quicker to reformat your computer and start over than to try and piece
  together an environment to build the firmware.
- You use Windows, macOS, or Linux _without_ Docker.

## What is Conda?

![Conda]({% img_url conda-developer-environments/conda.png %})

Conda is a "package, dependency and environment management for any
language"[^1]. It is most popular in the world of data science, but it can
easily be used in any field, including firmware development! Conda has become my
favorite way to manage the developer and continuous integration environments for
my firmware projects.

Conda is primarily used to manage Python distributions and environments. It can
either be used as a replacement for virtualenv[^2] or used alongside it if
necessary. It also provides `pip` so that PyPi packages can be installed as
well.

The primary benefit of using Conda over Virtualenv is its ability to install and
manage system binary packages that one may need in a developer environment, such
as the `arm-none-eabi-gcc` toolchain, `wget`, or Make.

## How Does Conda Work?

When a developer installs Conda on their computer, a command-line tool called
`conda` becomes available. `conda` is used to install, manage, and activate the
Conda virtual environments.

The local Conda environment is kept in sync with a manifest file, typically
called `environment.yml` and revision controlled. It works similarly to a
`requirements.txt` file in Python or `package.json` in the JavaScript world.
This enables a developer to check out **any revision** from the past and
immediately have the proper tools installed to build and debug.

There are also public repositories which host packages built by Anaconda, Inc.
and the community. The two most popular are the self-named Anaconda and
[Conda Forge](https://conda-forge.org/) repositories. Conda Forge is supported
by the community and developers contribute packages through Github, after which
they are built and released for all platforms through their CI/CD system.

When a package, Python or binary, is installed through Conda to a particular
environment, it is placed in a separate "environment" directory (e.g.
`~/miniconda3/envs/example`). To activate these virtual environments, Conda will
edit the shell's `$PATH` variable, prepending it with the directory of the
desired environment. For example, if we activate the `example` environment, the
`$PATH` variable becomes

```
# Activate environment
$ conda activate example

(example)
$ echo $PATH
/Users/tyler/miniconda3/envs/example/bin:...
```

This forces the current shell to look within the above directory _first_ before
searching the default package paths, such as `/usr/bin` or `/usr/local/bin`. For
example, below we print which `make` binary and version is used by the shell,
before and after activating a Conda environment.

```
$ which make && make --version
/usr/bin/make
GNU Make 3.81

# Activate our Conda environment
$ conda activate example

(example)
$ which make && make --version
/Users/tyler/miniconda3/envs/example/bin/make
GNU Make 4.2.1
```

### Common Developer Environment Alternatives

I have found Conda to be the least restrictive and easiest way for firmware
teams to manage developer environments, but let's go through the most popular
alternatives.

#### Docker

Docker is the best way to create developer environments from scratch and for
temporary purposes (such as in continuous integration). A `Dockerfile`, which
commonly lives in a project's repo, defines any and all setup command, such as
downloading and install system packages, installing Python requirements, and
setting up permissions and volumes.

Despite the numerous benefits, Docker is not a great fit for local developer
environments.

- By using Docker, you'll have two file systems, one on the host machine and one
  within the container. Without a large investment in up-front tooling to help
  developers manage the code, build artifacts, debugging tools, and volume
  mounts, it is cumbersome and slow.
- On macOS or Windows, shared volumes have file-sharing performance issues,
  which can cause build times to increase between 2-10x depending on where the
  files live.[^3]
- For developers that like the use of IDE's and other visual tools, a virtual
  machine using VMWare or VirtualBox may be more appropriate.

With all that said, using Docker in your CI system is a no-brainer. Conda and
Docker can even be used _together_, where Docker manages the system image and
low-level setup, and Conda manages all of the packages required to build and
test the firmware.

Check out [this previous Interrupt
post]({% post_url 2019-09-17-continuous-integration-for-firmware %}) for how to
get up and running quickly with Docker and continuous integration.

#### Included System Package Managers

System package managers, such as Brew, Apt, MinGW, etc., are very good at
installing packages and managing dependencies between them. They are also great
at ensuring that the modern software you use everyday works correctly, safely,
and is kept up-to-date. They are not good at ensuring that the software or
firmware that **you** build continues to work correctly. They are also not good
at juggling multiple versions of packages that aren't the default ones.

It is not uncommon for firmware consultancies or companies to have multiple
different firmware projects in flight. This could be a legacy project and a
newer one. I imagine the following wouldn't be far off:

**Old Project Dependencies:** GNU Make 4.1, GCC 5.4.1, GDB 7.11, Python
3.5<br />
**New Project Dependencies:** GNU Make 4.2.1, GCC 8.3.1, GDB 8.3,
Python 3.6

In the above example, to be able to go back and forth between these two
projects, we'd have to uninstall and reinstall these four packages and their
dependencies. We'd also be taking the risky bet that doing so wouldn't affect
other packages that we have installed.

In an ideal world, you should be able to use `brew upgrade` or
`apt update && apt upgrade` anytime and it wouldn't affect anything related to
the project's firmware build. This is **not possible** when using system package
managers and I strongly advise against using this approach.

#### Virtualenv

Virtualenv works well for managing Python environments and packages, but it
doesn't manage system binaries. It also requires that each Python version be
installed prior to being used in a virtual environment, such as with `pyenv` or
the system package manager.

You can use Virtualenv alongside Conda as well if that is necessary, but it is
not required. Conda is a full replacement for Virtualenv.

#### Virtual Machine

Using a virtual machine is a great way to isolate the environment even more so
that every developer uses a similar environment and is provided with a GUI
and/or IDE. It's also especially useful if everyone has a Linux machine and the
build requires Windows, or vice-versa. However, I still find issues with this
approach.

- Sharing file systems between the host and image has performance issues.
- Using the UI within a virtual machine is generally slower.
- One still would need some sort of environment management with the VM, as
  packages will inevitably change every couple of months.

If a VM is chosen, I suggest using Conda **within it** to help manage the
packages that are used within the VM. Otherwise, the team would have to package
and distribute a new VM image each time a dependency changes, or hope that
everyone knows when and how to update the system (and does it).

## Example Conda Walk-through

In this part of the post, we will set up Conda on our local machine and set up
an environment that is able to build, flash, and debug an nRF52 device.

If you'd like to follow along, you may download the Interrupt repo and navigate
to the example directory.

```
$ git clone git@github.com:memfault/interrupt.git
$ cd interrupt/example/conda-developer-environments/
```

> Before we begin, I want to bring comfort to anyone skeptical about installing
> yet another tool. Conda is the opposite of invasive, and everything is
> installed into a single directory, `$HOME/miniconda3` by default. If you want
> to uninstall Conda, deleting that directory will be all that is necessary.

### Installing Conda

To install the `conda` client on a machine, the recommended way is to download
the interactive
[Miniconda installer](https://docs.conda.io/en/latest/miniconda.html) and run it
through `bash`. The following steps apply to both Linux and macOS. If you are
using Windows, there is a Miniconda installer that can be downloaded from the
same page.

```bash
# Download for Mac:
$ wget https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh
# Download for Linux:
$ wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
# install it
$ bash Miniconda3-latest-*-x86_64.sh
```

The final step in the installer is to make sure the
`$HOME/miniconda3/etc/profile.d/conda.sh` script is loaded in the shell. When
the following prompt is given:

```
Do you wish the installer to initialize Miniconda3
by running conda init? [yes|no]
```

If you'd like Conda to edit your `.bashrc` or `.zshrc` file automatically, you
can say `yes` here. If you do, I suggest running the following command after the
installation is complete so that the base Conda environment won't automatically
activate.

```
$ conda config --set auto_activate_base false
```

If you would rather edit the startup file yourself, add the following line:

```
. "$HOME/miniconda3/etc/profile.d/conda.sh"
```

If everything was successful, after reloading our shell, we can now run the
`conda` command in the shell.

```
$ conda --version
conda 4.7.12
```

### Creating the Environment

The next step is to create an isolated environment in which we can install
packages into. We will create an environment called `example` using Conda's
[`create` command](https://docs.conda.io/projects/conda/en/latest/commands/create.html)

```
$ conda create -n example
Collecting package metadata (current_repodata.json): done
Solving environment: done

## Package Plan ##

  environment location: /Users/tyler/miniconda3/envs/example

Proceed ([y]/n)? y

Preparing transaction: done
Verifying transaction: done
Executing transaction: done
#
# To activate this environment, use
#
#     $ conda activate example
#
# To deactivate an active environment, use
#
#     $ conda deactivate

```

Now, we are able to load up our environment using:

```
$ conda activate example
```

Our current environment has zero packages installed. We can double check this by
running `conda list`.

```
(example)
$ conda list
# packages in environment at /Users/tyler/miniconda3/envs/example:
#
# Name                    Version                   Build  Channel

```

### Configuring the Environment

We want to install the following packages:

- ARM GCC Toolchain (8-2019-q3-update)
- Make (4.2.1)
- Python (3.6)
- nrfutil (6.0.0)

There are two ways to install packages. The first is by installing them directly
with the CLI (e.g. `conda install make`), and the second is by defining them in
a `environment.yml` file, generally placed in the root of the project. We will
use the second method.

There are also multiple repositories to install packages from. The two most
popular ones are "Anaconda" and "Conda Forge". We will use Conda Forge as our
primary repository, since it has more mainstream packages that we as firmware
developers use. We will also add Memfault's Conda repository (memfault), as it
has the ARM GCC package.

To search for packages that can be installed, you can navigate to the
[Conda Forge](https://anaconda.org/conda-forge) repository and browse around. It
is also possible to install community packages built by others, but mileage may
vary.

Below is the `environment.yml` file that contains all of our dependencies.

```yaml
channels:
  - memfault
  - conda-forge
  - defaults
dependencies:
  - make=4.2.1
  - gcc-arm-none-eabi=8.2019.q3.update
  - python=3.6.9
  - pip
  - pip:
      - nrfutil==6.0.0
```

If you'd rather manage your `pip` packages in a separate `requirements.txt`
file, the following should work for the `pip:` section.

```
  - pip:
    - -r requirements.txt
```

### Populating the Environment

With our `environment.yml` file set with all the dependencies, we are ready to
install them! This is done using the `conda env update` command.

```
conda env update -f environment.yml
```

If successful, we'll now have all the packages we defined available on our
`$PATH` and stored within the `~/miniconda3/envs/example` environment directory.

```
$ which make
/Users/tyler/miniconda3/envs/example/bin/make

$ which arm-none-eabi-gdb-py
/Users/tyler/miniconda3/envs/example/bin/arm-none-eabi-gdb-py
```

We are now able to build, flash, and debug as any developer should using `make`,
`arm-none-eabi-gcc`, and `nrfjprog`.

> If any developer joins the team and needs to set up an environment, the only
> two things they would need to do is install Conda and run
> `conda create -n my_env -f environment.yml` in the project. Easy, right!?

### Adding Another Package

Let's assume we have 10 engineers working on our project distributed across the
globe. We also have a problem where our firmware builds are slowing down, and
we'd like to speed them up. A clever engineer realized that we could be using
`ccache` to locally cache built object files between subsequent builds. This
speeds up our firmware build by 50%, which is a huge win!

We want _everyone_ to have `ccache` installed and to use it with every build. We
could send out an email to the team, mention it in Slack, and maybe even error
out in the Makefile if we see the developer does not have it installed. All
these methods are tedious and not 100% reliable.

Since every developer on the project is using Conda at this point, we can add an
entry for `ccache` in our `environment.yml` and this becomes trivial.

```yaml
dependencies:
  # System Packages
  ...
  - ccache
```

The next time a developer runs `conda env update`, they'll have `ccache`
available on their `PATH` and can start using it immediately.

```
$ conda env update -f environment.yml
$ which ccache
/Users/tyler/miniconda3/envs/test/bin/ccache
```

## Other Benefits of Using Conda

The more I used Conda, the more I enjoyed the subtle but enormous benefits. They
have all saved me hours of time both for myself and my co-workers. I have
compiled my favorite benefits below.

### Easily Re-Create Environments

It's not uncommon for your build environment to stop working. Maybe a package
becomes corrupted, or a package is accidentally installed or uninstalled. Or
maybe you run into a problem locally and want to confirm that it _isn't_ your
build environment before reaching out to your teammates.

With Conda, this becomes incredibly simple! You can very quickly delete and
re-create your environment using

```
$ conda env remove -n my_env
$ conda env create -n my_env -f environment.yml
$ conda activate my_env
```

This will remove the old environment **in its entirety** and re-create it. The
process takes about 30 seconds. In comparison, re-provisioning a virtual machine
or your computer's operating system could take an hour or a whole day
respectively.

### Bouncing Between Projects and Commits

If you change projects, commits, or branches often, it's simple to make sure
that all packages are installed and with the correct versions.

```
$ git checkout <build from 9 months ago>
$ conda env update -f environment.yml
$ make
```

### Easier Git Bisect

For those who have used `git bisect` over a large date range (3+ months), you
might recall how difficult it can be. This is because for every commit, you need
install the versions of the packages that were required **at that time**. Some
teams do this through a revision controlled wiki page, change-log in the
README.md, or through a (sigh) guess-and-check method. If it builds, it works,
right? (No)

With Conda, the build and test loop becomes simple!

```
# Update all required packages
$ conda env update -f environment.yml

# Build and flash the firmware
$ make && flash

# Test the system

# Mark as good or bad
$ git bisect [good|bad]

# Repeat many times...
```

### Distributing a Conda Environment

If you want to be able to export and distribute a Conda environment, including
Python, pip packages, and system packages, this is possible using the official
Conda package called [Conda Pack](https://conda.github.io/conda-pack/).

This is useful when you need to give a build or test environment to an end
customer, a vendor, or a factory in China. It is also useful where the recipient
has limited or no access to the Internet. These packaged environments can be
used **without** installing Python, Conda, or any other packages on the host
machine.

## Creating Your Own Conda Packages

There is a high chance that all packages required by your project, team, or
developers are not yet built for Conda and hosted on the Conda Forge repository.
Fret not, as building and publishing Conda packages is relatively simple. We'll
quickly walk through how I built and published the `gcc-arm-none-eabi` package
that is hosted [here](https://anaconda.org/memfault/gcc-arm-none-eabi) and built

[ARM GNU Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).

Creating a package for this toolchain is straight-forward since the binaries are
already built for each platform. All we need to do is download the tarball,
extract it, and copy the contents into a Conda package.

> If you want to jump ahead and look at the final recipe, it is hosted in
> Memfault's
> [conda-recipes Github repo](https://github.com/memfault/conda-recipes/tree/master/gcc-arm-none-eabi).

### Creating the Build Environment

The first thing we need to do is create an environment which has `conda-build`
and `anaconda-client` installed, both of which will be used to build and publish
our package.

```
# Create the Conda Environment we'll use for building
$ conda create -n build

# Activate the environment called `build`
$ conda activate build

# Install the necessary packages to build and publish Conda packages
(build)
$ conda install conda-build anaconda-client
```

Now we'll want to create an empty directory, with a single `meta.yaml` file
within it.

```
$ mkdir /tmp/arm-none-eabi-gcc && cd /tmp/arm-none-eabi-gcc
$ touch meta.yaml
```

### Defining the `meta.yaml` File

Next, we'll set up our `meta.yaml` file with the following information, with
each field described below. The definition is small because we are merely
repackaging binaries.

```yaml
package:
  name: gcc-arm-none-eabi
  version: 8.2019.q3.update

source:
  url: https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-mac.tar.bz2 # [osx]
  sha256: fc235ce853bf3bceba46eff4b95764c5935ca07fc4998762ef5e5b7d05f37085 # [osx]

build:
  string: "0"

test:
  commands:
    - arm-none-eabi-gcc --version
```

- `package`: This is the Name and Version of the Conda package that we are
  building. e.g. if we are building Make 4.2.1 for Conda, then Name would be
  "Make" and Version would be "4.2.1".
- `source`: This is either the fields `url` and `sha256` which points to a Zip
  or Tarball containing the source code or binaries, or it can also reference a
  Git repository using the `git_url` and `git_rev` fields.
- `build`: This is the build identifier. If a user publishes a package but it
  was built incorrectly, they would use a new version of this identifier when
  they re-publish the package. A monotonically increasing integer or a random
  hash is appropriate.
- `test`: These are test commands to run against the package once it is build.
  In our example above, we have the package print out the version.

For information about all the fields in the `meta.yaml` file, reference
[this documentation page](https://docs.conda.io/projects/conda-build/en/latest/resources/define-metadata.html)

### Writing the build script `build.sh`

The last thing we need is to define a `build.sh` file. This script will be run
after the build completes, and is responsible for building and moving the
necessary binaries and supporting files to the directory that will be packaged
up.

If we download and extract the toolchain from the ARM website, the folder
structure is as follows:

```
gcc-arm-none-eabi-8-2019-q3-update
├── arm-none-eabi
├── bin
├── lib
└── share
```

We'll want to copy **all** of these directories into our Conda package, so our
script is simply

```
#!/usr/bin/env bash
cp -R $SRC_DIR/* $PREFIX
```

`SRC_DIR` is the directory where the source files were extracted to, and
`PREFIX` is the build prefix to which the build script should install the
resulting files. For more variables exported during the build process, reference
[this page](https://docs.conda.io/projects/conda-build/en/latest/user-guide/environment-variables.html#environment-variables-set-during-the-build-process).

### Building and Publishing the Package

We've finished setting up our package metadata, so now it's time to build and
publish it. To build the package, we run

```
(build)
$ conda build .
```

This will download our source tarball from the ARM website, verify the checksum,
extract the contents, and then, using the `build.sh` script, copy the contents
to the package working directory, which it will then compress back into a
tarball into a Conda package!

After the build finishes, the command will output another command starting with
`anaconda upload ...` which you can use to publish this new Conda package under
your own personal repository.

However, before publishing a package, I generally like to confirm it works in a
clean Conda environment on my local machine. We can do this by pointing to our
`build` environments local repository and specifying the package name
`gcc-arm-none-eabi` when installing the package.

```
(build)
$ conda deactivate
$ conda create -n test
$ conda activate test
(test)
$ conda install -c $HOME/miniconda3/envs/build/conda-bld/ gcc-arm-none-eabi
```

After the package is installed, we can verify it's in our new Conda environment
and can run the `--version` command

```
$ which arm-none-eabi-gcc
/Users/tyler/miniconda3/envs/test/bin/arm-none-eabi-gcc
(test)
$ arm-none-eabi-gcc --version
arm-none-eabi-gcc (GNU Tools for Arm Embedded Processors 8-2019-q3-update) ...
```

It works! Now we can publish it using the `anaconda upload ...` command printed
previously in the terminal.

> NOTE: It is also possible to automate this test procedure by adding test
> scripts. The
> [Conda docs](https://docs.conda.io/projects/conda-build/en/latest/resources/define-metadata.html#test-section)
> are a great place to start and
> [here](https://github.com/memfault/conda-recipes/blob/master/gcc-arm-none-eabi/meta.yaml#L16-L18)
> is an example which performs the steps above.

This was by no means a comprehensive tutorial of how to build a Conda package,
so I suggest reading the following references if you want to learn more.

- [Building conda packages from scratch](https://docs.conda.io/projects/conda-build/en/latest/user-guide/tutorials/build-pkgs.html)
- [Defining metadata (meta.yaml)](https://docs.conda.io/projects/conda-build/en/latest/resources/define-metadata.html)
- [Build scripts (build.sh, bld.bat)](https://docs.conda.io/projects/conda-build/en/latest/resources/build-scripts.html)

If you want to look at a more detailed recipe, I've also published a Conda
recipe that builds and packages the popular C++ unit testing framework
CppUTest[^4]
[on Github](https://github.com/memfault/conda-recipes/tree/master/cpputest)

{:.no_toc}

## Closing

I'm still learning more and more about Conda each month, but ever since using it
a few years ago at my previous job, I haven't looked back. I believe using Conda
and [setting up
Invoke]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}) for
projects were two amazing ways we were able to 2x our teams productivity with
minimal effort.

Have you found something that works well for you and your team? I'd be curious
to hear what other people have found success with to keep developer environments
in sync.

{:.no_toc}

## References

[^1]: [Conda](https://docs.conda.io/en/latest/)
[^2]: [Virtualenv](https://virtualenv.pypa.io/en/latest/)
[^3]: [Docker Volume Mount Filesystem Performance](https://docs.docker.com/docker-for-mac/osxfs-caching/)
[^4]: [CppUTest](http://cpputest.github.io/)
