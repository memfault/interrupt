---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
#
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
#
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md
#
# It will now show up in the front page when running Jekyll locally.

title: A random bug
description:
  Post Description (~140 words, used for discoverability and SEO)
author: david
#image: img/<post-slug>/cover.png # 1200x630
---

I was appointed as software/firmware team leader for a subsection of a company
whose main business was in heavy electrical engineering. The section had a
flagship suite of three products, two of which could operate standalone.
The third was the human interface device (HID) which consists of a small
LCD display and a main control which was a rotary switch with a pushbutton.
Turning the knob clockwise and anticlockwise moved the on-screen cursor
through the menu items and pressing the control selected the highlighted
menu item. Quick and intuitive to operate. The other two units were an
instrumentation/measurement unit and the third was a controller. A full
installation comprised all three units connected together using a CAN bus
interface. A large installation could have up to 300 units installed.
A little bit of background of why the reader should read this post.

<!-- excerpt start -->

In this post, I tell the tale of a terrible bug which would sometimes partially
shut down plants. On this debugging journey, I dug into distributed systems,
control theory, the CAN protocol, and more.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## The Problem

One of my tasks was to investigate fault reports sent by a couple of large
household name companies. The issue was that in a large installation, control
or HID units would sometimes "randomly" get the wrong values and take
inappropriate actions as a result. This was a rare but high nuisance- value
issue that could result in faulty operation of the plant. In the worst case
scenario this could result in partial plant shutdown. Failure frequency was
every two or three months. Obviously, the nature of the plant being what it was
and the customers being who they were, there was significant incentive to
resolve this issue.

At this point, the eagle-eyed and astute readers will have spotted the problem
and are way ahead of this story! What we have here is a distributed control
system (DSC) which was the core of the whole SCADA system supplied to the
customers. Individual units and panels were operating asynchronously.

The engineering issue here is that the product was designed in parts with no
formal systems-level design. This is often the case, where each module is
designed with the features required or specified but little consideration given
to the interaction of the parts of the system. It works in our limited testing
so ship it! The fundamental design oversight was that each measurement and
control unit stored it own individual operating parameters but shared data in a
central "database". In quotes here because in reality a shared table of values.
The data was updated using the CAN bus.

## The Plan

So, confronted with this situation it is obvious that "Houston - we have a
problem!" Obvious because code inpection revelaed there to be no
synchronisation primitives in the system. Locking of individual records in a
database with multiple readers and writers is a problem which was solved a long
time ago but is a bit tricky in the control/ SCADA system just outlined. So how
do we go about solving this problem? Code inspection was a non-starter - I had
already scrutinised the code and more hours staring at it was unlikely to
reveal the answer. All three units were built from a common codebase because
the CAN communications code was identical and the build system allowed the
different types of functionality to be pulled in at configuration time. The MCU
was the same for each type of module.

My strategy was to throw some statistical analysis at it. Some crude
calculations enabled me to work out how many units would be needed in order to
get the bug to manifest in a reasonable timeframe. The plan was to build a
large test installation with monitoring sytems in place to alert when the rogue
values were detected. My plan as presented to senior management was met with a
lukewarm reception but they did agree to release a smaller number of units from
stores to enable the team to build a test rig. This was set up in the
office/lab in the middle of the floor and was about six feet high with units on
each of the four side of the rig.

The rig was assembled and started up. After about two weeks of continuous 24/7
operation, the bug showed up and I was onto it, now armed with the additional
instrumentation and monitoring data. 

Fortunately, with the data I was able to return to the code with more informed
eyes and was able to put a "fix" in place. More of a workaround really because
what was really required was a code refactoring with more rigorous
mutex/semaphore style locks in place. However, more testing showed that the fix
really did work - according to my statistical analysis anyway!

## Conclusion

If you are thinking that the company should have done more testing, the units
are tested with a specialised test-ring and a dedicated operator with
familiarity of the equipment and a lot of experience.  This is an ongoing
process and so the company was investing time and money in testing induvidual
units. However, you can only test what you have thought about and correct
functioning for a limited time period is not predictive of a large scale
installation with up to 300 individual modules operating autonomously. Each
installation is "mocked up" (ie, not driving actual loads) and tested for
correct functionality and demonstrated to the customer for final sign-off
before being sent for installation and commissioning.

This highlights that old chestnut "absence of evidence is not proof of
abscence". Or in other words, just because you can't see a bug doesn't mean
that it isn't there!

The takeaway points here are that formal design and design reviews are
essential but can be tedious (but donuts make the process sweeter!) and
full-scale testing is also required and however much you do, chances are it
isn't enough! Your test installation needs to be bigger than the largest
installation than your customer is likely to want and needs to be run and
monitored continuously. But go ahead - you try putting that one over to a
senior mnagement team who are more concerned about the bottom line on the
financial accounts!

<!-- Interrupt Keep START --> {% include newsletter.html %}

{% include submit-pr.html %} <!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
