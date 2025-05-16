---
title: "Gophyr: Building a Gopher Client for Zephyr with Claude"
description: "AI-skeptical engineer builds Gopher client for Zephyr using AI"
author: jrsharp
tags: [zephyr, ai, claude, gopher, retrocomputing]
---

<!-- excerpt start -->

This article chronicles my unexpected 3-hour adventure using Claude to create
Gophyr: a fully functional Gopher client for Zephyr, complete with a Zephyr
shell command set.

<!-- excerpt end -->

[![asciicast](https://asciinema.org/a/719343.svg)](https://asciinema.org/a/719343)

## Introduction

You know how it is -- you're logged into your Zephyr shell and suddenly feel the
need to surf gopherspace... Eh? "What's gopherspace?", you ask?

Well, I hope to answer _both_ the "What?" _and_ the "Why?" for you as I walk
through:

- A (brief) Gopher history lesson
- The process of developing a functional Gopher client for Zephyr using an AI
  coding agent
- A tour of the working client and notable Gopher destinations
- What we can learn from embracing simplicity in modern embedded systems

## History Lesson: The Gopher Protocol

About the time Sir Berners-Lee was
[doing his thing](https://en.wikipedia.org/wiki/Tim_Berners-Lee), some folks at
the University of Minnesota brought us a more _spartan_
[vision of hypertext](<https://en.wikipedia.org/wiki/Gopher_(protocol)#Origins>).
Gopher, as defined in RFC1436 [^rfc1436], offers a simple menu-oriented
interface for navigating content (resources) across servers.

![An example Gopher menu displayed in Firefox 1.5](https://upload.wikimedia.org/wikipedia/commons/1/15/Gopher_in_Firefox_1.5.png)

What makes Gopher particularly interesting for embedded systems:

1. Minimal protocol overhead
2. Text-based responses ideal for serial terminals
3. Simple parsing requirements
4. Low memory footprint

This simplicity makes it a perfect candidate for implementation on
resource-constrained systems like modern microcontrollers running Zephyr.

### Protocol Specification Simplified

At its core, the Gopher protocol is refreshingly minimal:

```plaintext
                   GOPHER PROTOCOL FLOW
                   --------------------
│ CLIENT                                            SERVER │
│                                                          │
│  1. Open TCP connection to server (port 70)              │
│     ----------------------------------------------->     │
│                                                          │
│  2. Send selector string + CRLF                          │
│     (empty for root menu)                                │
│     ----------------------------------------------->     │
│                                                          │
│                      3. Server processes selector...     │
│                                                          │
│  4. Receive response data (text, binary, menu...)        │
│     <-----------------------------------------------     │
│                                                          │
│     For MENU response:                                   │
│     Each line has format:                                │
│     [Type][Description]\t[Selector]\t[Host]\t[Port]\r\n  │
│                                                          │
│     Where Type is a single character:                    │
│     0 = Text file    1 = Directory                       │
│     7 = Search       h = HTML document                   │
│     i = Info message                                     │
│                                                          │
│     Response ends with single "." line                   │
│                                                          │
│  5. Close connection when done                           │
│     <------------------------------------------->        │
```

That's it - no cookies, no sessions, no headers, no complex state management.
Just a simple selector string sent to the server, and a straightforward response
back. When you want to navigate to another resource, you make a new connection
with a new selector. This elegant simplicity makes Gopher perfect for
constrained environments like embedded systems.

## The Development Process

The inspiration for this project struck one evening after working with my
[nRF7002-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF7002-DK).
It dawned on me that building a Gopher client for Zephyr would be a great test
for my new acquaintance, Claude (3.7). Now, I understand the mention of AI may
illicit a certain negative response, and frankly, that may be entirely deserved.
I don't know that my own thoughts are even now resolved on this topic. Still,
I've been encouraged by my coworkers to give it a chance, so I decided to take
the opportunity to learn about agent-assisted coding as applied to
retrocomputing and embedded systems.

### An efficient project kick-off

I began by asking Claude to create a new Zephyr shell application project,
creating the essential build files according to convention. Claude understood
the assignment. The serial shell [^zephyr_shell] provides a constrained but
perfectly suitable CLI interface for a Gopher client. I regularly use CLI-driven
Gopher clients on desktop systems ([cgo](https://github.com/kieselsteini/cgo) is
a favorite), and in this case, we have to engage our Gopher client functions
through the `gopher` command prefix. (I later improved the ergonomics slightly
by introducing `g` as an alias)

Once I confirmed a working project template, I simply asked Claude to add a
Gopher (RFC1436-compliant) client. The Gopher protocol is refreshingly
straightforward - TCP-based client-server communication with a simple
line-oriented text format. Each menu item is marked with a single-character
code, and server responses need minimal parsing. Claude was able to grok the
standard immediately.

### Iterative Development (So _this_ is "vibe coding"...)

This turned out to be my first "vibe coding" session (cringe-worthy term
notwithstanding), and I found myself falling naturally into a collaborative
rhythm with Claude. Despite initial skepticism, I was impressed by how quickly
it grasped the task from minimal prompts, delivering immediately useful code.
Claude even showed surprising initiative - when I vaguely suggested "improve the
rendering output," it implemented a series of enhancements, including ANSI color
codes I hadn't considered.

For this initial session, I just used Anthropic's Claude web UI (this seemed
like a good place to start) and developed a simple file-copy flow for the
outputs from the browser. This was only slightly cumbersome and allowed me to
maintain a greater sense of control over the flow of information from the AI
into my new codebase. (I have since let my guard down and discovered the
benefits (and frustrations) of tighter agent integration ala Cursor -- and more
recently, the new Claude CLI)

The only real hiccups came from API changes. Claude wasn't familiar with the
latest Zephyr APIs in my pre-release branch, requiring updates to networking
calls and shell command registration patterns. I took a bit of a daft approach,
feeding compiler errors back directly, but the compiler was eventually satisfied
-- the familiar edit-compile-run loop is preserved!

What struck me most from this initial session was how implementation details
faded into the background. Instead of syntax and API minutiae, our conversation
focused on capabilities and features. I found myself thinking at a higher level
of abstraction - "we need a menu rendering system" rather than "let's loop
through these structs." This shift created a fluid creative process where
technical constraints didn't dominate. The unexpected benefit wasn't just
productivity, but a more enjoyable, conceptual approach to problem-solving.

After about three hours, (and only 15 prompts!) I had a very serviceable Gopher
client capable of rendering Gopher menus and displaying ASCII text resources.
There were bugs aplenty (responses being truncated, mostly), but it worked! I've
since fixed several bugs and begun work on new functionality, testing it on my
ESP32-S3 hardware along the way. Maybe this will become my preferred Gopher
client as I chip away at the `TODO` list one credit at a time?

### Claude's own recollection of that initial conversation

In preparing this article, I went back and found this open tab and asked Claude
about the conversation. This was the response:

```plaintext
In this chat, you've sent 16 prompts/messages:

Initial request to create a Zephyr RTOS application with serial shell interface
Feedback about compilation failure
Message saying "awesome, thanks!" and request to add WiFi connectivity
Report about compilation failure with specific error
Message about code compiling and request to add a Gopher client
Message about compilation issues with struct fields
Message with errors from paste.txt
Feedback about display functions not connecting properly
Message with the paste.txt document about gopher_shell.c failing to compile
Message with new compiler errors
Note that the latest changes weren't in the file
Message with compiler errors about "size_t" and other issues
Message with more errors about "struct net_if_addr_ipv4"
Message with link error for "net_if_ipv4_addr_get"
Message saying "That did the trick!" and request to add support for non-canonical Gopher item types
Current message asking how many prompts were used

This has been a detailed troubleshooting and development conversation with multiple iterations to get everything working correctly.
```

Yeah, this sounds right. I didn't experience any hallucinations, really, or any
of the kinds of circular thinking I've encountered in later sessions.

## Navigating Gopherspace in style

With Gophyr compiled and running on the nRF7002-DK, it's time to take it out for
a spin on the _Route 66 of the Information Superhighway_. Let's fire up the
serial terminal and explore this historic digital landscape.

Gopherspace is one of those early digital neighborhoods you might be surprised
still exists. (Remember, shortwave radio is
[still a thing](https://hackaday.com/blog/?s=shortwave) too!) It's an alternate
web where content is organized by type and server, not chaotically linked like
today's "surface web." Here you'll find text files (lots of them), directory
menus, search interfaces, and proxies to popular www services.

With Gophyr running on our shiny MCU board, we can use the following Gopher
client commands to navigate this space in style:

```plaintext
uart:~$ gopher
Gopher Client Commands:
----------------------
gopher ip - Display network information
gopher connect <host> [port] - Connect to a Gopher server
gopher get [selector] - Request a document or directory
gopher view <index> - View an item from the directory
gopher back - Navigate back to previous item
gopher search <index> <search_string> - Search using a search server
gopher help - Display this help message
uart:~$
```

SDF.org is a great place to start our Gopher journey:
([sign up for a shell account](https://sdf.org/?signup), while you're at it,
eh?)

![sdf.org in full-color Gophyr, running on an nRF7002-DK](/img/gophyr/gophyr_sdf.png)

### Notable Destinations

Beyond SDF, we can navigate to several interesting Gopher servers still
operational today:

#### **Floodgap Systems** (gopher.floodgap.com)

```plaintext
shell> gopher connect gopher.floodgap.com

Welcome to Floodgap Systems' official gopher server.
Floodgap has served the gopher community since 1999
(formerly gopher.ptloma.edu).

We run Bucktooth 0.2.9 on xinetd as our server system.
gopher.floodgap.com is an IBM Power 520 Express with a 2-way
4.2GHz POWER6 CPU and 8GB of RAM, running AIX 6.1 +patches.
Send gopher@floodgap.com your questions and suggestions.

***********************************************************
**              OVER 20 YEARS SERVING YOU!               **
**               Plain text is beautiful!                **
***********************************************************

1: [DIR] Does this gopher menu look correct?
(plus using the Floodgap Public Gopher Proxy)
2: [DIR] Are you running a bot? Are we blocking you?
Please read this document. Please don't cost us money.
3: [DIR] Super-Dimensional Fortress: SDF Gopherspace
Get your own Gopherspace and shell account!

...
```

#### **HN Gopher - A Hacker News Mirror**

```plaintext
shell> gopher connect hngopher.com
    _    _            _               _   _
    | |  | |          | |             | \ | |  hngopher.com:70
    | |__| | __ _  ___| | _____ _ __  |  \| | _____      _____
    |  __  |/ _` |/ __| |/ / _ \ '__| | . ` |/ _ \ \ /\ / / __|
    | |  | | (_| | (__|   <  __/ |    | |\  |  __/\ V  V /\__ \
    |_|  |_|\__,_|\___|_|\_\___|_|    |_| \_|\___| \_/\_/ |___/

              - The Underground Hacker News Mirror
                        (updated hourly)

                              _________________
                              |\                \
                              | \                \
                _     _      |\ \________________\
                (>\---/<)     |\\ |      ____     |
                ,'     `.     |.\ |     |\   \    |
              /  q   p  \    |   |     |.\   \___|____
              (  >(_Y_)<  )    \  |     ||\\   \       \
              >-' `-' `-<-.    \ |     || \\___\       \
              /  _.,,  ,,,- \    \|_____||  ||  !\_______\
            /,    )`  '(    )   ______ ||  ||  !  . . . |
            ; `._.'      `--<    \000  \~\\ ||  !   . .  |
          :     \        |  )    \000 9\ \\||  !    .   |
          \      )       ;_/      \000 9\ \.|  !        |
            `._ _/_  ___.'-'''      \000 9\ \|__!________/
              `--'''                ^^^^^`` \/_____\
                                        hjw/hectoras/mozz
________________________________________________________________
This is a mirror of the popular tech news aggregator Hacker News
(https://news.ycombinator.com). All stories and discussions have
been converted to plain-text ASCII. Have fun exploring the site!
________________________________________________________________

1: [DIR] Live Feed
  (last update 2025-05-12 21:02:10 UTC)

...

```

## Learnings

### Assessing Utility

Gophyr might seem like just a fun project (and it _is_ fun, to be sure), but it
reveals several truths. Simple, well-defined protocols shine in constrained
environments. Modern embedded systems and early personal computers share
surprising similarities. And minimalist design principles prove their enduring
relevance.

The fact that a fully functional Gopher client can be developed in a single
evening and run on a microcontroller testifies to both the protocol's elegance
and modern embedded capabilities.

Perhaps even more practically, Gophyr serves as an advanced Zephyr diagnostic
tool. And you can always use this thing as your Gopher daily driver -- reading
phlogs, news, etc. without distraction.

### Reflections on AI-Assisted Development

So about my AI-skepticism...you see, I used ChatGPT a total of one time before
this -- uno (and only then because I was lured in by a novel integration with
another, now defunct retro protocol!). And now? Well, working with Claude wasn't
the soul-sucking experience I'd feared, nor did it diminish the engineering
thought process. Instead, it shifted focus from syntax to semantics, from
implementation details to architectural choices.

For well-defined problems like implementing a simple protocol, an AI assistant
accelerates development while still requiring human expertise and judgment. I
found myself in a directorial role, focusing on the "why" and "what" while
collaborating on the "how."

This conclusion doesn't invalidate concerns about AI in software development,
but suggests a nuanced middle ground between hype and dismissal. The key is
choosing appropriate problems with clear boundaries where creative engineering
decisions remain in human hands.

### Future Possibilities

What might the future hold for Gophyr? I have some ridiculous ideas I'm only
half-joking about:

1. **Enhanced media support**: Adding simple image rendering ala
   [aalib](https://en.wikipedia.org/wiki/AAlib) (Working on it!)

2. **Local caching**: Storing frequently visited menus to reduce network usage
   -- perfect for those times when you're browsing gopherspace on a
   battery-powered device in the wilderness

3. **Microcontroller personal computer**: Implementing Gophyr on a custom
   microcontroller-based personal computing device with its own keyboard and
   display. Who needs a Raspberry Pi when you can browse gopherspace on
   purpose-built hardware? (TTGO T-Deck?)

4. **Other Zephyr shell backends?**: Gopher over MQTT, [^mqtt_shell] anyone?

5. **Ancient RFC series?**: Look for a future interrupt article on another
   ancient-but-relevant RFC implementation.

### Final Thoughts

Gopher embodies a zen-like philosophy increasingly relevant in our
over-engineered digital landscape. It's elegant minimalism serves as a
counterpoint to the modern tendency toward complexity for complexity's sake.
Gopher's clarity of purpose feels like a breath of fresh air in a world of
bloated web standards and frameworks built upon frameworks.

As embedded systems continue to grow in complexity, the lessons from simple,
robust protocols like Gopher become even more valuable. Sometimes the best way
forward is to look back – to rediscover the power of focused, intentional design
that does exactly what it needs to do and nothing more.

This principle extends to development tools as well, including AI assistants.
The most valuable approach might be a pragmatic one that recognizes limitations
and strengths, focusing on collaboration that produces meaningful results. Much
like Gopher's simple elegance, sometimes the most effective tools aren't the
most complex ones, but those that solve specific problems with precision and
intention.

Oh, and did Claude help write this article? Let's just say I'm confident that
your favorite parts were written by me. ;)

---

The complete source code for Gophyr is available at
[GitHub](https://github.com/jrsharp/gophyr). If you're interested in trying it
out on your own Zephyr device, follow the setup instructions in the repository
README. YMMV.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^rfc1436]: [`rfc1436`](https://www.rfc-editor.org/rfc/rfc1436.html)
[^zephyr_shell]: [`zephyr_shell`](https://docs.zephyrproject.org/latest/services/shell/index.html)
[^mqtt_shell]: [`mqtt_shell`](https://docs.zephyrproject.org/apidoc/latest/shell__mqtt_8h.html)
<!-- prettier-ignore-end -->
