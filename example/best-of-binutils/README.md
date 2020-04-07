# Introduction

Example code used while writing the Best of GNU Binutils interrupt post. Two directories:

- nrf52_example - A simple application targetting the nrf52 used in the article to explore code
  size. More details can be found in the README in the directory
- mylib_desktop - Some example code used for static libraries which can be compiled locally

The article also referenced a Zephyr example build. Detailed instructions about how to set up the
project can be found [here](https://docs.zephyrproject.org/latest/getting_started/index.html)

Once the project is set up, the demo app can be built by:

```
$ cd <zephyr_project>/samples/net/wifi
$ west build -b disco_l475_iot1
```
