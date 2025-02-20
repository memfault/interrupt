---
title: Building your own Android hardware observability
description: 
author: victorlai
---

<!-- excerpt start -->

In this article, I walk through what a typical journey building internal observability tooling for your AOSP device might look like, and the variety of pitfalls you might encounter as you scale from 1s to 10s to 1000s of Android devices in the field, based off my experience talking to AOSP developers and teams and personally as an Android app developer for AOSP hardware.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Introduction to Android hardware observability

With the popularity of mobile app development over the last decade, many developers are at least vague familiar with what's involved with shipping and maintaining an Android app. But if your only experience is with Android apps, but you're suddenly thrown into a project developing on Android hardware, it can be surprising how the single mobile app is just a tiny fraction of the complexity involved.

When you ship an Android app to Google's Play Store so it can be installed on Android smartphones from Google, Samsung, Motorola, or some other manufacturer, you're only responsible for delivering a single Android app. But when you deliver Android hardware, there are actually tens (hundreds?) of Android apps on the device to think about &mdash; like Status Bar, Settings, Software Updater, Dialer, Messaging, Documents, Keyboard, and more. And there are even more native services that run as part of the core Android OS. Each custom Android device is also usually ordered from an ODM (Original Design Manufacturer), which customizes the base version of the Android Open Source Project (AOSP) with ODM-specific changes to support that specific SoC (system-on-a-chip) by providing drivers for hardware components. So at the end of the day, at the bare minimum, you're responsible for a device that's running some parts your own Android apps, some parts your ODM's customizations, and the whole of AOSP, which is also itself essentially running a customized version of Linux.

To make things even more complicated, if that Android hardware was ordered from an ODM, you might not contractually have access to the source code of the OS (since the ODM would rather not let you see the types and quality of changes to AOSP), which greatly limits your ability to dig into and understand the operation of the device, instead relying on the terms of support in your contract.

This web of complexity might not seem like a big issue, but it is the nature of all software and hardware today that there are bugs, and the more complicated the system, the more complicated the bugs that can arise. It's unrealistic to expect to catch all bugs during development, and so what we need are a variety of tools to help diagnose and fix problems when the hardware is in the field and in our customer's hands.

## A journey in collecting device data from the field

If you're developing Android hardware, it's likely the core of your device is likely several standard Android mobile apps, and you can use the mature ecosystem of Android app observability tools like Crashlytics, Bugsnag, Instabug, etc to maintain the quality of those apps.

The first hiccup in development often happens when a user observes a bug that isn't captured by those existing app observability tools. The user's error report can often be pretty vague (the screen just went black!), so we need a way to get more information from that user's device. And early on in the process of Android development, we may have come across something called [Android bugreports](https://developer.android.com/studio/debug/bug-report), so it's likely we have a process in place for the user to create that report, and then send it to us. In some places I've seen a process where the user is walked through how to create a bugreport and then email it to the engineering team!

There are many obvious problems with this approach: they might already be unhappy with the experience and unwilling to cooperate; they might have returned the product already; they might be busy and miss the message. But if there are a low number of devices in the field, this can feel somewhat sustainable, and we didn't need to build or change anything in the software!

Of course, as we acquire more users, we can improve the process here. We might first focus on automating the bugreport collection to avoid having the customer do the work &mdash; when they call in with an issue, we can remotely trigger a bugreport for them. To do this, we'll have to develop a new native service in our AOSP build to listen for a trigger, which might be a big feature in itself if push notifications are not supported. And then we'll have to find a sustainable, secure location to upload the bugreports to. We might have to compromise: the bugreports might be sent to a personal email in the interim, or the bugreports might never expire, or the bugreports might be shimmed into an existing system that wasn't meant to handle that file size or volume. But it'll work, for now.

