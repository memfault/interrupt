---
title: "GNU Make Overview and Guidelines"
description: "A brief overview of GNU make features and suggestions for getting
the most out of it"
author: noah
image: /img/gnu-make-guidelines/gnu-make-guidelines.img
---

GNU Make is a very commonly used program for building C language software. It's
used for example when [building the Linux
kernel](https://wiki.archlinux.org/index.php/Kernel/Traditional_compilation#Compilation),
and many commonly used GNU/Linux programs and software libraries.

Most embedded software developers will probably run across software that uses
GNU Make. Though there are [many many alternatives](https://en.wikipedia.org/wiki/List_of_build_automation_software), it's still
commonly used in new and existing software, and it's widely supported.

<!-- excerpt start -->

This article explains some general concepts and features of GNU Make, with some
recommendations for getting the most out of a Make build! Consider it a brief
guided tour through some of my favorite/most used Make concepts and features 🤗

<!-- excerpt end -->

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## What is GNU Make?

[GNU Make](https://www.gnu.org/software/make/) is a program that automates
running shell commands, that helps when running repetitive tasks.

It is typically used to transform files into some other form, eg compiling
source code into programs or libraries.

It enables tracking "prerequisites" and running a hierarchy of commands to
produce "targets".

The GNU Make manual is very good:
[https://www.gnu.org/software/make/manual/html_node/index.html](https://www.gnu.org/software/make/manual/html_node/index.html)

When searching for help on Make, using `gnu make xxx` as the search terms can be
helpful.

Let's dive in!

## Invoking Make

Running `make` will load a file named `Makefile` from the current directory,
and attempt to update the default **goal** (more on goals later).
> *the default is to try `GNUmakefile`, `makefile`, and `Makefile`, in that
> order*

You can specify another file with the `-f/--file` arg: `make -f foo.mk`

You can specify any number of *goals* by listing them as positional arguments:

```bash
# typical goals
make clean all
```

You can run make in another directory with the `-C` arg:

```bash
make -C some/sub/directory
```

Make will run as if it first `cd`'d to that directory.

*Fun fact- `git` also can be run with `-C` for the same effect!*

## Parallel invocation

Make can run jobs in parallel if you provide the `-j` or `-l` options. A
guideline I've been told is to set the job limit to 1.5 times the number of
processor cores you have:

```bash
# a machine with 4 cores:
make -j 6
```

Anecdotally, I've seen slightly better CPU utilization with the `-l` "load
limit" option, vs. the `-j` "jobs" option. YMMV though!

Here's a small bash/python snippet to provide this automatically:

```bash
export CPUCOUNT=(python -c "import psutil; print(int(psutil.cpu_count(logical=False)*1.5))")
make -l $CPUCOUNT
```

## Anatomy of a Makefile

A Makefile contains **rules** used to produce **targets**.

Some basic components of a Makefile are shown below:

```makefile
# Comments are prefixed with the '#' symbol

# A variable assignment
FOO = "hello there!"

# A rule creating target "test", with "test.c" as a prerequisite
test: test.c
	# The contents of a rule is called the "recipe", and is typically composed
	# of one or more shell commands.
	# It must be indented from the target name (historically with tabs, spaces
	# are permitted)

	# Using the variable "FOO"
	echo $(FOO)

	# Calling the C compiler using a predefined variable naming the default C
	# compiler, '$(CC)'
	$(CC) test.c -o test
```

Let's take a look at each part of the example above!

## Variables

Variables are used with the syntax `$(FOO)`, where `FOO` is the variable name.

Variable contain purely strings, Make language does not have other data types.

Appending to a variable will add a space and the new content:

```makefile
FOO = one
FOO += two
# FOO is now "one two"

FOO = one
FOO = $(FOO)two
# FOO is now "onetwo"
```

### Variable assignment

In GNU Make syntax, variables are assigned with two "**flavors**":

1. *recursive expansion*: value is set when the variable is consumed
   ```makefile
   FOO = 1
   BAR = foo=$(FOO)
   FOO = 2
   $(info BAR=$(BAR))
   # prints BAR=foo=2
   ```
2. *simple expansion*: value is set when Make processes the expression
   ```makefile
   FOO = 1
   BAR := foo=$(FOO)
   FOO = 2
   $(info BAR=$(BAR))
   # prints BAR=foo=1
   ```

*Note: the `$(info)` function is being used above to print stuff out, and can be
handy when debugging Makefiles!*

Variables which are not explicitly or implicitly or automatically set will
evaluate to an empty string.

### Environment variables

Environment variables are carried into the Make execution environment, for
example:

Makefile contents:

```makefile
$(info YOLO variable = $(YOLO))
```

Setting the variable in the shell command when running `make`:

```bash
$ YOLO="hello there!" make
YOLO variable = hello there!
make: *** No targets.  Stop.
```

*Note: Make prints the "No targets" error because our Makefile had no targets
listed!*

If you use the `?=` assignment syntax, Make will only assign that value if the
variable doesn't already have a value:

Makefile:

```makefile
# default CC to gcc
CC ?= gcc
```

We can then override `$(CC)` in that Makefile:

```bash
$ CC=clang make
```

Another common pattern is to allow inserting additional flags; in the Makefile,
append to the variable instead of directly assigning to it, which permits
passing flags in from the environment:

```makefile
CFLAGS += -Wall
```

Then we can add extra flags on our command:

```bash
CFLAGS='-Werror=conversion -Werror=double-promotion' make
```

This can be very useful!

### Overriding variables

A special category of variable usage is called **overriding variables**. Using
this command-line option will override the value set **ANYWHERE ELSE** in the
environment or Makfile!

Makefile:

```makefile
# the value passed in the make command will override
# any value set elsewhere
YOLO = "not overridden"
$(info $(YOLO))
```

Command:

```bash
# setting "YOLO" to different values in the environment + makefile + overriding
# variable, yields the overriding value
$ YOLO="environment set" make YOLO='overridden!!'
overridden!!
make: *** No targets.  Stop.
```

Overriding variables can be confusing!

### Target-specific variables

These variables are only available in the recipe context.

They also apply to any prerequisite recipe!

```makefile
# set the -g value to CFLAGS
# applies to the prog.o/foo.o/bar.o recipes too!
prog : CFLAGS = -g
prog : prog.o foo.o bar.o
	echo $(CFLAGS) # will print '-g'
```

### Implicit variables

These are pre-defined by Make (unless overridden with any other variable type of
the same name). Some common examples:

- $(CC) - the C compiler (`gcc`)
- $(AR) - archive program (`ar`)
- $(CFLAGS) - flags for the C compiler

Full list here:

[https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html](https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html)

### Automatic variables

Special variables always set by make, available in recipe context.

They can be useful to prevent duplicated names (Don't Repeat Yourself).

A few common ones:

```makefile
# $@ : the target name, here it would be "test.txt"
test.txt:
	echo HEYO > $@

# $^ : name of all the prerequisites
all.zip: foo.txt test.txt
	gzip -c $^ > $@
```

See more at:
[https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html](https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html)

## Targets (goals)

Targets are the left hand side in the rule syntax:

```makefile
target: prerequisite
	recipe
```

Targets almost always name **files**. This is because Make uses last-modified
time to track if a target is newer or older than its prerequisites, and whether
it needs to be rebuilt!

When invoking Make, you can specify which target you want to build as the `goal`
by specifying it as a positional argument:

```bash
# make the 'clean' and 'all' targets
make clean all
```

If you don't specify a goal in the command, Make uses the first target specified
in the makefile, called the "default goal" (you can also
[override](https://www.gnu.org/software/make/manual/html_node/Goals.html) the
default goal if you need to).

### Phony targets

Sometimes it's useful to have meta-targets like `all`, `clean`, `test`, etc.

In those cases, you don't want Make to check for a file named `all`/`clean` etc.

Make provides the `.PHONY` target syntax to mark a target as not pointing to a
file:

```makefile
# Say our project builds a program and a library 'foo' and 'foo.a'; if we want
# to build both by default we might make an 'all' rule that builds both
.PHONY: all

all: foo foo.a
```

If you have multiple phony targets, a good pattern might be to append each to
`.PHONY` where it's defined:

```makefile
# the 'all' rule that builds and tests. note that it's listed first, to make it
# the default rule
.PHONY: all
all: build test

# compile foo.c into a program 'foo'
foo: foo.c
	$(CC) foo.c -o foo

# compile foo-lib.c into a library 'foo.a'
foo.a: foo-lib.c
	# compile the object file
	$(CC) foo-lib.c -c foo-lib.o
	# use ar to create a static library containing our object file. using the
	# '$@' variable here to specify the rule target 'foo.a'
	$(AR) rcs $@ foo-lib.o

# a phony rule that builds our project; just contains a prerequisite of the
# library + program
.PHONY: build
build: foo foo.a

# a phony rule that runs our test harness. has the 'build' target as a
# prerequisite! Make will make sure (pardon the pun) the build rule executes
# first
.PHONY: test
test: build
	./run-tests.sh
```

**_NOTE!!! `.PHONY` targets are ALWAYS considered out-of-date, so Make will
ALWAYS run the recipe for those targets (and therfore any target that has a
`.PHONY` prerequisite!). Use with caution!!_**

### Implicit rules

Implicit rules are provided by Make. I find using them to be confusing, since
there's so much behavior happening behind the scenes. You will ocassionally
encounter them in the wild, so be aware.

Here's a quick example:

```makefile
# this will compile 'test.c' with the default $(CC), $(CFLAGS), into the program
# 'test'. it will handle prerequisite tracking on test.c
test: test.o
```

Full list of implicit rules here:

[https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html](https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html)

### Pattern rules

Pattern rules let you write a generic rule that applies to multiple targets via
pattern-matching:

```makefile
%.o: %.c
	$(CC) -c $^ -o $@
```

The rule will then be used to make any target matching the pattern, which above
would be any file matching `%.o`, eg `foo.o`, `bar.o`, for example if you use
those `.o` files to build a program:

```makefile
OBJ_FILES = foo.o bar.o

# Use CC to link foo.o + bar.o into 'program'
program: $(OBJ_FILES)
	$(CC) -o $@ $<
```

## Prerequisites

As seen above, these are targets that Make will check when running a rule. They
can be files or other targets.

If any prerequisite is newer (modified-time) than the target, Make will run the
target rule.

In C projects, you might have a rule that converts a C file to an object file,
and you want the object file to rebuild if the C file changes:

```makefile
foo.o: foo.c
	# use automatic variables for the input and output file names
	$(CC) $^ -c $@
```

## Automatic prerequisites

A very important consideration for C language projects is to trigger
recompilation if an `#include` header files change for a C file. This is done
with the `-M` compiler flag for gcc/clang, which will output a file you will
then use the Make `include` directive to tell make which C files include which
header files. See more details here:

[https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html](https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html)
[http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/](http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/)

The basic form might be:

```makefile
# these are the compiler flags for emitting the dependency tracking file. note
# the usage of the '$<' automatic variable
DEPFLAGS = -MT $@ -MMD -MP -MF $<.d

test.o: test.c
	$(CC) $(DEPFLAGS) $< -c $@

# bring in the prerequisites by including all the .d files
include $(wildcard *.d)
```

## Order-only prerequisites

These prerequisites will only be built if they don't exist; if they are newer
than the target, they will not trigger a target re-build.

A typical use is to create a directory for output files; emitting files to a
directory will update its mtime attribute, but we don't want that to trigger a
rebuild.

```makefile
OUTPUT_DIR = build

# output the .o to the build directory, which we add as an order-only
# prerequisite- anything right of the | pipe is considered order-only
$(OUTPUT_DIR)/test.o: test.c | $(OUTPUT_DIR)
	$(CC) -c $^ -o $@

# rule to make the directory
$(OUTPUT_DIR):
	mkdir -p $@
```

## VPATH

`VPATH` is a special Make variable that contains a list of directories Make
should search when looking for prerequisites and targets.

It can be used to emit object files or other derived files into a `./build`
directory, instead of cluttering up the `src` directory:

```makefile
# This makefile should be invoked from the temporary build directory, eg:
# $ mkdir -p build && cd ./build && make -f ../Makefile

# Derive the directory containing this Makefile
MAKEFILE_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# now inform Make we should look for prerequisites from the root directory as
# well as the cwd
VPATH += $(MAKEFILE_DIR)

SRC_FILES = $(wildcard $(MAKEFILE_DIR)/src/*.c)

# Set the obj file paths to be relative to the cwd
OBJ_FILES = $(subst $(MAKEFILE_DIR)/,,$(SRC_FILES:.c=.o))

# now we can continue as if Make was running from the root directory, and not a
# subdirectory

# $(OBJ_FILES) will be built by the pattern rule below
foo.a: $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)

# pattern rule; since we added ROOT_DIR to VPATH, Make can find prerequisites
# like `src/test.c` when running from the build directory!
%.o: %.c
	# create the directory tree for the output file 👍
	echo $@
	mkdir -p $(dir $@)
	# compile
	$(CC) -c $^ -o $@
```

Use of `VPATH` can be a little confusing, and you can usually achieve the same
out-of-tree behavior by putting the generated files in a build directory without
needing `VPATH`.

## Recipe

The "recipe" is the shell commands to be executed to create the target. They are
passed into a sub-shell (`/bin/sh` by default). The rule is considered
successful if the target is updated after the recipe runs (but is not an error
if this doesn't happen).

```makefile
foo.txt:
	# a simple recipe
	echo HEYO > $@
```

If any line of the recipe returns a non-zero exit code, Make will terminate and
print an error message. You can tell Make to ignore non-zero exit codes by
prefixing with the `-` character:

```makefile
.PHONY: clean
clean:
	# we don't care if rm fails
	-rm -f ./build
```

Prefixing a recipe line with `@` will disable echoing that line before
executing:

```makefile
clean:
	@# this recipe will just print 'About to clean everything!'
	@# prefixing the shell comment lines '#' here also prevents them from
	@# appearing during execution
	@echo About to clean everything!
```

Make will expand variable/function expressions in the recipe context before
running them, but otherwise not process it. If you want to access shell
variables, escape them with `$`:

```makefile
USER = linus

print-user:
	# print out the shell variable $USER
	echo $$USER

	# print out the make variable USER
	echo $(USER)
```

## Functions

Make functions are called with the syntax:

```makefile
$(function-name arguments)
```

`arguments` is a comma-delimited list of arguments.

### Built in functions

There are several functions provided by Make. I most commonly use those for text
manipulation:
[https://www.gnu.org/software/make/manual/html_node/Text-Functions.html](https://www.gnu.org/software/make/manual/html_node/Text-Functions.html)
[https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html](https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html)

For example:

```makefile
FILES=$(wildcard *.c)

# you can combine function calls; here we strip the suffix off of $(FILES) with
# the $(basename) function, then add the .o suffix
O_FILES=$(addsuffix .o,$(basename $(FILES)))

# note that the GNU Make Manual suggests an alternate form for this particular
# operation:
O_FILES=$(FILES:.c=.o)
```

### User-defined functions

You can define your own functions as well:

```makefile
reverse = $(2) $(1)

foo = $(call reverse,a,b)
```

A more complicated but quite useful example:

```makefile
# recursive wildcard (use it instead of $(shell find . -name '*.c'))
# taken from https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

C_FILES = $(call rwildcard,.,*.c)
```

### Shell function

You can have Make call a shell expression and capture the result:

```makefile
TODAYS_DATE=$(shell date --iso-8601)
```

I'm cautious when using this feature, though; it adds a dependency on whatever
programs you use, so if you're calling more exotic programs, make sure your
build environment is controlled (eg in a container or with
[Conda](https://interrupt.memfault.com/blog/conda-developer-environments)).

## Conditionals

Make has syntax for conditional expressions:

```makefile
FOO=yolo
ifeq ($(FOO),yolo)
$(info foo is yolo!)
else
$(info foo is not yolo :( )
endif

# testing if a variable is set; unset variables are empty
ifneq ($(FOO),)  # checking if FOO is blank
$(info FOO is unset)
endif
```

The "complex conditional" syntax is just the `if-elseif-else` combination:

```makefile
# "complex conditional"
ifeq ($(FOO),yolo)
$(info foo is yolo)
else ifeq ($(FOO), heyo)
$(info foo is heyo)
else
$(info foo is not yolo or heyo :( )
endif
```

## `include` directive

You can import other Makefile contents using the `include` directive:

`sources.mk`:

```makefile
SOURCE_FILES := \
  bar.c \
  foo.c \

```

`Makefile`:

```makefile
include sources.mk

OBJECT_FILES = $(SOURCE_FILES:.c=.o)

%.o: %.c
	$(CC) -c $^ -o $@
```

## Sub-make

Invoking Make from a Makefile should be done with the `$(MAKE)` variable:

```makefile
somelib.a:
	$(MAKE) -C path/to/somelib/directory
```

This is often used when building external libraries.

It's also used heavily in Kconfig builds (eg when building the linux kernel).

Note that this approach has some pitfalls:

- recusive invocation can result in slow builds
- tracking prerequisites can be tricky; often you will see `.PHONY` used

More details on the disadvantages here:

[http://aegis.sourceforge.net/auug97.pdf](http://aegis.sourceforge.net/auug97.pdf)

## Metaprogramming

`Make`'s `eval` directive allows us to generate Make syntax at runtime:

```makefile
# generate rules for xml->json in some weird world
FILES = $(wildcard inputfile/*.xml)

# create a user-defined function that generates rules
define GENERATE_RULE =
$(eval
# prereq rule for creating output directory
$(1)_OUT_DIR = $(dir $(1))/$(1)_out
$(1)_OUT_DIR:
	mkdir -p $@

# rule that calls a script on the input file and produces $@ target
$(1)_OUT_DIR/$(1).json: $(1) | $(1)_OUT_DIR
	./convert-xml-to-json.sh $(1) $@
)

# add the target to the all rule
all: $(1)_OUT_DIR/$(1).json
endef
```

```makefile
# produce the rules
.PHONY: all
all:

$(foreach file,$(FILES),$(call GENERATE_RULE,$(file)))
```

Note that approaches using this feature of Make can be quite confusing, adding
helpful comments explaining what the intent is can be useful for your future
self!

## Debugging Makefiles

I typically use the Make equivalent of `printf`, the `$(info/warning/error)`
functions, for **small problems**, for example when checking conditional paths that
aren't working:

```makefile
ifeq ($(CC),clang)
$(error whoops, clang not supported!)
endif
```

For **debugging** why a rule is running when it shouldn't (or vice versa), you can
use the `--debug` options:
[https://www.gnu.org/software/make/manual/html_node/Options-Summary.html](https://www.gnu.org/software/make/manual/html_node/Options-Summary.html)

I recommend redirecting stdout to a file when using this option, it can produce
a lot of output.

For **profiling** a make invocation (eg for attempting to improve compilation
times), this tool can be useful:

[https://github.com/rocky/remake](https://github.com/rocky/remake)

Check out the tips here for compilation-related performance improvements:

[https://interrupt.memfault.com/blog/improving-compilation-times-c-cpp-projects](https://interrupt.memfault.com/blog/improving-compilation-times-c-cpp-projects)

## Recommendations

A list of recommendations for getting the most of Make:

1. targets should usually be real files
2. always use `$(MAKE)` when issuing sub-make commands
3. try to avoid using `.PHONY` targets- if the rule generates any file artifact,
   consider using that as the target instead of a phony name!
4. generally avoid using implicit rules
5. for C files, make sure to use `.d` automatic include tracking!
6. use metaprogramming with caution
7. use automatic variables in rules
8. put lots of comments in Makefiles, especially if there is complicated
   behavior or subtle syntax used
9. use the `-j` or `-l` options to run Make in parallel!
10. try to avoid using the `touch` command to track rule completion

## Outro

I hope this article has provided a few useful pointers around GNU Make!

Make remains common in C language projects, probably due to its usage in the
Linux kernel. Many more recently developed statically compiled programming
languages provide their own build infrastructure (Go, Rust, etc); however, when
integrating Make-based software into those languages, for example when building
a C library to be called from Rust, it can be surprisingly helpful to understand
some Make concepts!

You may also encounter [automake](https://www.gnu.org/software/automake/) in
open source projects (look for a `./configure` script). This is a related tool
that generates Makefiles, and is worth a look (especially if you are writing
C software that needs to be very widely portable).

There are many competitors to GNU Make available today, I encourage everyone to
look into them. [Bazel](https://bazel.build/) and
[Meson](https://mesonbuild.com/) are some interesting options.
[CMake](https://cmake.org/) is also pretty popular and worth a look!

## References

- Good detailed dive into less common topics (shout out on `remake`):<br/>
  [https://blog.jgc.org/2013/02/updated-list-of-my-gnu-make-articles.html](https://blog.jgc.org/2013/02/updated-list-of-my-gnu-make-articles.html)

- Mix of very exotic and simpler material:<br/>
  [https://tech.davis-hansson.com/p/make/](https://tech.davis-hansson.com/p/make/)

- Useful tutorial:<br/>
  [http://maemo.org/maemo_training_material/...](http://maemo.org/maemo_training_material/maemo4.x/html/maemo_Application_Development_Chinook/Chapter_02_GNU_Make_and_makefiles.html)

- Nice pictures:<br/>
  [https://www.jfranken.de/homepages/johannes/vortraege/make.en.html](https://www.jfranken.de/homepages/johannes/vortraege/make.en.html)

- Very nice summary:<br/>
  [https://www.alexeyshmalko.com/2014/7-things-you-should-know-about-make/](https://www.alexeyshmalko.com/2014/7-things-you-should-know-about-make/)
