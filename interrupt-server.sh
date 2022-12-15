#!/usr/bin/env bash

# This script is used to run the interrupt site locally in a docker container.

# Optionally, build a local image instead of pulling prebuilt
if [ -n "${BUILD_DOCKER_IMAGE:-}" ]; then
    docker build -t memfault/interrupt .
fi

docker run --rm -i -t --publish=4000:4000 --volume "${PWD}":/memfault/interrupt memfault/interrupt
