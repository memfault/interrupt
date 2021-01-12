---
title: Balancing Test Coverage vs. Overhead
author: erikfogg
---

<!-- excerpt start -->

In a perfect world, all software is given precisely the time and budget it needs to thrive. Code is uniformly well-written to industry best practices, with a complete test suite covering all code. In practice, development teams continually have to prioritize and balance tasks in order to meet deadlines and avoid going over budget. These trade-offs can take many different forms in software development, but an unfortunate truth is that test coverage (and testing in general) is often among the first things on the chopping block. 


<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What is test coverage?

Test coverage refers to how extensively software is covered by its test suite. Exactly how extensive a test suite should be is a matter of debate within software testing. For example, a comprehensive test suite would not only test each component in a codebase but would also cover branch conditions and (ideally) every possible configuration of conditionals.

Tests can also take forms outside of directly testing the codebase, such as end to end (E2E) testing. This involves testing the entire process of using software for an end-user, from the very beginning to the very end of the end-user experience.

## Why test coverage is important in software testing

Test coverage is important for a number of different reasons in software engineering. For certain software products, such as software requiring certification like in aviation, it is mandated. In other disciplines, it is seen as a best practice in order to provide a level of certainty as to how a piece of software will perform before it is released to the public. Releasing a buggy product can quickly damage a software company’s reputation or ruin a product, so test coverage is important to ensure that all areas of a piece of software work as expected.

## Test coverage vs code coverage 

Code coverage refers to how much of a codebase is covered by tests. Unit testing is the most common method used to test code. Unit tests are tests that are written against a particular unit of code, such as a collection of methods inside a class. Each method in a class will have several tests written against it, which provide a series of different inputs and assert an expected output. How many units of code are covered by tests determines the percentage of code coverage.

Certain programming paradigms are easier to test than others. Functional programming is particularly easy to test, as each function is intended to be idempotent and can be called in isolation. However, many other times, code is not as clean and easy to test. Certain class methods may expect certain environmental configurations. They may be tightly coupled to other classes or methods and may expect to be passed results from external resources, such as a database connection or API result. These expected inputs then need to be mocked, and other special conditions must be taken into consideration, all of which take up valuable time and money.

Test coverage refers to the wider scope of testing and how much of a product is tested. This includes not just the underlying code but how the user experiences it. UI/UX and front-end testing across different devices and platforms, user story testing, and E2E testing all fall under this umbrella. 

Thus, it is sometimes advocated that 100% test coverage in tests is not necessarily desirable or is seen as a misuse of available resources. Development teams need to balance testing with other demands, such as regularly updating the software to add new features and attract more customers.

## How to reduce coverage testing overhead

Testing is costly in terms of the time it takes to develop and maintain, but also because of the time it takes to run. How long it takes to run an extensive test suite can fluctuate wildly depending on the software and the tests involved. To add to this, regression testing advocates for testing each new change against the entire existing codebase to ensure that no new bugs are introduced. When developing using a methodology that advocates for the continuous integration of small, rapidly developed commits, an extensive suite can quickly develop into a testing bottleneck.

There are several different methods that can be used to mitigate this and reduce the overhead of extensive test coverage. The tests themselves could be analyzed and refactored to reduce the time it takes for them to run as much as possible. Some tests will unavoidably take longer to run than others though, so certain groups of tests can be placed into categories so that they are not run as frequently as other tests or at all under certain conditions.

Other options, such as continuous testing and parallel testing, are available. These methods either run your tests as you write them or run your test suite in parallel, so you do not have to wait for some of the longer tests to finish before moving on to other tests. Both methods deliver results as they happen, cutting down on the waiting time required when running tests, which can be on the order of magnitude of hours to days for sufficiently mature regression tests run in series.

## How much testing is too much?

There is an argument that chasing 100% code coverage is a fool’s errand. This is because development teams have limited resources and must distribute them in the most efficient manner possible to develop the software and remain profitable. How severe a bug adversely affects a product is also heavily dependent on the type of bug, the area of the application the bug might appear in, and the target market of the software in question. A bug with the UI in a rarely used section of a social media app could be overlooked by many users, but a bug that results in users accessing other users’ funds in a banking app would be catastrophic. Thus, it is important to triage errors and prioritize test coverage effectively, given the time and resources available.

## Tips To Maximize Testing Coverage/Efficient Testing

Maximizing available testing resources effectively is generally done by categorizing potential bugs. This can be done in a number of ways. One such way would be by feature usage: features that are used more commonly and extensively should receive a greater proportion of testing time and resources than other features.

Another categorization is by risk, or how severely a bug impacts software. In the example above, a bug that let users access the funds of other users would be a very high-risk bug and should receive much more time and resources than a simple, low-risk UI bug of little consequence.

Feature criticality is another category for test development. This involves testing features in order of how critical they are for the app to work successfully. If a feature breaking has little impact on the user experience, then it should receive fewer resources than a feature that is integral to the user experience.

## Benefits of Balancing Coverage Testing & Overhead

The ‘iron triangle’ of software development involves balancing the allocation of resources to competing needs within a project by a set of criteria, such as speed, priority, and cost. Companies too stuck inside this paradigm may consider that too much testing entails too high a cost or slows down product development. However, it’s not necessarily the case that time and resources devoted to testing is time and resources sacrificed elsewhere.

Smart test coverage involves analyzing a project and identifying what areas to prioritize in testing. This is highly project dependent, and 100% code coverage is not always necessarily the best possible option. Understanding where and what the risks are in software enables teams to develop sufficient test coverage to get the software to market without sacrificing time spent on new features or running over budget.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^micropython]: [MicroPython](https://micropython.org/)
[^jerryscript]: [JerryScript](https://jerryscript.net/)
[^use-after-free]: [CWE-416: Use After Free](https://cwe.mitre.org/data/definitions/416.html)
[^freertos_asserts]: [FreeRTOS - xTaskCreateStatic](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/7c67f18ceebd48ae751693377166df0c52f4a562/tasks.c#L589-L605)
[^static_assert]: [static_assert - CppReference](https://en.cppreference.com/w/c/language/_Static_assert)
<!-- prettier-ignore-end -->
