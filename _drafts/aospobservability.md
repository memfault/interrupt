---
title: Building your own AOSP observability tooling
description:
  A comparison between Android app versus AOSP observability, and a timeline of 
  how internal observability tooling grows.
author: victorlai
---

<!-- excerpt start -->

AOSP device observability is a relatively unexplored field, so there's still so much information to share with each other. In this Memfault-adjacent article, I walk through a common progression in how internal tooling for AOSP hardware progresses, based off my experience talking to AOSP developers and teams in the field, and personally as an Android app developer for AOSP hardware.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}


## _Android_ vs AOSP observability

For those less familiar, Android is the general name of the operating system, whereas AOSP (or Android Open Source Project), is the generic implementation of Android. Generally speaking, companies will build on top of the generic AOSP implementation, and add their own company- and product-specific customizations and proprietary functionality. For example, the Google’s Pixel phones run a priority Pixel version of AOSP, as do Samsung’s Galaxy phones or Amazon’s Fire tablets, to add features or control the device in a way that only make sense at the point in time for Google or Samsung or Amazon customers.

There are definitely many good reasons to build on top of AOSP. Android is really the only option for custom, touch-first experiences, and the popularity and price point of phone, tablet, and watch form factors is really attractive for a lot of different product experiences. Android SoC vendors bundle support for a lot of common capabilities by default: Bluetooth, LTE, Wi-Fi, cameras, batteries, sensors, and more. And Android also has an incredibly strong community and ecosystem, the continued support of Google, and millions of apps and developers.

But people often don’t distinguish between Android developers and AOSP developers enough. While Android *app* development is incredibly popular especially as a hobby, AOSP development is only really done for work due to the custom SoC required (outside of the LineageOS or GrapheneOS folks). More importantly, the actual work done by Android versus AOSP developers is often totally different. Android *app* developers are generally only involved with product-centric app development (which in itself is a huge problem space), whereas AOSP developers may develop some apps, but also work much closer to the kernel, by writing drivers, implementing vendor or hardware interfaces, creating and updating sepolicy, and more.

What this means is that there’s a whole world of Android device- and hardware-specific information and insights, that aren’t served by off the shelf observability tools, and AOSP developers have often been left to build their own tooling.

### The Limits of App Observability

Android developers are very familiar with Android **app** observability tools - everyone by default integrates Crashlytics, Bugsnag, Sentry, Instabug, or another observability SDK. When I worked on an AOSP product, we actually built 10 or so apps to replicate some device functionality, so we had to integrate the observability SDK into all 10 apps. But what about the other 10 to 100 apps and native services that we don't normally touch that were part of the generic AOSP implementation?

When you build on top of AOSP, you’re actually potentially running 4 different sources of "apps": your own custom-built apps, your SoC vendor’s apps, the generic AOSP apps, and if you have an app store, third-party apps installed from the app store. You can only install an app observability SDK in your own custom-built apps, you can’t install SDKs into all these other apps, so you immediately hit the limit of what information app observability SDKs can provide.

![Apps you don't build yourself can't integrate app observability SDKs](/img/aosp-observability/app-sources.svg)

There are also 4 kinds of “apps”: (1) the common Android app written in Java or Kotlin, (2) native apps using C++ using the Android NDK, (3) binaries written in C++ or Rust, and (4) init.rc shell services which can invoke the binaries and more. App observability SDKs often come with NDK support, but they run into the same problem of only working on apps you build yourself, and there are 10 to 100 other apps running on the device that aren’t monitored.

## Get the data

### Logcat logs

To get more observability (outside of custom-built apps), the first thing that many Android developers build is a way to access raw data from the device - in this case, logs.

While there’s a lot of debate about whether logging in production is harmful to device performance or not, it’s undeniable that there’s a lot of value in logs (and a lot of noise). So logs end up being the first tool that developers fall back to when debugging a problem. Except that logs are ephemeral, so inevitably developers will need build a way to cache more logs than just the default of 4MB (depending on the Android version). In my case, we had developed a way to pull the logs off a device if my laptop was connected to the same Wi-Fi network, so early on I would connect to our alpha user’s network to do some debugging.

### Push vs Pull

This might’ve been sufficient for a while (a very short while), but we thought a better way was to have users send us logs when they encounter a problem, so we created a bug report flow that would zip the log cache and then send an email with the logs attached, plus some information about the device and user.

As time went on though, we started running into more and more situations where the cached logs didn’t actually span the time of the incident, so they were basically useless. This is the natural result of a reactive pull-based model, and so we got to brainstorming what other solutions were possible.

