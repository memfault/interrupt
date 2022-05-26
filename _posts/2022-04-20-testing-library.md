---
title: Testing library
description: Design of a testing library
author: veverak
tags: [c++, testing]
---

<!-- excerpt start -->

We have multiple testing library focused on desktop C++ aplications, but there is a lack of library designed for embedded devices.

The traditional libraries are not designed for constrained resources and rely on functionality like a filesystem or standard output.

I decided to design a testing library for microcontrollers.
In this article I want to show rationale, design choices, and thoughts on the prototype.

<!-- excerpt end --->

{% include newsletter.html %}

{% include toc.html %}

## Motivation

When developing any code, being able to test is crucial for sustainable development.

In the case of code that is executable on systems with OS, widely used solutions are GoogleTest or Catch libraries frameworks.
What we usually expect from such a framework is:
- a tool that will organize and orchestrate the execution of the tests
- basic functions/API to check the corretness of the results in the test
- features for scaling: fixtures, parameterized tests, executing tests multiple times, metrics

In the context of microprocessosrs, these libraries are not usable.
They rely on the file system, input/output into a terminal, dynamic memory, and do not care about tight limits for code size.

These frameworks are usable only for testing of  the embedded firmware.
These parts are independent on the hardware: algorithms, internal business logic etc..
We, however, can't test anything that is tied to the hardware.

For that reason, I decided to implement a custom opinionated testing framework designed for a specific use case: executing tests on the embedded hardware itself.

The goal is to be able to test embedded code that is tied to the hardware itself:
- interrupt-based mechanics
- control algorithms that are unpractical to simulate
- code tied to peripherals

## Requirements

Based on my experience and opinions, I decided to specify the following requirements:

emlabcpp integration
   The code is tightly integrated into an existing C++20 library that I am developing. 
   That is: it can't be used without the library.
   This eases development of the testing framework as I reuse functionality from the library, specifically: protocol library.

simplicity
   The library should be simple and should not try to provide entire set of functionality that catch/gtest provides.
   That should not be necessary and I prefer simpler and more effective tool.

integration into existing testing tools
   Wide set of tools exist that can work with test results of catch/gtest - for example gitlab has integration of test results from these tools.
   The library should be compatible from this perspective - it should be integrable into existing systems.

small footprint
   The assertion is that a big percentage of available memory of the microchip will be taken by the application code itself.
   That implies that the library should have small memory footprint - so it can coexist with present code.

no dyn. memory, no exception
   Both are C++ features which we may want to avoid in the firmware.
   The testing library should not require them for it's functionality, to allow usage in context when they are not enabled at all.

no platform fixation
   Ideally, we would prefer this to be reusable between different embedded platforms and situations.
   That imposes the limit that the library should not be tied to any specific platform.


## Design

The library itself is implemented as a two part system:

reactor
   Is present in the embedded device, and controls it.
   It has small footprint and has limited functionality, it can:
      - register tests to itself
      - store bare minimum information about firmware/tests
      - execute the tests
      - communicate information/exchange data between itself and controller

controller
   Controls the testing process and is presented on the device that controls the tests.
   It is still developet as microcontroller compatible software, but there si weak assumption that it will be mostly used on PC.

   It can:
      - communicate with and control the reactor
      - load test information from reactor
      - orchestrate test execution
      - provide input data for tests
      - provide data collected from the tests

The separation of the design into two tools impose restriction: the tests on the embedded device can't be executed without the controller. 
But that allows really small footprint of the testing firmware on the firmware size, as I can move as much of the testing logic as reasonable to the controller side.
Especially data collection can be done in a way that nothing is stored in the reactor itself.

The communication method between the parts is not defined.
Both parts use messages for communication, but is up to the user to implement how the messages are transfered.
Each expect to be provided with an interface that implements read/write methods - it's up to the user to design how.
This makes it platform independent and gives flexibility for various scenarios.
But I do silently expect that UART will be mostly used.

The way the controller gets input data and processes the collected data from tests is up to the user.
The interface for controller just provides an API for both.

In the end, the perspective one can use for this is:
The testing library is just fancy remote execution library - the controller executes functions registered to reactor in the firmware and collects result.

## Basic implementation details

