---
title: "Pocket article: Continuous Integration with GitHub Actions"
author: leoribg
description: A step by step guide on how to setup continuous integration for firmw
tags: [firmware-ci]
---

## Introduction

<!-- excerpt start -->

Using Continuous Integration (CI) practices is essential to bringing security, quality, and automation to your firmware development team and release pipeline. Without CI, firmware builds might become non-[reproducible]({% post_url
2019-12-11-reproducible-firmware-builds %}), certain build environments or configurations may deteriorate or break, and there will be less confidence in production builds that are shipped from individual developer machines.

In this post, I will show how to set up a Continuous Integration pipeline using GitHub and Docker to help your team integrate the features into a mainline firmware project.

<!-- excerpt end -->

## Continuous Integration Tools

Continuous Integration is a practice of validating the features integrated into the main project at a regular cadence or with each change to the main repo.

For that, you will need to perform at least these steps:

- **[Git flow](https://nvie.com/posts/a-successful-git-branching-model/)** - Git usage model to handle development and release branches in a Git repository.
- **Automatic compliance**  - Check if the project is still compiling before merging a feature.
- **Code quality** - Check if there are vulnerabilities in the code.

To complete these steps, we will use the following tools:

- **Make** - Firmware development IDE environments and custom build systems are usually subpar. To be able to more easily build the firmware, we'll generate Makefiles and use Make as our build system.
- **GitHub Actions** - It kicks off a compilation and static code analysis on the cloud. It allows us to run build scripts on every commit or pull request to make sure everything still works with the new changes inserted. ([https://docs.github.com/en/actions](https://docs.github.com/en/actions)).
- **Docker** - The Linux container will compile the project and run static analysis on the code. ([https://www.docker.com/](https://www.docker.com/)) .
- **Cpp Check** - Perform the static analysis of the code and generate a log from it. ([https://cppcheck.sourceforge.io/](https://cppcheck.sourceforge.io/)).

## Creating an Example Project

To give you an example of how to set up a continuous integration pipeline in a firmware project, I will use the [Silicon Labs Thunderboard Sense Board](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit?tab=overview) to run a simple project with [Simplicity Studio IDE](https://www.silabs.com/developers/simplicity-studio) that we will use to build and perform static analysis. 

> Even though the tutorial is specific to the Simplicity Studio build system, the general approach should apply to other build systems.

If you already have a set of Makefiles that you're ready to build, you can skip down to the [next section](#configuring-github-actions).

### Create and Compile Project Locally

Within Simplicity Studio, under My Products, select the blue plus symbol to add a new product.

<p align="center">
  <img
    width="400"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/2-add-new-board.png"
    alt="Add new board"
  />
</p>

<p align="center">
  <img
    width="600"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/3-add-board.png"
    alt="New board"
  />
</p>

After you connect or add the board, go to the Launcher perspective on Simplicity Studio, click on the board, and next click on Create New Project button.

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/4-new-project.png"
    alt="New project"
  />
</p>

Select the Empty C Project and Finish!

<p align="center">
  <img
    width="600"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/5-empty-c.png"
    alt="Empty project"
  />
</p>

To generate the project Makefile, open the empty.slcp file and do the following steps.

<p align="center">
  <img
    width="700"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/6-generators.png"
    alt="Generators"
  />
</p>

Select the GCC Makefile option on Change Project Generators, save, and Force Generation.

<p align="center">
  <img
    width="700"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/7-gcc-makefile-gen.png"
    alt="Makefile Generator"
  />
</p>

After that, the empty.Makefile and empty.project.mak files will be created in the project root directory.

Now you can build the project in the terminal since Simplicity Studio installed the toolchain.

If you are running it on Linux, you can go directly to the terminal, but if you are using Windows, you will need to install [Cygwin](https://www.cygwin.com/) and execute the command.

```bash
make -f empty.Makefile
```

Now the project can be built from the terminal, which is necessary for building within the GitHub Actions CI environment.

## Test the compilation on a Docker Container

We will use a docker image with Simplicity Studio to build our project on a container. This will allow us to configure GitHub Actions to use the docker image to do the same.

Let's start the container using the image that has Simplicity :

```powershell
docker run -it leoribg/simplicity-studio-5
```

Docker will provide you with a shell. Within the shell prefix, it will provide an identifier of the container, which we will use in the next step.

```
# 1027ffa9c954 is the identifier
root@1027ffa9c954:/#
```

In a separate window in your Windows environment, we'll copy the project files to the container:

```powershell
docker cp C:\Users\leonardo\SimplicityStudio\v5_workspace\empty 1027ffa9c954:/project
```

In the original Docker shell terminal window, you can now compile the project and run `cppcheck` to perform static analysis.

```bash
cd /project/
make -f empty.Makefile -j8
cppcheck . --output-file=cpplog.txt
```

### Create the Git Repository

If your project isn't yet version controlled with git and pushed to GitHub, you can follow the [official GitHub instructions](https://docs.github.com/en/get-started/importing-your-projects-to-github/importing-source-code-to-github/adding-locally-hosted-code-to-github) to do so. 

## Configuring GitHub Actions

You can start to configure the GitHub Actions for your project by creating the .yml file with your pipeline inside the “.github/workflows” folder. Or, you can create at your repository page by creating the file following a template provided by GitHub. Let's do the second option, that's easier for those who are starting.

So, go to your repository page and click on the “Actions” tab:

<p align="center">
  <img
    width="400"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/10-actions.png"
    alt="GitHub Actions"
  />
</p>

Choose the right template:

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/11-actions-template.png"
    alt="Actions Template"
  />
</p>

This tutorial will use a single workflow to execute our jobs. So, it will only have a single .yml file.

The .yml file starts like this, we will modify it.

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/12-cpp-workflow.png"
    alt="Cpp Workflow"
  />
</p>

Let's modify the content of our .yml file to execute the build and the static analysis of the project.

We will indicate the Docker Image we use to execute the workflow's steps. After we will build the project, upload the image as an artifact. Finally, we will make a static analysis in the code and upload the artifact with the results.

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/13-cpp-editing.png"
    alt="Cpp Editing"
  />
</p>

## Trigger our GitHub Action

As we have configured the workflow to run every commit or pull-requests on the master branch, when you commit the changes, it will trigger the GitHub Actions on the project.

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/14-commit-workflow.png"
    alt="MakeCommit Workflowfiles"
  />
</p>

After all the tests have been completed and passed, the workflow run is shown like this: 

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/15-workflow-run.png"
    alt="Run Workflow"
  />
</p>

Also, the artifacts were being uploaded and are available for download.

<p align="center">
  <img
    width="800"
    src="/img/setup-a-ci-pipeline-using-github-actions-and-docker/16-workflow-artifacts.png"
    alt="Workflow Artifacts"
  />
</p>

## All done!

You have configured your first CI workflow using GitHub Actions. 

I hope this can help you to start using GitHub Actions on your future projects!

## References & Links

- [Git Flow](https://www.atlassian.com/br/git/tutorials/comparing-workflows/gitflow-workflow)
- [GitHub Actions](https://docs.github.com/en/actions)
- [Docker](https://www.docker.com/)
- [CppCheck](https://cppcheck.sourceforge.io/)
- [Silicon Labs Thunderboard Sense Board](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit?tab=overview)
- [Simplicity Studio IDE](https://www.silabs.com/developers/simplicity-studio)
- [Cygwin](https://www.cygwin.com/)