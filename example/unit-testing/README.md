Here are two examples from the Interrupt post [Embedded C/C++ Unit Testing Basics](https://interrupt.memfault.com/blog/unittests-cpputest). 

## Initial Setup

```
# macOS
brew install cpputest lcov

# Ubutunu
sudo apt-get install -y cpputest lcov
```

In `MakefileWorkerOverrides.mk`, it is also necessary to define the two values, which depend
on where CppUTest was installed on the platform.

```
CPPUTEST_HOME ?= /usr/local/Cellar/cpputest/3.8
TARGET_PLATFORM ?= 
```

## Minimal Example

This contains a very simple module `my_sum.c` which is tested by a simple test located at `tests/src/test_my_sum.cpp`

The tests can be run by running:

```
$ cd minimal/tests
$ make
$ make lcov
$ # open build/test_coverage/index.html
```

## Complex Example

This example includes using [littlefs](https://github.com/ARMmbed/littlefs) in a unit test so we
need to clone the repository first

The tests can be run by running:

```
$ cd complex
$ git clone -b v2.1.2 https://github.com/ARMmbed/littlefs.git
$ cd tests
$ make
$ make lcov
$ # open build/test_coverage/index.html
```
