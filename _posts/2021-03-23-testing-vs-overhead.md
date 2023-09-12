---
title: Balancing Test Coverage vs. Overhead
author: erikfogg
tags: [testing, unit-testing]
---

> This article is written by an external contributor, [Erik Fogg](authors/erikfogg) of [ProdPerfect](https://prodperfect.com/), along with Tyler Hoffman representing the firmware side of the story. Enjoy!

<!-- excerpt start -->

In a perfect world, all software and firmware are given precisely the time and budget it needs to be successful, code is uniformly well-written to industry best practices, and the code is complemented with a complete test suite instrumenting all aspects of the software.

In practice, this rarely, if ever, happens! Development teams and organizations continually have to re-prioritize tasks to meet deadlines and avoid going over budget. In this article, we'll cover how you can think about rapidly modifying and updating firmware, testing these changes, all the while allowing you to get the product out the door.

<!-- excerpt end -->

If you are working at a hardware company right now, which I assume many of you are, you are **undeniably struggling** to procure various hardware components. We've heard that prices of STM32's have sky-rocketed from $1-2 to $10-20! While rapidly modifying and updating the firmware to accommodate different hardware components, a balance between testing and getting s*** out the door needs to be found. We'll dive into some ways that might help you.

{% include newsletter.html %}

{% include toc.html %}

## What is Test Coverage?

Test coverage refers to how extensively software is covered by its test suite. Exactly how extensive a test suite should be is a matter of debate within software testing. For example, a comprehensive test suite would not only test each component in a codebase but would also cover branch conditions and ideally every possible configuration of conditionals. As you can imagine, the number of test cases would be enormous!

Tests can also take forms outside of directly testing the codebase, such as end-to-end (E2E) testing. This involves testing the entire process of using software from an end-user's perspective, from the very beginning to the very end of the end-user experience. If your end-product that your customers use is a set of API endpoints, then your E2E tests should use the API endpoints directly with the standard authentication methods that your app expects.

For hardware and firmware, end-to-end tests would likely be initiated through an [on-device shell or CLI]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}) through the UART or over USB. The E2E test result could be determined by validating the responses from the CLI command or by checking the environment after the test case, such as ensuring a file exists or flash region is written with a particular value.

## Why Test Coverage is Important in Software Testing

Test coverage is important for several different reasons in software engineering. For certain software products, such as software requiring certification like in aviation, rigorous test coverage is mandated. In other disciplines where the safety of others isn't at stake, it is seen as a best practice to provide a level of certainty as to how a piece of software will perform before it is released to the public.

Releasing a buggy product can quickly damage a software company’s reputation or ruin a product. In the world of firmware and hardware, a buggy release might ultimately turn into a bricked device if the firmware update flow is broken.

## What is Code Coverage

Code coverage refers to how much of a codebase is covered by tests. Below is an example output from LCOV[^lcov], a popular tool for testing code coverage.

<p align="center">
    <img width="600" src="{% img_url testing-vs-overhead/lcov.png %}"/>
</p>

A code coverage report will provide you with information about the number of units of code (and percentage of total units) that have been run by the tests as well as point out exactly which units haven't been tested. This makes it easy to find the branches of code that have not been tested at all.

[Unit testing]({% post_url 2019-10-08-unit-testing-basics %}) is the most common method developers use to test code. Unit tests are tests that are written to instrument a single unit of code, such as a collection of methods inside a class or module. Each method in a class will have several tests written against it, which provide a series of different inputs and assert an expected output. How many units of code are covered by tests determines the percentage of code coverage.

