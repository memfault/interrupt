#!/usr/bin/env bash

set -e

# This script is used to run the interrupt site locally in a docker container.
docker build -t memfault/interrupt .
docker run --rm -i -t --publish=4000:4000 --volume "${PWD}":/memfault/interrupt memfault/interrupt
