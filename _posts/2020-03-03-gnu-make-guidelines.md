---
title: "A Shallow Dive into GNU Make"
description: "A brief overview of GNU Make and its features, Makefile syntax, and how to use Make to build C/C++ software and firmware"
author: noah
image: /img/gnu-make-guidelines/gnu-make.png
tags: [toolchain, c]
---

GNU Make is a popular and commonly used program for building C language software. It is used when [building the Linux
kernel](https://wiki.archlinux.org/index.php/Kernel/Traditional_compilation#Compilation)
and other frequently used GNU/Linux programs and software libraries.

Most embedded software developers will work with GNU Make at some point in their career, either using it to compile small libraries or building an entire project. Though there are [many, many
alternatives to Make](https://en.wikipedia.org/wiki/List_of_build_automation_software),
it's still commonly chosen as the build system for new software given its feature set and wide support.

<!-- excerpt start -->

This article explains general concepts and features of GNU Make and includes
recommendations for getting the most out of a Make build! Consider it a brief
guided tour through some of my favorite/most used Make concepts and features 🤗.

<!-- excerpt end -->

If you feel like you already know Make pretty well, feel free to skip the tutorial portion and [jump to my personal recommendations](#recommendations).

{% include toc.html %}

## What is GNU Make?

[GNU Make](https://www.gnu.org/software/make/) is a program that automates the
running of shell commands and helps with repetitive tasks. It is typically used to transform files into some other form, e.g. compiling
source code files into programs or libraries.

It does this by tracking **prerequisites** and executing a hierarchy of commands to
produce **targets**.

Although the GNU Make manual is lengthy, I suggest giving it a read as it is the best reference I've found:
[https://www.gnu.org/software/make/manual/html_node/index.html](https://www.gnu.org/software/make/manual/html_node/index.html)

Let's dive in!

## When to choose Make

Make is suitable for building small C/C++ projects or libraries that would
be included in another project's build system. Most build systems will have a
way to integrate Make-based sub-projects.

For larger projects, you will find a more modern build system easier to work with.

I would suggest a build system other than Make in the following situations:

- When the number of targets (or files) being built is (or will eventually be) in the hundreds.
- A "configure" step is desired, which sets up and persists variables, target definitions, and environment configurations.
- The project is going to remain internal or private and will not need to be built by end users.
- You find debugging a frustrating exercise.
- You need the build to be cross platform that can build on macOS, Linux, and Windows.

In these situations, you might find using
[CMake](https://cmake.org/), [Bazel](https://bazel.build/),
[Meson](https://mesonbuild.com/), or another modern build system a more pleasurable experience.

## Invoking Make

Running `make`  will load a file named `Makefile` from the current directory
and attempt to update the default **goal** (more on goals later).

> Make will search for files named `GNUmakefile`, `makefile`, and `Makefile`, in that
> order

You can specify a particular makefile with the `-f/--file` argument:

```bash
$ make -f foo.mk
```

You can specify any number of *goals* by listing them as positional arguments:

```bash
# typical goals
$ make clean all
```

You can pass Make a directory with the `-C` argument, and this will run Make as if it first `cd`'d into that directory.

```bash
$ make -C some/sub/directory
```

> Fun fact: `git` also can be run with `-C` for the same effect!

### Parallel Invocation

Make can run jobs in parallel if you provide the `-j` or `-l` options. A
guideline I've been told is to set the job limit to 1.5 times the number of
processor cores you have:

```bash
# a machine with 4 cores:
$ make -j 6
```

Anecdotally, I've seen slightly better CPU utilization with the `-l` "load
limit" option, vs. the `-j` "jobs" option. YMMV though!

There are a few ways to programmatically find the CPU count for the current
machine. One easy option is to use the python `multiprocessing.cpu_count()`
function to get the number of threads supported by the system (note on a system
with hyper-threading, this will use up a lot of your machine's resources,
but is probably preferable to letting Make spawn an unlimited number of jobs).

```bash
# call the python cpu_count() function in a subshell
$ make -l $(python -c "import multiprocessing; print(multiprocessing.cpu_count())")
```

{:.no_toc}

#### Output During Parallel Invocation

If you have a lot of output from the commands Make is executing in parallel, you
might see output interleaved on `stdout`. To handle this, Make has the option [`--output-sync`](https://www.gnu.org/software/make/manual/html_node/Parallel-Output.html).

I recommend using **`--output-sync=recurse`**, which will print the entire
output of each target's recipe when it completes, without interspersing other
recipe output.

It also will output an entire recursive Make's output together if your recipe
is using recursive make.

## Anatomy of a Makefile

A Makefile contains **rules** used to produce **targets**. Some basic components of a Makefile are shown below:

```makefile
# Comments are prefixed with the '#' symbol

# A variable assignment
FOO = "hello there!"

# A rule creating target "test", with "test.c" as a prerequisite
test: test.c
	# The contents of a rule is called the "recipe", and is
	# typically composed of one or more shell commands.
	# It must be indented from the target name (historically with
	# tabs, spaces are permitted)

	# Using the variable "FOO"
	echo $(FOO)

	# Calling the C compiler using a predefined variable naming
	# the default C compiler, '$(CC)'
	$(CC) test.c -o test
```

Let's take a look at each part of the example above.

### Variables

Variables are used with the syntax `$(FOO)`, where `FOO` is the variable name.

Variables contain purely strings as Make does not have other data types. Appending to a variable will add a space and the new content:

```makefile
FOO = one
FOO += two
# FOO is now "one two"

FOO = one
FOO = $(FOO)two
# FOO is now "onetwo"
```

#### Variable Assignment

In GNU Make syntax, variables are assigned with two "**flavors**":

1. *recursive expansion*: `variable = expression`
   The expression on the right hand side is assigned verbatim to the variable-
   this behaves much like a macro in C/C++, where the expression is evaluated
   when the variable is used:
   ```makefile
   FOO = 1
   BAR = $(FOO)
   FOO = 2
   # prints BAR=2
   $(info BAR=$(BAR))
   ```
2. *simple expansion*: `variable := expression`
   This assigns the *result* of an expression to a variable; the expression is
   expanded at the time of assignment:
   ```makefile
   FOO = 1
   BAR := $(FOO)
   FOO = 2
   # prints BAR=1
   $(info BAR=$(BAR))
   ```

> Note: the `$(info ...)` function is being used above to print expressions and
can be handy when debugging makefiles!*`

Variables which are not explicitly,
[implicitly](https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html),
nor
[automatically](https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html)
set will evaluate to an empty string.

#### Environment Variables

Environment variables are carried into the Make execution environment. Consider
the following makefile for example:

```makefile
$(info YOLO variable = $(YOLO))
```

If we set the variable `YOLO` in the shell command when running `make`, we'll set the value:

```bash
$ YOLO="hello there!" make
YOLO variable = hello there!
make: *** No targets.  Stop.
```

> Note: Make prints the "No targets" error because our makefile had no targets
listed!

If you use the `?=` assignment syntax, Make will only assign that value if the
variable doesn't already have a value:

Makefile:

```makefile
# default CC to gcc
CC ?= gcc
```

We can then override `$(CC)` in that makefile:

```bash
$ CC=clang make
```

Another common pattern is to allow inserting additional flags. In the makefile,
we would append to the variable instead of directly assigning to it.

```makefile
CFLAGS += -Wall
```

This permits passing extra flags in from the environment:

```bash
$ CFLAGS='-Werror=conversion -Werror=double-promotion' make
```

This can be very useful!

#### Overriding Variables

A special category of variable usage is called **overriding variables**. Using
this command-line option will override the value set **ANYWHERE ELSE** in the
environment or Makefile!

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

Overriding variables can be confusing, and should be used with caution!

#### Target-Specific Variables

These variables are only available in the recipe context. They also apply to any prerequisite recipe!

```makefile
# set the -g value to CFLAGS
# applies to the prog.o/foo.o/bar.o recipes too!
prog : CFLAGS = -g
prog : prog.o foo.o bar.o
	echo $(CFLAGS) # will print '-g'
```

#### Implicit Variables

These are pre-defined by Make (unless overridden with any other variable type of
the same name). Some common examples:

- `$(CC)` - the C compiler (`gcc`)
- `$(AR)` - archive program (`ar`)
- `$(CFLAGS)` - flags for the C compiler

Full list here:

[https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html](https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html)

#### Automatic Variables

These are special variables always set by Make and available in recipe context.
They can be useful to prevent duplicated names (Don't Repeat Yourself).

A few common automatic variables:

```makefile
# $@ : the target name, here it would be "test.txt"
test.txt:
	echo HEYO > $@

# $^ : name of all the prerequisites
all.zip: foo.txt test.txt
	# run the gzip command with all the prerequisites "$^", outputting to the
	# name of the target, "$@"
	gzip -c $^ > $@
```

See more at:
[https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html](https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html)

### Targets (Goals)

Targets are the left hand side in the rule syntax:

```makefile
target: prerequisite
	recipe
```

Targets almost always name **files**. This is because Make uses last-modified
time to track if a target is newer or older than its prerequisites and whether
it needs to be rebuilt!

When invoking Make, you can specify which target(s) you want to build as the
`goal`s by specifying it as a positional argument:

```bash
# make the 'test.txt' and 'all.zip' targets
make test.txt all.zip
```

If you don't specify a goal in the command, Make uses the first target specified
in the makefile, called the "default goal" (you can also
[override](https://www.gnu.org/software/make/manual/html_node/Goals.html) the
default goal if you need to).

#### Phony Targets

Sometimes it's useful to have meta-targets like `all`, `clean`, `test`, etc. In these cases, you don't want Make to check for a file named `all`/`clean` etc.

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
# the 'all' rule that builds and tests. Note that it's listed first to make it
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

> **NOTE!!! `.PHONY` targets are ALWAYS considered out-of-date, so Make will
ALWAYS run the recipe for those targets (and therefore any target that has a
`.PHONY` prerequisite!). Use with caution!!**

#### Implicit Rules

Implicit rules are provided by Make. I find using them to be confusing since
there's so much behavior happening behind the scenes. You will occasionally
encounter them in the wild, so be aware.

Here's a quick example:

```makefile
# this will compile 'test.c' with the default $(CC), $(CFLAGS), into the program
# 'test'. it will handle prerequisite tracking on test.c
test: test.o
```

Full list of implicit rules here:

[https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html](https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html)

#### Pattern Rules

Pattern rules let you write a generic rule that applies to multiple targets via
pattern-matching:

```makefile
# Note the use of the '$<' automatic variable, specifying the first
# prerequisite, which is the .c file
%.o: %.c
	$(CC) -c $< -o $@
```

The rule will then be used to make any target matching the pattern, which above
would be any file matching `%.o`, e.g. `foo.o`, `bar.o`.

If you use those `.o` files mentioned above to build a program:

```makefile
OBJ_FILES = foo.o bar.o

# Use CC to link foo.o + bar.o into 'program'. Note the use of the '$^'
# automatic variable, specifying ALL the prerequisites (all the OBJ_FILES)
# should be part of the link command
program: $(OBJ_FILES)
    $(CC) -o $@ $^
```

### Prerequisites

As seen above, these are targets that Make will check before running a rule. They
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

#### Automatic Prerequisites

A very important consideration for C language projects is to trigger
recompilation if an `#include` header files change for a C file. This is done
with the `-M` compiler flag for gcc/clang, which will output a `.d` file you
will then import with the Make `include` directive.

The `.d` file will contain the necessary prerequisites for the `.c` file so any
header change causes a rebuild. See more details here:

[https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html](https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html)
[http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/](http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/)

The basic form might be:

```makefile
# these are the compiler flags for emitting the dependency tracking file. Note
# the usage of the '$<' automatic variable
DEPFLAGS = -MMD -MP -MF $<.d

test.o: test.c
    $(CC) $(DEPFLAGS) $< -c $@

# bring in the prerequisites by including all the .d files. prefix the line with
# '-' to prevent an error if any of the files do not exist
-include $(wildcard *.d)
```

#### Order-Only Prerequisites

These prerequisites will only be built if they don't exist; if they are newer
than the target, they will not trigger a target re-build.

A typical use is to create a directory for output files; emitting files to a
directory will update its `mtime` attribute, but we don't want that to trigger a
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

### Recipe

The "recipe" is the list of shell commands to be executed to create the target. They are
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
	-rm -r ./build
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
running them, but will otherwise not process it. If you want to access shell
variables, escape them with `$`:

```makefile
USER = linus

print-user:
	# print out the shell variable $USER
	echo $$USER

	# print out the make variable USER
	echo $(USER)
```

## Advanced Topics

These features are less frequently encountered, but provide some powerful
functionality that can enable sophisticated behavior in your build.

### Functions

Make functions are called with the syntax:

```makefile
$(function-name arguments)
```

where `arguments` is a comma-delimited list of arguments.

#### Built-in Functions

There are several functions provided by Make. The most common ones I use are for text
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

#### User-Defined Functions

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

#### Shell Function

You can have Make call a shell expression and capture the result:

```makefile
TODAYS_DATE=$(shell date --iso-8601)
```

I'm cautious when using this feature, though; it adds a dependency on whatever
programs you use, so if you're calling more exotic programs, make sure your
build environment is controlled (e.g. in a container or with
[Conda]({% post_url 2020-01-07-conda-developer-environments %})).

### Conditionals

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

### `include` Directive

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

### Sub-make

Invoking Make from a Makefile should be done with the `$(MAKE)` variable:

```makefile
somelib.a:
	$(MAKE) -C path/to/somelib/directory
```

This is often used when building external libraries. It's also used heavily in Kconfig builds (e.g. when building the Linux kernel).

Note that this approach has some pitfalls:

- Recursive invocation can result in slow builds.
- Tracking prerequisites can be tricky; often you will see `.PHONY` used.

More details on the disadvantages here:

[http://aegis.sourceforge.net/auug97.pdf](http://aegis.sourceforge.net/auug97.pdf)

### Metaprogramming with `eval`

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

### VPATH

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

I recommend avoiding use of `VPATH`. It's usually simpler to achieve the same
out-of-tree behavior by outputting the generated files in a build directory
without needing `VPATH`.

### `touch`

You may see the `touch` command used to track rules that seem difficult to
otherwise track; for example, when unpacking a toolchain:

```makefile
# our tools are stored in tools.tar.gz, and downloaded from a server
TOOLS_ARCHIVE = tools.tar.gz
TOOLS_URL = https://httpbin.org/get

# the rule to download the tools using wget
$(TOOLS_ARCHIVE):
	wget $(TOOLS_URL) -O $(TOOLS_ARCHIVE)

# rule to unpack them
tools-unpacked.dummy: $(TOOLS_ARCHIVE)
	# running this command results in a directory.. but how do we know it
	# completed, without a file to track?
	tar xzvf $^
	# use the touch command to record completion in a dummy file
	touch $@
```
I recommend avoiding the use of `touch`. However there are some cases where it
might be unavoidable.

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

### Profiling

For **profiling** a make invocation (e.g. for attempting to improve compilation
times), this tool can be useful:

[https://github.com/rocky/remake](https://github.com/rocky/remake)

Check out the tips here for compilation-related performance improvements:

[https://interrupt.memfault.com/blog/improving-compilation-times-c-cpp-projects]({% post_url 2020-02-11-improving-compilation-times-c-cpp-projects %})

### Using a Verbose Flag

If your project includes a lot of compiler flags (search paths, lots of warning
flags, etc.), then you may want to simplify the output of Make rules. It can be
useful to have a toggle to easily see the full output, for example:

```makefile
ifeq ($(V),1)
Q :=
else
Q := @
endif

%.o: %.c
	# prefix the compilation command with the $(Q) variable
	# use echo to print a simple "Compiling x.c" to show progress
	@echo Compiling $(notdir @^)
	$(Q) $(CC) -c $^ -o $@
```

To enable printing out the full compilation commands, set the `V` environment
variable like so:

```bash
$ V=1 make
```

## Full Example

Here's an annotated example of a complete build process for an example C
project. You can see this example and the source tree
[here](https://github.com/memfault/interrupt/tree/master/example/gnu-make-guidelines).

```makefile
# Makefile for building the 'example' binary from C sources

# Verbose flag
ifeq ($(V),1)
Q :=
else
Q := @
endif

# The build folder, for all generated output. This should normally be included
# in a .gitignore rule
BUILD_FOLDER := build

# Default all rule will build the 'example' target, which here is an executable
.PHONY:
all: $(BUILD_FOLDER)/example

# List of C source files. Putting this in a separate variable, with a file on
# each line, makes it easy to add files later (and makes it easier to see
# additions in pull requests). Larger projects might use a wildcard to locate
# source files automatically.
SRC_FILES = \
    src/example.c \
    src/main.c

# Generate a list of .o files from the .c files. Prefix them with the build
# folder to output the files there
OBJ_FILES = $(addprefix $(BUILD_FOLDER)/,$(SRC_FILES:.c=.o))

# Generate a list of depfiles, used to track includes. The file name is the same
# as the object files with the .d extension added
DEP_FILES = $(addsuffix .d,$(OBJ_FILES))

# Flags to generate the .d dependency-tracking files when we compile.  It's
# named the same as the target file with the .d extension
DEPFLAGS = -MMD -MP -MF $@.d

# Include the dependency tracking files
-include $(DEP_FILES)

# List of include dirs. These are put into CFLAGS.
INCLUDE_DIRS = \
    src/

# Prefix the include dirs with '-I' when passing them to the compiler
CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

# Set some compiler flags we need. Note that we're appending to the CFLAGS
# variable
CFLAGS += \
    -std=c11 \
    -Wall \
    -Werror \
    -ffunction-sections -fdata-sections \
    -Og \
    -g3

# Our project requires some linker flags: garbage collect sections, output a
# .map file
LDFLAGS += \
    -Wl,--gc-sections,-Map,$@.map

# Set LDLIBS to specify linking with libm, the math library
LDLIBS += \
    -lm

# The rule for compiling the SRC_FILES into OBJ_FILES
$(BUILD_FOLDER)/%.o: %.c
	@echo Compiling $(notdir $<)
	@# Create the folder structure for the output file
	@mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# The rule for building the executable "example", using OBJ_FILES as
# prerequisites. Since we're not relying on an implicit rule, we need to
# explicitly list CFLAGS, LDFLAGS, LDLIBS
$(BUILD_FOLDER)/example: $(OBJ_FILES)
	@echo Linking $(notdir $@)
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Remove debug information for a smaller executable. An embedded project might
# instead using [arm-none-eabi-]objcopy to convert the ELF file to a raw binary
# suitable to be written to an embedded device
STRIPPED_OUTPUT = $(BUILD_FOLDER)/example-stripped

$(STRIPPED_OUTPUT): $(BUILD_FOLDER)/example
	@echo Stripping $(notdir $@)
	$(Q)objcopy --strip-debug $^ $@

# Since all our generated output is placed into the build folder, our clean rule
# is simple. Prefix the recipe line with '-' to not error if the build folder
# doesn't exist (the -f flag for rm also has this effect)
.PHONY: clean
clean:
	- rm -rf $(BUILD_FOLDER)
```

## Recommendations

A list of recommendations for getting the most of Make:

1. Targets should usually be real files.
2. Always use `$(MAKE)` when issuing sub-make commands.
3. Try to avoid using `.PHONY` targets. If the rule generates any file artifact,
   consider using that as the target instead of a phony name!
4. Try to avoid using implicit rules.
5. For C files, make sure to use `.d` automatic include tracking!
6. Use metaprogramming with caution.
7. Use automatic variables in rules. _Always_ try to use `$@` for a recipe
   output path, so your rule and Make have the exact same path.
8. Use comments liberally in Makefiles, especially if there is complicated
   behavior or subtle syntax used. Your co-workers (and future self) will thank
   you.
9. Use the `-j` or `-l` options to run Make in parallel!
10. Try to avoid using the `touch` command to track rule completion

## Outro

I hope this article has provided a few useful pointers around GNU Make!

Make remains common in C language projects, most likely due to its usage in the
Linux kernel. Many recently developed statically compiled programming
languages, such as Rust or Go, provide their own build infrastructure. However, when
integrating Make-based software into those languages, for example when building
a C library to be called from Rust, it can be surprisingly helpful to understand
some Make concepts!

You may also encounter [automake](https://www.gnu.org/software/automake/) in
open source projects (look for a `./configure` script). This is a related tool
that generates Makefiles, and is worth a look (especially if you are writing
C software that needs to be very widely portable).

There are many competitors to GNU Make available today, I encourage everyone to
look into them. Some examples:

- [CMake](https://cmake.org/) is pretty popular (the Zephyr project uses this)
  and worth a look. It makes out-of-tree builds pretty easy
- [Bazel](https://docs.bazel.build/versions/master/tutorial/cpp.html) uses a
  declarative syntax (vs. Make's imperative approach)
- [Meson](https://mesonbuild.com/) is a meta-builder like `cmake`, but by
  default uses Ninja as the backend, and can be very fast

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
