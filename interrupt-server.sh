#!/bin/bash
docker build -t interrupt . && docker run -it -v $(pwd):/memfault/interrupt:cached --rm -p 4000:4000 --rm interrupt