# Docker build container


## Build renode minimal example

Create a minimal docker image that contains everything to build
the binary. The binary is build during the creation of the image.

> docker build --progress=plain -t renode .


## Get example to host

Start the container with
> docker run --mount src="$(pwd)",target=/mnt,type=bind -it renode /bin/bash

In the container go into folder renode and copy the build binary to the host.
> cp renode-example.elf /mnt