We eventually landed on the desire for a push-based model - literally a push messaging system, where we could poke the device remotely to trigger an upload. And we designed an app that could query FileProviders and ContentProviders for any additional data. But we hit a couple of roadblocks: our device was not GMS-compliant so we didn’t have Firebase Cloud Messaging and would have to integrate our own push messaging system, and we needed to enlist the help of a backend team to build the push backend and the push trigger, but also a storage solution, and no team had the budget to help and our teams didn’t have the expertise to contribute. This ends up being a common sticking point with internal tools for mobile teams - Android and AOSP developers have a lot of mobile experience and expertise, but often need help creating a maintainable backend solution that makes sense for the volume and cost of storing and processing the data.

![The logs that are pulled might not contain useful data from around the time of the incident](/img/aosp-observability/logs-buffer.svg)

### Android Bug Reports

Logs are one tool to diagnose an issue, but Android has so much more information if you can access it - namely, Android Bug Reports, which gathers data from all over the device into a single zip file. So any AOSP observability solution that can capture and upload logcat logs automatically, may also want to build a way to trigger and upload Android Bug Reports when needed, too. And so Bug Reports are often the second thing that developers build when they just need more AOSP observability.

## Understand the trends

As we encountered more and more bugs, and downloaded more and more logs, several colleagues decided to write a number of different parsers in Python that they would run on the logs to rule out certain problems, or filter the logs for certain crashes. As they encountered more and more issues, if they could identify the issue in the log, they could write a parser for that issue and re-use it for next time.

This kind of speaks to 2 kinds of issues with manually combing through logs:

- While everyone agrees that knowing how to comb through logs is an incredibly valuable skill to develop, at some point in time the tediousness wears on you, and you’d rather have something tell you what and where the useful insight is.
- As the number of devices grows, figuring out the relative scale of an issue by manually parsing each device’s logs becomes infeasible, and you really want to know how many devices are impacted by a certain issue to determine the priority.

At this point, teams often start to recognize another tool is needed to progress from *debugging* individual devices, to *monitoring* trends across the entire fleet.

### Buy _and_ Build

Most teams that develop Android hardware already buy Android app observability tools, like Crashlytics, plus a general purpose product analytics tool, like Grafana, for their backend, so it seems natural to try to fit AOSP data into these tools. But these tools are not designed for AOSP, they’re designed for apps or for servers, and so their data models or pricing structures can feel like they don’t make sense.

- Existing Android app observability tools have a battle-tested data model for Java and Kotlin app crashes and C++ tombstones. But there’s really no way to record any other forms of data, so you have to coerce other kinds of crashes, like kernel panics, into the data model, which can be finicky and tedious to manually manage. -
- Combining an Android app observability tool with an product analytics tool might get you the best of both worlds, if you understand all the limitations involved. For example, most observability SDKs are all clearly server-focused (such as by providing a Java SDK as opposed to an Android SDK), so they often have no regard for data usage, bandwidth, battery use, or device performance, and it can be a minefield in a lot of ways.

If you can make a decision on what to buy, then it’s up to the team to build whatever pipeline that’ll collect, transform, and send the device-level data into the observability tools. There can be so many interesting decisions to make around whether which kinds of processing belongs on the device versus in the cloud, depending on whether the team can act full-stack or not, what data is being collected, and the costs involved. A dedicated engineer or team may need to be staffed to develop the necessary functionality or to manage its costs.

![A simplified view of the eventual AOSP observability pipeline](/img/aosp-observability/analytics-pipeline.svg)

### A Single Source of Truth

The common pain point that eventually shows up in more mature organizations and products, is around having to jump around multiple tools to investigate an issue or monitor a release. So there’s soon a desire to centralize information in one place. That often looks like configuring these tools to export and import data to each other, or even building yet another tool that’ll summarize the previous ones. And it becomes incredibly valuable to be able to query both the app observability and product analytics tool at the same command.

## Conclusion

There's a ton of content on what observability means for Android apps, but content and tooling for AOSP device observability feels sparse in comparison. Most AOSP developers end up building internal observability tooling because of the lack of standard tooling. And most AOSP developers end on a similar journey: they discover the limits of the their apps observability tool, and they figure they want to pull logcat logs, then they find a way to push the logs from the device, and then they think of ways to pull the logs, and then discover even more Android data they want to pull, and then they finally realize they don't want to pull and parse the data individually and manually every time there's a problem.

One of the reasons I joined Memfault was to build the tooling I wish I had previously, and to help other Android developers working on AOSP avoid going through the same years of struggles, to learn the same lessons I had. And there is still many more lessons in AOSP observability to learn, if you have experience in observability at scale, I'd love to hear from you.


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

