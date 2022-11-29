#!/bin/sh

if ! command -v docker &> /dev/null
then
    echo "docker could not be found"
    exit
fi

docker_hash=`docker build -q .`

# Check for failure
if [ $? -ne 0 ]; then
    echo "Build failed on Dockerfile"
    exit
fi

# Parse lua file to make sure there are no errors
docker run --rm -it -v`pwd`:/app $docker_hash luac -p example3/example3.lua
if [ $? -ne 0 ]; then
    echo "Build failed on parsing lua file"
    exit 1
fi

# Use SWIG generator
docker run --rm -it -v`pwd`:/app $docker_hash swig -lua -o example3/swig.c example3/bindings.i
if [ $? -ne 0 ]; then
    echo "Build failed on swig bindings"
    exit
fi

# Generate SWIG runtime
docker run --rm -it -v`pwd`:/app $docker_hash swig -lua -external-runtime example3/swig_runtime.h
if [ $? -ne 0 ]; then
    echo "Build failed on swig runtime"
    exit
fi

# Run cmake
docker run --rm -it -v`pwd`:/app $docker_hash cmake -S example3/ -B example3/build
if [ $? -ne 0 ]; then
    echo "Build failed on cmake gen"
    exit
fi

# Build application
docker run --rm -it -v`pwd`:/app $docker_hash cmake --build example3/build
if [ $? -ne 0 ]; then
    echo "Build failed on cmake make"
    exit
fi

# Run application inside Docker
docker run --rm -it -v`pwd`:/app $docker_hash /bin/sh -c "cd example3 && ./build/example3"