It's likely we start encountering situations where the bugreports might not be enough. So we get the idea to upload the device logs as well. We'll quickly discover the different types of [logcat buffers](https://developer.android.com/tools/logcat) on the device: like main, system, or crash, and see that some kernel errors showed up in the system buffer in the past. The size of the buffers are quite small, so you might have to find ways to increase the size of the buffers to persist them for longer. And it could make sense to append these device logs onto the same bugreport system as before.

Over time, this process could improve iteratively, as it's kind of generally good enough. If there isn't enough data in the bugreport, or in the device logs, there might be more areas of AOSP to query and add to be uploaded, like tombstone files, or dumpsys, or recovery logs, or system services. We may eventually refine the backend too, so there's a dedicated area for selecting all the uploaded data from a single device.

## Common traps to avoid

### Push vs Pull

While we may have a push notification to trigger a report to be collected, the system as a whole is still a pull-based system. That is, it requires someone to request data from the device. An obvious symptom of this, is that sometimes when a bug happens to a user, the user might take several days to reach out to customer support, and the data is not in the buffer anymore. Or, there might be a delay in customer support that causes the data to be lost. Or you might want to pro-actively check how often an issue is happening, but you can't in good conscious randomly request data from hundreds of devices for no reason.

We might be able to think of ways to convert our existing system to automatically push data when it detects an issue, but it can run into a lot of different roadblocks: maybe our backend wasn't designed for uploading a higher volume of data, maybe there's data we need to dedupe, maybe it's not very performant to do that on the device.

### Data fatigue

Inevitably after tens or hundreds of bugs and customers investigated and fixed or abandoned, we might recognize that this is all just raw data. And as programmers, we can develop tools to look for what's important from the raw data &mdash; we can parse the logs automatically for keywords, we can parse the bugreport automaticaly for parts of interest, and we can have a tool summarize what's interesting rather than look for it manually. There's often a lot of noise, and we might be convinced we could filter it out.

This starts looking like an infinite problem though. Depending on the complexity of the data, there might be too many variables to reasonably compare, and instead you want to build visualizations instead. You can take that visualization problem really far, and build something like [Battery Historian](https://github.com/google/battery-historian) to help visualize the data locally.

### Campfire or wild fire

At a certain scale, you can't realistically fix every bug that comes your way. So you need to start developing a system of prioritization to determine whether a fire is a small campfire or a widespread wild fire. If we develop a way to trigger data uploads automatically, and if we develop a way to parse that data into something actionable, then we can start the process of uniquely identifying one issue from another, and then counting the number of devices that are experiencing that issue. These numbers will need to be uploaded to another kind of metrics service so that they can be sliced when needed.

### Building the team

While all of the work above can seem somewhat doable, the reality is that ends up requiring a team of engineers with rather diverse skillsets across the entire stack, not just Android OS developers, to both build the first iteration of the system, and maintain it as it scales to more devices and encounters a variety of bugs: AOSP developers are needed to collect and upload the data from the device and provide domain expertise to the others, backend developers are needed to maintain and update the processing pipelines as they evolve, frontend engineers might be needed to create a nicer way to download the data, data scientists to ingest the metrics and set up charts and alerts and a way to query the data. Staffing a full team just to maintain a hodgepodge observability solution is a tall ask, for many engineers who would rather be building new features.

## Summary

There's a ton of content on what observability means for Android apps, but content and tooling for AOSP device observability is sparse in comparison, leading many AOSP developers to just build internal tools because of that lack of standardization. And most AOSP developers end on a similar journey: they discover the limits of the their apps observability tool, they then figure they want to pull Android bugreports and [logcat device logs](https://interrupt.memfault.com/blog/device-logging), then they find a way to push the logs from the device instead of pull, and then they discover even more Android data they want to include, but would rather parse the data automatically, and do it for every device, not just the one they're working on, and fight to build a team that could do all of this sustainably.

One of the reasons I joined Memfault was to build the tooling I wish we had previously, because I ran through all the same roadblocks I described above, and it'd be awesome to help other developers to learn the same lessons I had in less time. And there are still many more lessons in AOSP observability to learn, so if you have experience with observability at scale, I'd love to hear from you.


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