Each part is object - `testing_reactor` object and `testing_controller` object.
Both are designed to take control of their application and both expect to be provided with appropriate interface `testing_reactor_interface` and `testing_controller_interface`.
Interfaces are designed/selected by the user and define how the object interacts with it's environment.

In case of the embedded firmware, one creates instance of the reactor, registers tests into it and passes control to the reactor.
This is done in a way that still gives user some control over the main loop::
```cpp
   emlabcpp::testing_basic_reactor rec{"test suite name"};
   my_reactor_interface rif{..};
   // register tests

   while(true){
      rec.tick(rif);
   }
```

The reactor expects that it's `tick` method is called repeatedly and the method contains one iteration of reactors control loop.
It either answers the reactor in the control loop or actually executes entire test during one `tick` call - it can block for a while.

`controller` has similar beavior and interface. With the exception that the `controller_interface` also contains customization points for additional features:
- methods to provide input data for tests on request
- `on_test(emlabcpp::testing_result)` method that is called with results of one test call
- `on_error` method that is called once error happens in the library.

It's up to the user to implement the interface for the specific use case or to use existing integration in the library.

## Dynamic memory

Both the `reactor` and the `controller` contains data structure with dynamic size.
To avoid dynamic memory, I wanted to use `std::pmr`: that is, that the internal containers would use allocator and expects memory resource as an input argument.
This implements the behavior: "the central objects expect a memory resource they should use for allocation of adata".

I think that this fits the use case quite nicely, as both types require dynamic data structures but at the same way I want them to be usable without dynamic memory itself - compromise is interface that can be provided with static buffers.

However `std::pmr` does not feel usable, as the default construction of allocator uses a default memory instance that exists as a global object. (that can be changed only at runtime)
The default instance uses new/delete.
That means that it is easy for code that uses `std::pmr` to include in the firmware entire stack for dynamic allocation - something that I want to avoid.

Given that I implemented custom allocator/memory_resource concept that mirrors the wanted behavior but avoids the problem with default instance.
That means that to use the objects, user has to instance a memory resource also provided by `emlabcpp` and give it to the object.

To ease usage, there exists `emlabcpp::testing_basic_reactor` which inherits from the `reactor` and provdies it with basic memory resource that can be used by it - sane default.

## Binary protocol

The binary protocol is intetionally considered an implementation detail, as I want to have freedom to change it at will.

It is implemented with a protocol library I did previously in C++. The short description is: imagine protocol buffers, but instead of external tool it is just C++ library that gets definition of protocol via templates.

## Data exchange

The framework provides mechanic to exchange data between controller and reactor.