> To learn how to add code coverage reporting to your embedded projects, you can check out the instructions [here]({% post_url 2019-10-08-unit-testing-basics %}#code-coverage).

Certain programming paradigms are easier to test than others. Functional programming is particularly easy to test, as each function is intended to be idempotent and can be called in isolation. However, many other times, code is not as clean and easy to test. Certain functions may expect certain environmental configurations or dependency functions to be compiled in. They may be tightly coupled to other modules or methods and may expect to be passed results from external sources, such as a database connection, API result, file on the filesystem, or external sensor.

These expected inputs need to be [mocked]({% post_url 2020-05-12-unit-test-mocking %}) and other special conditions must be taken into consideration, all of which take up valuable developer's time and company resources.

## Test Coverage vs Code Coverage

Test coverage refers to the wider scope of testing and how much of a product is tested. This includes not just the underlying code but how the user experiences it. UI/UX testing across different devices and platforms, user story testing, and E2E testing all fall under the umbrella of test coverage.

Thus, it is sometimes advocated that 100% *test coverage* in tests is not necessarily desirable or is seen as a misuse of available time and energy. Development teams need to balance testing with other demands, such as regularly updating the software to add new features to attract more customers, improve reviews and ratings, and prolong the life of the device.

On the contrary, it is generally desirable to reach 100% *code coverage*, but it can not be confused with 100% *test coverage*. Code coverage only points out the percentage of code units that were cycling over when running a single or a suite of tests. It can not be confused with testing every branch and set of arguments, which would be *test coverage*.

## How to Reduce Coverage Testing Overhead

Testing is costly in terms of the time it takes to develop, maintain, and ultimately run the tests. How long it takes to run an extensive test suite can fluctuate wildly depending on the software and the tests involved. To add to this, regression testing advocates for testing each new change against the entire existing codebase to ensure that no new bugs are introduced. When developing using a methodology that advocates for the continuous integration of small, rapidly developed commits, an extensive suite can quickly develop into a testing bottleneck.

Several different methods can be used to mitigate this and reduce the overhead of extensive test coverage. The tests themselves could be analyzed and refactored to reduce the time it takes for them to run as much as possible. Some tests will unavoidably take longer to run than others though, so certain groups of tests can be placed into categories so that they are not run as frequently as other tests or at all under certain conditions.

Other options, such as continuous testing and parallel testing, are available. These methods either run your tests as you write them or run your test suite in parallel, so you do not have to wait for some of the longer tests to finish before moving on to other tests. Both methods deliver results as they happen, cutting down on the waiting time required when running tests, which can be on the order of magnitude of hours to days for sufficiently mature regression tests run in series.

## How Much Testing Is Too Much?

There is an argument that chasing 100% code coverage is a fool’s errand. This is because development teams have limited resources and must distribute them in the most efficient manner possible to develop the product and remain profitable. How severe a bug adversely affects a product is also heavily dependent on the type of bug, the area of the application the bug might appear in, and the target market of the product in question. A bug with the UI in a rarely used section of a UI could be overlooked by many users, but a bug that results in a connected battery to ignite or the firmware to reboot at an inopportune time could be catastrophic. Thus, it is important to triage errors and prioritize test coverage effectively, given the time and resources available.

## Tips To Maximize Testing Coverage/Efficient Testing

All of this testing takes time. Maximizing available testing resources effectively is generally done by categorizing potential bugs. This can be done in a number of ways.

### Categorizing Test Cases

One such way would be by *feature usage*: features that are used more commonly and extensively should receive a greater proportion of testing time and resources than other features.

Another categorization is by *risk*, or how severely a bug impacts a product or end-user. In the example above, a bug in the battery driver that could potentially lead to a failure would be a high-risk bug and should receive much more time and resources than a simple, low-risk UI bug of little consequence.

*Feature criticality* is another category for test development. This involves testing features in order of how critical they are for the product to work successfully. If a feature breaking has little impact on the user experience, then it should receive fewer resources than a feature that is integral to the user experience.

Two parts of the codebase that should be rigorously tested no matter what are [firmware update]({% post_url 2020-06-23-device-firmware-update-cookbook%}) and factory reset. Firmware update will allow your devices to update from one firmware to another, and factory reset will bring the device back to the state it was in when it was first provisioned. Factory reset will typically involve zeroing external flash memory, deleting all user preferences, removing bonding information, wiping the filesystem, and ideally loading the original firmware that the device started with. The only things that should remain between factory resets are the device provisioning information and maybe some logs to determine what caused the factory reset in the first place.

### Testing Without Hardware

Testing with hardware-in-the-loop is a necessary evil of device manufacturers, but it can be done only for a few basic test cases, such as firmware update, factory reset, power monitoring, and basic hardware functionality. Beyond that, most of the firmware can be tested without hardware with either unit tests or emulators! Emulators, such as Renode and QEMU, provide a perfect platform to run tests on with mostly-original firmware images. Emulators don't require teams to purchase extra hardware, they scale infinitely, you'll experience fewer hardware test flakes, and the performance of hardware does not degrade over time.

In today's silicon shortage, emulators also provide a team with a way to start or continue building firmware without hardware in hand. For example, the Zephyr RTOS[^zephyr] has support for the Cortex-M0 and Cortex-M3 MCU's in QEMU, which should provide a solid platform to start building some applications that run within the Zephyr environment.

The Memfault team is also a big fan of Renode[^renode] for MCU emulation ([quick start guide]({% post_url 2020-03-23-intro-to-renode %})), and it even has support for the nRF52 and STM32F4. Emulators really shine when they are hooked up to continuous integration systems and tests are run with them in an [automated fashion]({% post_url 2020-07-07-test-automation-renode %}).

### Use Unit Tests Where Possible

Unit tests are the fastest way to test a single module of code and prevent it from regressing through constant modifications and refactoring. Unit tests generally run on the host machine and will run in seconds or minutes, providing developers with the quickest build-deploy-test cycles. Use them wherever possible and when a proper integration test combining multiple modules or hardware isn't required.

## Benefits of Balancing Coverage Testing & Overhead

The ‘iron triangle’ of software development involves balancing the allocation of resources to competing needs within a project by a set of criteria, such as speed, priority, and cost. Companies too stuck inside this paradigm may consider that too much testing entails too high a cost or slows down product development. However, it’s not necessarily the case that time and resources devoted to testing are time and resources sacrificed elsewhere.

Smart test coverage involves analyzing a project and identifying what areas to prioritize in testing. This is highly project dependent, and 100% code coverage is not always necessarily the best possible option. Understanding where and what the risks are in software enables teams to develop sufficient test coverage to get the software to market without sacrificing time spent on new features or running over budget.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^lcov]: [LCOV](http://ltp.sourceforge.net/coverage/lcov.php)
[^zephyr]: [Zephyr RTOS](https://docs.zephyrproject.org/)
[^renode]: [Renode](https://renode.io/)
<!-- prettier-ignore-end -->
