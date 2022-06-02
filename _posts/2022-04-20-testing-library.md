---
title: Testing library
description: Design of a testing library
author: veverak
tags: [c++, testing]
---
<!-- excerpt start -->

We have multiple testing libraries focused on C++ applications for GPOS (general-purpose operating system), but there is a lack of testing libraries designed for embedded devices.

The traditional libraries are not designed for constrained resources and rely on functionality like a filesystem or standard output.

I decided to design a testing library for microcontrollers.
In this article, I want to show the rationale, design choices, and thoughts on the prototype.

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

emlabcpp integration
   The code is tightly integrated into my personal C++20 library.
   That is: it can't be used without the library.
   This eases the development of the testing framework as I reuse functionality from the library, specifically: the protocol library inside emlabcpp.

simplicity
   The library should be simple and should not try to provide an entire set of functionality that Catch/Google Test offers.
   That should not be necessary, and I prefer a simpler and more efficient tool.

integration into existing testing tools
   A wide set of tools exist that can work with the test results of Catch/Google Test - for example, GitLab has the integration of test results from these tools.
   The library should be compatible from this perspective - it should be integrable into existing systems.

small footprint
   The assertion is that the application code itself will take a big percentage of the available memory of the microchip.
   That implies that the library should have a small memory footprint - so it can coexist with present code.

no dynamic memory, no exception
   Both are C++ features that we may want to avoid in the firmware.
   The testing library should not require them for its functionality to allow usage in context when they are not enabled at all.

no platform fixation
   Ideally, we would prefer this to be reusable between different embedded platforms and situations.
   That imposes the limit that the library should not be tied to any specific platform.


## Design

The library itself is implemented as a two-part system:

reactor
   It is present in the embedded device and controls it.
   It has a small footprint and limited functionality. It can:
      - register tests to itself
      - store bare minimum information about firmware/tests
      - execute the tests
      - communicate information/exchange data between itself and the controller

controller
   Controls the testing process and is presented on the device that controls the tests.
   It is still developed as microcontroller-compatible software (no dynamic memory, no exceptions), but there is a weak assumption that it will be mainly used on a system with GPOS.
   It can:
      - communicate with and control the reactor
      - load test information from the reactor
      - orchestrate test execution
      - provide input data for tests
      - provide data collected from the tests

The separation of the design into two tools imposes restrictions: the tests on the embedded device can't be executed without the controller. 
But that allows a minimal memory footprint of the testing firmware on the firmware size, as I can move as much of the testing logic as reasonable to the controller side.
Especially data collection can be done so that everything is stored in the controller.

The communication method between the parts is not defined.
Both parts use messages for communication, but it is up to the user to implement how the messages are transferred.
Each expects to be provided with an interface that implements read/write methods - it's up to the user to design how.
This makes it platform-independent and gives flexibility for various scenarios.
But I do silently expect that UART will be mostly used.

The way the controller gets input data and processes the collected data from tests is up to the user.
The interface for the controller only provides an API for both.

In the end, the perspective one can use for this is:
The testing library is just a fancy remote execution library - the controller executes functions registered to the reactor in the firmware and collects results.

## Basic implementation details

Each part is object - `testing_reactor` object and `testing_controller` object.
Both are designed to take control of their application.
Both expect to be provided with user-provided objects implementing interfaces `testing_reactor_interface` and `testing_controller_interface`.
Interfaces are implemented by the user and define how the object interacts with its environment.

In the case of the embedded firmware, one creates an instance of the reactor, registers tests into it, and passes control to the reactor.
This is done in a way that still gives user some control over the main loop::
```cpp
   emlabcpp::testing_basic_reactor rec{"test suite name"};
   my_reactor_interface rif{..};
   // register tests

   while(true){
      rec.tick(rif);
   }
```

The reactor expects that its `tick` method is called repeatedly, and the method contains one iteration of the reactor's control loop.
It either answers the reactor in the control loop or executes the entire test during one `tick` call - it can block for a while.

The `controller` has similar behavior and interface. With the exception that the `controller_interface` also contains customization points for additional features:
- methods to provide input data for tests on request
- `on_test(emlabcpp::testing_result)` method that is called with results of one test call
- `on_error` method is called once an error happens in the library.

It's up to the user to implement the interface for the specific use case or use existing implementations (the library may provide some).

## Dynamic memory

Both the `reactor` and the `controller` contains data structure with dynamic size.
To avoid dynamic memory, I wanted to use `std::pmr`: the internal containers would use an allocator and expect memory resource as an input argument.
This implements the behavior: "The central objects expect a memory resource they should use for data allocation."

I think that this fits the use case quite nicely, as both types require dynamic data structures, but in the same way, I want them to be usable without dynamic memory itself - compromise is an interface that can be provided with static buffers.

However, `std::pmr` does not feel usable, as the default construction of the allocator uses a default memory instance that exists as a global object. (that can be changed only at runtime)
The default instance uses new/delete.
That means that it is easy for code that uses `std::pmr` to include in the firmware the entire stack for dynamic allocation - something that I want to avoid.