Tests can request test data from the controller as an form of input.
(It's up to the user how controller gets/provides that data)
The request is a blocking communication operation - the input is not stored on the side of reactor.

The test can collect data - reactors has an API to send data to the controller.
The controller stores the data during test execution and it is passed to the user once test is done in test_result.

In both cases, I use only simple key/value mechanism.
That is each data point is made of 'key' that identifies it and corresponding 'value'.

To give some flexibility, the types are:

key
   can be either string or integer

value
   can be string, integer, bool, unsigned

In each case, the framework is able to serialize (thanks to `emlabcpp::protocol` library) and deserialize any of the types and send them over the communication channel.

As for the strings: These are limited by size to 32 characters, as this way I can use static buffers for them and they do not have to be allocated.


## Examples of tests

I tried to prepare a simple interface for the registration of tests, as may general assumption is that the tests should be easy to write.
(Note: Generally I don't mind some cost on setting up the library, but I think that adding test should be easy)
To guide the explanation let's assert we are testing wending machine:

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

What happens here is that lambda function is registered as an test.
That test is identified by "my simple test" and that is used to identify it from controller.

Once the test is executed (that is: controller tells the reactor to execute it), it is provided with `testing_record` object that serves as an API between the test and the reactor.

The testing code should use the record to get any data from controller, collect any data during the test and mainly: to provide information whenever the test failed or succceeded.

In the example you can see usage of all the primivites:
 - `rec.get_arg<int>("product_id")` tells the reactor to ask controller for argument with key `product_id` and retreive it as integer type
 - `rec.expect( product_id < MAX_PRODUCTS_N )` is a form checking properties in the test - in any moment if `false` is passed to the `expect(bool)` method the test is marked as failed.
 - `rec.collect("released: ", product_id )` collects the data `product_id` with key `released: ` and sends it to the controller.

## Building the tests

That is solely handled by the user, the testing framework just provides a object that expects communication API and can register test - how that is assembled into a firmware is up to the user.

The idea is that single 'testing firmware' will be a collection of multiple tests registered into one reactor.
It's up to the user to orchestrate the build process in a way that this is sensible.

In case of CMake, I decided to split the application itself into "application library" and "main executable". 
That is, most of the logic of the firmware is in the application library and the main executable just implements main function and starts up the application library.

The main executable of tests uses that library to prepare and setup tests.
Note that the idea is that there are multiple test binaries with different tests, I don't assume that all the tests would fit into one binary.

This way, any test firmware is closely similar to the application executable - just with different main file.

## Google Test

One small win that appeared was that given the flexibility, it was easy to integrate gtest and controller together.
That is, the controller can register each test from reactor as a test in the google test library.
Tt can use the gtest facillity on PC to provide user-readable output about execution of the tests, more orchestration logic and output of the testing in form of JUnit XML files.
These can be used by tools like gitlab to provide test results in it's GUI.

What this means? that it was easy to provide necessary facility for the testing firmware to be integrated into modern CI with traditional tools.
And yet the integration is not tight, any integration into gtest is just a set of few functions/classes in emlabcpp t hat can be ignored for anybody not favoring gtest.

Test output from the project I used this framework first time can look like this:

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

In this example, the controller registered all tests that were in the firmware (on device that was connected to the PC and was accessible via the `/dev/ttyACM0` serial device) as google tests they were executed.

The name of the testing suite `emlabcpp::testing`, name of the test `basic_control_test` we all collected on the fly from the testing firmware itself, we can also see values collected by the test during the execution.

## Controller is independent

Based on the specific project and testing needs, one can use one binary with `controller` for multiple `reactors` , that is something I intend with actuall main project that uses it.

As the controller loads most information from the reactor and in case the gtest integration is used there is not much of the logic that can be varied.

Sole exception is how data is provided for the tests.
But than that again can be implemented in some general way - for example that the `controller` binary would load the data from json file in some generic way.

## Experience

Is quite limited so far, but I am happy with the first prototype.
I am sure that I will refactor the library in the future, as there are obvious places to be improved but so far it behaves good enough.
It gives me a simple way to test and develop various ways to control smart servomotor I am working on.
(Note: yes, this is one of the cases of "oh, I need to develop a library so I can do a project"...)

What could be developed more in the future and what pains me so far is:
 - it still does not report 100% of the possible errors on the side of the testing library - I have to go throu the codebase and be more strict
 - it can't handle exceptions - while it should not rely on them, I think the library should respect them, that means in case test throws exception it should not stop the reactor
 - data exchange can be improved - what can be exchanged as of now is quite limited, I suppose I can provide more types to send and receive
 - memory resource - use internal emlabcpp mechanism that is underdeveloped, that definetly would benefit from more work 
 - more experience in CI - I think I am on a good track to have automatized test in CI that are flashed to real hardware somewhere in laboratory. That could show limits of the library 
 - `get_arg<int>(` is an example of interface of test library that can result in an error. I don't want to move introduce an change in the API that would make it return error as I can't figure out anything that would not pollute the tests. The idea is that for the errors under category "error in processing of the test and not the test" that can't be handled the library: A. throws exception if possible B. stops at the point and spams the controller with error message. 

## The code

The testing library is part of emlabcpp - my personal library, which purpose is mostly for me to have up-to-date collection of tools for my development. 
Given that I restrain myself from saying "it should be used by others" as I don't really want to care about backward compability outside of my projects.

The primary example of the testing library is an example file: [emlabcpp/tests/testing](https://github.com/koniarik/emlabcpp/blob/v1.2/tests/testing_test.cpp)

The interface to the library itself is in: [emlabcpp/include/emlabcpp/experimental/testing](https://github.com/koniarik/emlabcpp/tree/v1.2/include/emlabcpp/experimental/testing)
