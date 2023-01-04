---
title: Setup a CI pipeline using GitHub Actions and Docker
description: A step by step guide on how to setup continuous integration for firmware projects with GitHub Actions.
author: leoribg
tags: [firmware-ci]
---

# Setup a CI pipeline using GitHub Actions and Docker

# Introduction

Continuous Integration (CI) practices are essential to bringing security, quality, and automation to your firmware development team. 

In this post, I will show how to set up a Continuous Integration pipeline using modern tools to help your team integrate the features into a mainline firmware project step by step.

# Continuous Integration Tools

Continuous Integration is a practice of validating the features integrated into the main project.

For that, you will need to perform at least these steps:

- **Git flow** - Rules to handle branches in a git repository.
- **Automatic compliance**  - Check if the project is still compiling before merging a feature.
- **Code quality** - Check if there are vulnerabilities in the code.

So, to complete these steps, we will use the following tools:

- **GitHub Actions** - It kicks off a compilation and static code analysis on the cloud. It allows us to run build scripts on every commit or pull request to make sure everything still works with the new changes inserted. ([https://docs.github.com/en/actions](https://docs.github.com/en/actions))
- **Docker** - The Linux container will compile the project and run static analysis in the code. ([https://www.docker.com/](https://www.docker.com/)) 
- **Cpp Check** - Perform the static analysis of the code and generate a log from it. ([https://cppcheck.sourceforge.io/](https://cppcheck.sourceforge.io/))

# Project creation

## Board and IDE

This tutorial is specific to the Simplicity Studio build system, but the general approach should apply to other build systems. I will use the [Silicon Labs Thunderboard Sense Board](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit?tab=overview) to run a simple project on [Simplicity Studio IDE](https://www.silabs.com/developers/simplicity-studio) that we will use to build and perform static analysis.

![Thunderboard](/img/setup-a-ci-pipeline-using-github-actions-and-docker/0-thunderboard.png)

## Create the project and compile it on the Local Machine

If you have the Thunderboard Sense board, you can connect it to the PC and check if the Simplicity Studio detects it.

![Board Selection](/img/setup-a-ci-pipeline-using-github-actions-and-docker/1-board-selection.png)

If you do not have the board, you can add it to my products tab and create the project anyway. It is not impeditive to continue. You can follow the steps in the next images.

![Add new board](/img/setup-a-ci-pipeline-using-github-actions-and-docker/2-add-new-board.png)

![New board](/img/setup-a-ci-pipeline-using-github-actions-and-docker/3-add-board.png)

After you connect or add the board, go to the Launcher perspective on Simplicity Studio, click on the board, and next click on Create New Project button.

![New project](/img/setup-a-ci-pipeline-using-github-actions-and-docker/4-new-project.png)

Select the Empty C Project and Finish!

![Empty project](/img/setup-a-ci-pipeline-using-github-actions-and-docker/5-empty-c.png)

To generate the project Makefile, open the empty.slcp file and do the following steps.

![Generators](/img/setup-a-ci-pipeline-using-github-actions-and-docker/6-generators.png)

Select the GCC Makefile option on Change Project Generators, save, and Force Generation.

![Makefile Generator](/img/setup-a-ci-pipeline-using-github-actions-and-docker/7-gcc-makefile-gen.png)

After that, the empty.Makefile and empty.project.mak files will be created in the project root directory.

![Makefiles](/img/setup-a-ci-pipeline-using-github-actions-and-docker/8-makefile.png)

Now you can build the project in the terminal since Simplicity Studio installed the toolchain.

If you are running it on Linux, you can go directly to the terminal, but if you are using Windows, you will need to install [Cygwin](https://www.cygwin.com/) and execute the command.

```bash
make -f empty.Makefile
```

Now the project can be built from the terminal, which is necessary for building in CI.

# Test the compilation on a Docker Container

We will use a docker image based on Ubuntu 18.04 with [Simplicity Studio v5](https://www.silabs.com/documents/login/software/SimplicityStudio-5.tgz) and the necessary [SDKs](https://github.com/SiliconLabs/gecko_sdk) installed to build our project on a container. It will allow us to configure GitHub Actions to use the docker image to do the same.

So let's pull the image to our computer:

```bash
docker pull leoribg/simplicity-studio-5:latest
```

Start the container from the image:

```bash
docker run -t -d leoribg/simplicity-studio-5
```

Copy the project files to the container:

```bash
docker cp C:\Users\leonardo\SimplicityStudio\v5_workspace\empty 1027ffa9c954:/project
```

Compile the project and run the `cppcheck` static code analysis tool:

```bash
cd /project/
make -f empty.Makefile -j8
cppcheck . --output-file=cpplog.txt
```

## Create the Git Repository

Since the project was created, let's create a git repository. 

For this tutorial, this is the repository I've used for my project:
[https://github.com/leoribg/memfault-empty-example](https://github.com/leoribg/memfault-empty-example)

[To start and add your project to a git repository](https://docs.github.com/en/get-started/quickstart/create-a-repo), run the following commands on your project directory:

```bash
git init
git add .
git commit -m "first commit"
git remote add origin git@github.com:leoribg/memfault-empty-example.git
git push -u origin master
```

# Configure the GitHub Actions

You can start to configure the GitHub Actions for your project by creating the .yml file with your pipeline inside the “.github/workflows” folder. Or, you can create at your repository page by creating the file following a template provided by GitHub. Let's do the second option, that's easier for those who are starting.

So, go to your repository page and click on the “Actions” tab:

![GitHub Actions](/img/setup-a-ci-pipeline-using-github-actions-and-docker/10-actions.png)

Choose the right template:

![Actions Template](/img/setup-a-ci-pipeline-using-github-actions-and-docker/11-actions-template.png)

This tutorial will use a single workflow to execute our jobs. So, it will only have a single .yml file.

The c-cpp.yml file starts like this, we will modify it.

```yaml
name: C/C++ CI

on: #These are the triggers that will run our jobs
  push:
    branches: [ "master" ]
  pull_request:
    branches: [ "master" ]

jobs: #Jobs are set of steps that will execute our tasks
  build:

    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3
    - name: configure
      run: ./configure
    - name: make
      run: make
    - name: make check
      run: make check
    - name: make distcheck
      run: make distcheck
```

Let's modify the content of our .yml file to execute the build and the static analysis of the project.

We will indicate the Docker Image we use to execute the workflow's steps. After we will build the project, upload the image as an artifact. Finally, we will make a static analysis in the code and upload the artifact with the results.

```yaml
name: C/C++ CI

on:
  push:
    branches: [ "master" ]
  pull_request:
    branches: [ "master" ]

jobs:
  build:

    runs-on: ubuntu-latest
    container:
      image: leoribg/simplicity-studio-5:latest #Docker Image

    steps:
    - uses: actions/checkout@v3
    - name: make #Build
      run: make -f empty.Makefile
    - name: binary artifact #Upload the image artifact
      uses: actions/upload-artifact@v3
      with:
        name: empty.s37
        path: ./build/debug/empty.s37
    - name: static analisys #Lint
      run: cppcheck . --output-file=cpplog.txt
    - name: lint artifact #Upload the lint artifact
      uses: actions/upload-artifact@v3
      with:
        name: lint_output.txt
        path: ./cpplog.txt
```

# Trigger our GitHub Action

As we have configured the workflow to run every commit or pull-requests on the master branch, when you commit the changes, it will trigger the GitHub Actions on the project.

![Commit Workflow](/img/setup-a-ci-pipeline-using-github-actions-and-docker/14-commit-workflow.png)

After all the tests have been completed and passed, the workflow run is shown like this: 

![Run Workflow](/img/setup-a-ci-pipeline-using-github-actions-and-docker/15-workflow-run.png)

Also, the artifacts were being uploaded and are available for download.

![Workflow Artifacts](/img/setup-a-ci-pipeline-using-github-actions-and-docker/16-workflow-artifacts.png)

# All done!

You have configured your first CI workflow using GitHub Actions. 

I hope this can help you to start using GitHub Actions on your future projects!

## Reference & Links

[^1]: [Git Flow](https://www.atlassian.com/br/git/tutorials/comparing-workflows/gitflow-workflow).
[^2]: [GitHub Actions](https://docs.github.com/en/actions)
[^3]: [Docker](https://www.docker.com/)
[^4]: [CppCheck](https://cppcheck.sourceforge.io/)
[^5]: [Silicon Labs Thunderboard Sense Board](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit?tab=overview)
[^6]: [Simplicity Studio IDE](https://www.silabs.com/developers/simplicity-studio)
[^7]: [Cygwin](https://www.cygwin.com/)