I decided to re-implement `std::pmr` in my custom library with a few changes in the API that are more fitting to the embedded library.
The key one is that memory resource with new/delete operators simply does not exists.
The user has to instance a memory resource also provided by `emlabcpp` and give it to the object.

As a simple alternative, there exists `emlabcpp::testing_basic_reactor,` which inherits from the `reactor` and provides it with a basic memory resource that can be used by it - sane default.

## Binary protocol

The binary protocol is intentionally considered an implementation detail, as I want to have the freedom to change it at will.

It is implemented with a C++ protocol library I did previously. The short description is: imagine protocol buffers, but instead of an external tool, it is just a C++ library that gets the definition of protocol via templates.

## Data exchange

The framework provides mechanics to exchange data between controller and reactor.

Tests can request test data from the controller as a form of input.
(It's up to the user how the controller gets/provides that data)
The request is a blocking communication operation - the input is not stored on the side of the reactor.

The test can collect data - reactors have an API to send data to the controller.
The controller stores the data during test execution, and it is passed to the user once the test is done in test_result.

In the case of input, I use a simple key/value mechanism.
In the case of the collected data, these can be organized into a tree, where each node has key/value pair.

That is, each data point is made of a 'key' that identifies it and its corresponding 'value.'

To give some flexibility, the types are:

key
   can be either string or integer

value
   can be string, integer, bool, unsigned

In each case, the framework can serialize (thanks to the `emlabcpp::protocol` library) and deserialize any types and send them over the communication channel.

As for the strings: These are limited by size to 32 characters, as this way, I can use static buffers for them, and they do not have to be allocated.


## Examples of tests

I tried to prepare a simple interface for the registration of tests, as I believe that tests should be easy to write.
(Note: Generally, I don't mind some cost of setting up the library, but I think that adding tests should be easy)
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

The testing code should use the record to get any data from the controller, collect any data during the test, and mainly: provide information whenever the test failed or succeeded.

In the example, you can see the usage of all the primitives:
 - `rec.get_arg<int>("product_id")` tells the reactor to ask controller for argument with key `product_id` and retreive it as integer type
 - `rec.expect( product_id < MAX_PRODUCTS_N )` is a form checking properties in the test - if `false` is passed to the `expect(bool)` method the test is marked as failed.
 - `rec.collect("released: ", product_id )` collects the data `product_id` with key `released: ` and sends it to the controller.

## Building the tests

The user solely handles that. The testing framework just provides an object that expects communication API and can register test - how that is assembled into a firmware is up to the user.

The idea is that single 'testing firmware' will collect multiple tests registered into one reactor.
It's up to the user to orchestrate the build process in a sensible way.

In the case of CMake, I decided to split the application itself into "application library" and "main executable." 
Most of the logic of the firmware is in the application library, and the main executable just implements the main function and starts up the application library.

The main executable of tests uses that library to prepare and set up tests.
Note that the idea is that there are multiple test binaries with different tests.
I don't assume that all the tests would fit into one binary.

This way, any test firmware is closely similar to the application executable - just with a different main file.

From the controller's perspective, it can be just a simple application that is meant to be executed on GPOS.

## Google Test

One small win that appeared was that, given the flexibility, it was easy to integrate Google Test and controller.
The controller can register each test from the reactor as a test in the google test library.
It can use the Google Test facility on GPOS to provide user-readable output about the execution of the tests, more orchestration logic, and output of the testing in the form of JUnit XML files.
Systems like GitLab can use this.

This shows that it was easy to provide the necessary facility for the testing firmware to be integrated into modern CI with traditional tools.
And yet the integration is not tight. Any integration into Google Test is just a set of few functions/classes in emlabcpp that can be ignored by anybody not favoring Google Test.

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
I am sure that I will refactor the library in the future, as there are prominent places to be improved but so far it behaves good enough.
It gives me a simple way to test and develop various ways to control the smart servomotor I am working on.
(Note: yes, this is one of the cases of "oh, I need to develop a library so I can do a project"...)

What could be developed more in the future and what pains me so far is:
 - it still does not report 100% of the possible errors on the side of the testing library - I have to go through the codebase and be more strict
 - it can't handle exceptions - while it should not rely on them, I think the library should respect them. That means in case the test throws an exception. It should not stop the reactor.
 - data exchange can be improved - what can be exchanged as of now is quite limited. I suppose I can provide more types to send and receive.
 - memory resource - uses internal emlabcpp mechanism that is underdeveloped, that definitely would benefit from more work.
 - more experience in CI - I think I am on a good track to having automatized tests in CI that are flashed to real hardware somewhere in the laboratory. That could show the limits of the library.

## The code

The testing library is part of emlabcpp - my personal library, which purpose is for me to have an up-to-date collection of tools for my development. 
I restrain myself from saying "it should be used by others," as I don't want to care about backward compatibility outside of my projects.

The primary example of the testing library is an example file: [emlabcpp/tests/testing](https://github.com/koniarik/emlabcpp/blob/v1.2/tests/testing_test.cpp)

The interface to the library itself is in: [emlabcpp/include/emlabcpp/experimental/testing](https://github.com/koniarik/emlabcpp/tree/v1.2/include/emlabcpp/experimental/testing)
