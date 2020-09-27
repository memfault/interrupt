#!/bin/bash
docker run -it --mount type=bind,source=$(pwd),target=/memfault/interrupt --name interrupt --net host --rm --tty interrupt:v1.0.0 "$@"
