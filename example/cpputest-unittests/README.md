Here are two examples from the Interrupt post [Embedded C/C++ Unit Testing Basics](https://memfault.com/blog/unittests-cpputest). 

## Minimal Example

This contains a very simple module `my_sum.c` which is tested by a simple test located at `tests/src/test_my_sum.cpp`

The tests can be run by running:

```
$ cd minimal/tests
$ make
```

## Complex Example

The tests can be run by running:

```
$ cd complex
$ git clone https://github.com/ARMmbed/littlefs.git
$ cd tests
$ make
```
