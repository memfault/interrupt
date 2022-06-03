---
title: Building an On-Device Embedded Testing Library
description: Documents the design, architecture, implementation, and pros and cons of an on-device embedded MCU testing library written in C++ and compatible with Google Test.
author: veverak
tags: [c++, testing]
---
<!-- excerpt start -->

There are too few C/C++ testing libraries designed for embedded devices. The traditional libraries are not designed for constrained resources and rely on host functionality like a filesystem or standard output.

In this post, I detail why I've decided to design a new testing library for microcontrollers and cover the rationale, design choices, and thoughts on the prototype.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Motivation

When developing any code, being able to test is crucial for sustainable development.

In the case of executable code on systems with GPOS, widely used solutions are GoogleTest or Catch libraries.
What we usually expect from such a framework is:
- a tool that will organize and orchestrate the execution of the tests
- basic functions/API to check the correctness of the results in the test
- features for scaling: fixtures, parameterized tests, executing tests multiple times, metrics

In the context of microprocessors, these libraries are not usable.
They rely on the file system, input/output into a terminal, and dynamic memory. They also do not care about tight limits for code size.

These frameworks are usable only for testing parts of the embedded firmware.
These parts are independent of the hardware: algorithms, internal business logic, and others.
We, however, can't test anything that is tied to the hardware.

For that reason, I decided to implement a custom opinionated testing framework designed for a specific use case: executing tests on the embedded hardware itself.

The goal is to be able to test embedded code that is tied to the hardware itself:
- interrupt-based mechanics
- control algorithms that are unpractical to simulate
- code tied to peripherals

## Requirements

Based on my experience and opinions, I decided to specify the following requirements:

- **emlabcpp Integration**<br>
   The test library is tightly integrated into my personal C++20 library, [emlapcpp](https://github.com/emlab-fi/emlabcpp).
   That is: it can't be used without the library.
   This eases the development of the testing framework as I reuse functionality from the library, specifically: the protocol library inside emlabcpp.

- **Simplicity**<br>
   The library should be simple and should not try to provide the entire set of functionalities that Catch/Google Test offers.

- **Integration into existing testing tools**<br>
   A wide set of tools exist that can work with the test results of Catch/Google Test. For example, GitLab has the integration of test results from these tools.
   The library should be compatible with these integrations.

- **Small footprint**<br>
   Since we are running tests on the device, and the application code itself will take a big percentage of the available memory of the microchip, the library needs to have a small footprint.

- **No dynamic memory. No exceptions.**<br>
   Both are C++ features that we may want to avoid in the firmware since embedded firmware often does not have dynamic memory.

- **No platform requirements**<br>
   The library should not be tied to any specific platform.


## Design

### Library Components

The library itself is implemented as a two-part system:

#### Reactor

The reactor is a module running on the embedded device and controls it to the necessary degree. It has a small footprint and limited functionality.

It can:
- register tests to itself
- store bare minimum information about firmware/tests
- execute the tests
- communicate information/exchange data between itself and the controller

#### Controller

The controller orchestrates the testing process and is run on the device that controls the tests, typically the host machine. It is still developed as microcontroller-compatible software (no dynamic memory, no exceptions), but there is a weak assumption that it will be mainly used on a system with GPOS (general-purpose operating system).

It can:
- communicate with and control the reactor
- load test information from the reactor
- orchestrate test execution
- provide input data for tests
- provide data collected from the tests

### Reactor and Controller Working Together

The separation of the design into two tools imposes restrictions: the tests on the embedded device can't be executed without the controller. 
But that allows a minimal memory footprint of the testing firmware on the firmware size, as I can move as much of the testing logic as reasonable to the controller side.
In particular, data collection can be done within the controller because it has ample storage.

The communication method between the two parts is not defined.
Both parts use messages for communication, but it is up to the user to implement how the messages are transferred.
Each expects to be provided with an interface that implements read/write methods, it's up to the user to design the implementation.
This makes it platform-independent and gives flexibility for various scenarios.
I do expect that a UART will be mostly used primarily to transfer data.

In the end, the perspective one can use for this is:
The testing library is just a fancy remote execution library - the controller executes functions registered to the reactor in the firmware and collects results.

## Basic Implementation Details

Both the reactor and the controller are C++ objects, `testing_reactor` and `testing_controller` respectively. They also expect to be provided with user-supplied objects that implement the interfaces `testing_reactor_interface` and `testing_controller_interface`.
The interfaces are implemented by the user and define how the object interacts with its environment.

In the case of the embedded firmware, one creates an instance of the reactor, registers tests into it, and passes control to the reactor.
This is done in a way that still gives the user some control over the main loop:
```cpp
emlabcpp::testing_basic_reactor rec{"test suite name"};
my_reactor_interface rif{..};
// register tests

while(true){
   rec.tick(rif);
}
```

The reactor expects that its `tick` method is called repeatedly and that the method contains one iteration of the reactor's control loop.
It either answers the reactor in the control loop or executes the entire test during one `tick` call. This means it can block for a while.

The `controller` has similar behavior and interface. With the exception that the `controller_interface` also contains customization points for additional features:
- methods to provide input data for tests on request
- `on_test(emlabcpp::testing_result)` method that is called with results of one test call
- `on_error` method is called once an error happens in the library.

It's up to the user to implement the interface for the specific use case or use existing implementations (the library may provide some).

## Dynamic Memory

Both the `reactor` and the `controller` contain data structures with dynamic size.
To avoid dynamic memory, I wanted to use `std::pmr`: the internal containers would use an allocator and expect memory resource as an input argument.
This implements the behavior: "The central objects expect a memory resource they should use for data allocation."

I think that this fits the use case quite nicely, as both types require dynamic data structures, but in the same way, I want them to be usable without dynamic memory itself. The compromise is an interface that can be provided with static buffers.

However, `std::pmr` does not feel usable, as the default construction of the allocator uses a default memory instance that exists as a global object that can be changed only at runtime.
The default instance uses new/delete.
That means that it is easy for code that uses `std::pmr` to include in the firmware the entire stack for dynamic allocation - something that I want to avoid.

I decided to re-implement `std::pmr` in my custom library with a few changes in the API that are more fitting to the embedded library.
The key change is that memory resource with new/delete operators simply does not exist.
The user has to instantiate a memory resource also provided by `emlabcpp` and give it to the object.

As a simple alternative, there exists `emlabcpp::testing_basic_reactor,` which inherits from the `reactor` and provides it with a basic memory resource that can be used by it. A sane default.

## Binary Protocol

The binary protocol is intentionally considered an implementation detail, as I want to have the freedom to change it at will.

It is implemented with a C++ protocol library I did previously. The short description is: imagine protocol buffers, but instead of an external tool, it is just a C++ library that gets the definition of protocol via templates.

## Data Exchange

The framework provides mechanics to exchange data between the controller and reactor.

Tests running on the embedded device can request test data from the controller as a form of input.
However, it's up to the user how the controller gets/provides that data.
The request is a blocking communication operation - the input is not stored on the side of the reactor.

The controller can also collect data from the embedded device, as reactors have an API to send data to the controller.
The controller stores the data during test execution, and it is passed to the user once the test is done in test_result.

In the case of input, I use a simple key/value mechanism.
In the case of the collected data, these can be organized into a tree, where each node has key/value pair.

That is, each data point is made of a 'key' that identifies it and its corresponding 'value.'

To give some flexibility, the types are:

- **key**<br>
   can be either string or integer

- **value**<br>
   can be string, integer, bool, unsigned

In each case, the framework can serialize (thanks to the `emlabcpp::protocol` library) and deserialize any types and send them over the communication channel.

As for the strings: These are limited by size to 32 characters, as this way, I can use static buffers for them, and they do not have to be allocated.


## Example Test

I tried to prepare a simple interface for the registration of tests, as I believe that tests should be easy to write.
I generally do not mind some cost of setting up the library, but I think that adding tests should be easy. 

To guide the explanation, let's assert we are testing a wending machine:

```cpp
   emlabcpp::testing_basic_reactor rec{"test suite for wending machine"};

   rec.register_callable("my simple test", [&]( emlabcpp::testing_record & rec){

      int product_id = rec.get_arg<int>("product_id");

      rec.expect( product_id < MAX_PRODUCTS_N );

      wending_machine::release_product(product_id);

      rec.collect( "released: ", product_id );

      bool occupied = wending_machine::is_takeout_area_occupied();

      rec.expect( occupied );
   });
```

What happens here is that the lambda function is registered as a test.
That test is identified by the "my simple test" string, used to identify it from the controller.

Once the test is executed (the controller tells the reactor to execute it), it is provided with a `testing_record` object that serves as an API between the test and the reactor.

The testing code should use the record to get any data from the controller, collect any data during the test, and primarily, provide information whenever the test failed or succeeded.

In the example, you can see the usage of all the primitives:
 - `rec.get_arg<int>("product_id")` tells the reactor to ask the controller for an argument with key `product_id` and retreive it as integer type
 - `rec.expect( product_id < MAX_PRODUCTS_N )` is a form checking properties in the test - if `false` is passed to the `expect(bool)` method the test is marked as failed.
 - `rec.collect("released: ", product_id )` collects the data `product_id` with key `released: ` and sends it to the controller.

## Building the Tests

The user handles this. The testing framework just provides an object that expects communication API and can register test - how that is assembled into a firmware is up to the user.

The idea is that single 'testing firmware' will collect multiple tests registered into one reactor.
It's up to the user to orchestrate the build process in a sensible way.

In the case of CMake, I decided to split the application itself into "application library" and "main executable." 
Most of the logic of the firmware is in the application library, and the main executable just implements the main function and starts up the application library.

The main executable of tests uses that library to prepare and set up tests.
Note that the idea is that there are multiple test binaries with different tests.
I don't assume that all the tests would fit into one binary.

This way, any test firmware is closely similar to the application executable - just with a different main file.

From the controller's perspective, it can be just a simple application that is meant to be executed on GPOS.

## Google Test Integration

One small win that appeared was that, given the flexibility, it was easy to integrate Google Test and controller.
The controller can register each test from the reactor as a test in the google test library.
It can use the Google Test facility on GPOS to provide user-readable output about the execution of the tests, more orchestration logic, and output of the testing in the form of JUnit XML files.
Systems like GitLab can use this.

This shows that it was easy to provide the necessary facility for the testing firmware to be integrated into modern CI with traditional tools.
And yet the integration is not tight. Any integration into Google Test is just a set of a few functions/classes in emlabcpp that can be ignored by anybody not favoring Google Test.

The test output from the project I used this framework the first time can look like this:

```
    ./cmake-build-debug/util/tester --device /dev/ttyACM0
   [==========] Running 1 test from 1 test suite.
   [----------] Global test environment set-up.
   [----------] 1 test from emlabcpp::testing
   [ RUN      ] emlabcpp::testing.basic_control_test
   /home/veverak/Projects/servio/util/src/tester.cpp:32: Failure
   Test produced a failure, stopping
   collected:
    11576 :	0
    11679 :	0
    11782 :	0
    11885 :	0
    11988 :	0
    12091 :	0
    12194 :	0
    12297 :	0
    12400 :	0
    12503 :	0
    12606 :	0
    12709 :	0
    12812 :	0
    12915 :	0
    13018 :	0
    13121 :	0
    13224 :	0
    13327 :	0
    13430 :	0
    13533 :	0
    13636 :	0
    13739 :	0
    13842 :	0
    13945 :	0
    14048 :	0
   [  FAILED  ] emlabcpp::testing.basic_control_test (2597 ms)
   [----------] 1 test from emlabcpp::testing (2597 ms total)

   [----------] Global test environment tear-down
   [==========] 1 test from 1 test suite ran. (2597 ms total)
   [  PASSED  ] 0 tests.
   [  FAILED  ] 1 test, listed below:
   [  FAILED  ] emlabcpp::testing.basic_control_test

    1 FAILED TEST
```

In this example, the controller registered all tests in the firmware (on the device that was connected to the PC and was accessible via the `/dev/ttyACM0` serial device). After that, it executed all of them.

The name of the testing suite `emlabcpp::testing` and the name of the test `basic_control_test` were all extracted on the fly from the testing firmware itself. We can also see values collected by the test during the execution.

## Controller is independent

Based on the specific project and testing needs, one can use one binary with a `controller` for multiple `reactors.` That is something I intend with the actual main project that uses it.

As the controller loads most information from the reactor and if the Google Test integration is used, there is not much logic that can be varied.

The sole exception is how data is provided for the tests.
But then it can be implemented in some general way - for example, the `controller` binary would load the data from a JSON file in some generic way.

## Experience

It is pretty limited, but I am happy with the prototype.
I am sure that I will refactor the library in the future, as there are prominent places to be improved but so far it behaves well enough.
It gives me a simple way to test and develop various ways to control the smart servomotor I am working on.
(Note: yes, this is one of the cases of "oh, I need to develop a library so I can do a project"...)

What could be developed more in the future and what pains me so far is:
 - it still does not report 100% of the possible errors on the side of the testing library - I have to go through the codebase and be more strict
 - it can't handle exceptions - while it should not rely on them, I think the library should respect them. That means in case the test throws an exception. It should not stop the reactor.
 - data exchange can be improved - what can be exchanged as of now is quite limited. I suppose I can provide more types to send and receive.
 - memory resource - uses internal emlabcpp mechanism that is underdeveloped, that definitely would benefit from more work.
 - more experience in CI - I think I am on a good track to having automatized tests in CI that are flashed to real hardware somewhere in the laboratory. That could show the limits of the library.

## The Code

The testing library is part of emlabcpp - my personal library, which purpose is for me to have an up-to-date collection of tools for my development. 
I restrain myself from saying "it should be used by others," as I don't want to care about backward compatibility outside of my projects.

The primary example of the testing library is an example file: [emlabcpp/tests/testing](https://github.com/koniarik/emlabcpp/blob/v1.2/tests/testing_test.cpp)

The interface to the library itself is in: [emlabcpp/include/emlabcpp/experimental/testing](https://github.com/koniarik/emlabcpp/tree/v1.2/include/emlabcpp/experimental/testing)
