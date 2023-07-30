---
title: "Using SWIG to generate bindings between C and Lua"
description: Trying out SWIG to automate generation of binding layer between Lua and C worlds
author: stawiski
tags: [c]
---

Lua is one of the many great interpreters that can be run on embedded devices. It's fast, uses little memory, is written in ANSI C, and is known by plenty of developers. For these reasons, many great teams are choosing to include a Lua interpreter in their embedded project (e.g. Panic with [their Playdate device](https://play.date/dev/)). You can think of Lua as an alternative to the MicroPython (Python) or JerryScript (Javascript) interpreters. However, there's a problem. Many of the libraries today for embedded devices are written in C, not Lua!

There are ways to make Lua and C work together and share data structures, but it requires a lot of boilerplate and complex code. The result is writing a lot of wrapper code integrating the two languages together that then must be maintained as the application evolves.

Thankfully, there is a project which helps automate the generation of this boilerplate code called SWIG.

<!-- excerpt start -->

This article covers how to write a C program that launches a Lua interpreter and then how to use SWIG to generate the necessary wrapper code to allow Lua scripts to access the functions and data inside of the C runtime.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What is SWIG?

[SWIG](https://www.swig.org/) is a tool that generates a binding layer between C/C++ and higher-level programming languages. The promise of SWIG is simple: write an interface file that annotates C/C++ headers with custom SWIG markers, and generates bindings to a given language, so that you can expose whatever functionality you want from C/C++ and use it in that language.

Let's explore generating bindings between applications written in C targeted for embedded hardware and Lua.

## Setting up

> To follow along, I've provided all example code on [GitHub in the Interrupt repo under `example/swig-for-c-and-lua`](https://github.com/memfault/interrupt/tree/master/example/swig-for-c-and-lua).

We’ll start by writing a Dockerfile that will host our desktop environment including Lua, SWIG, CMake, and GCC. Let’s build on top of `alpine:3.16` image for a smaller size.

First, let’s get Lua 5.4.4 in:

```dockerfile
FROM alpine:3.16

WORKDIR /app

RUN apk add git make curl gcc tar
RUN apk add libc-dev

RUN curl -R -O http://www.lua.org/ftp/lua-5.4.4.tar.gz
RUN tar zxf lua-5.4.4.tar.gz
RUN rm lua-5.4.4.tar.gz
RUN cd lua-5.4.4 && make all test && make install
```

Then add SWIG 4.1.0, GCC, CMake:

```dockerfile
RUN apk add git make automake autoconf libtool
RUN git clone https://github.com/swig/swig --branch v4.1.0

RUN apk add pcre2-dev
RUN apk add bison flex
RUN apk add gcc g++

RUN cd swig && ./autogen.sh && ./configure && make
RUN cd swig && make install

RUN apk add cmake
```

And finally, check if both Lua and SWIG programs are available (if any of these commands fails the Docker image build will fail too):

```dockerfile
RUN luac -v
RUN swig -version
```

Here is the whole resulting Dockerfile for reference ([link](https://github.com/memfault/interrupt/tree/master/example/swig-for-c-and-lua/Dockerfile)):

```dockerfile
FROM alpine:3.16

WORKDIR /app

RUN apk add git make curl gcc tar
RUN apk add libc-dev

RUN curl -R -O http://www.lua.org/ftp/lua-5.4.4.tar.gz
RUN tar zxf lua-5.4.4.tar.gz
RUN rm lua-5.4.4.tar.gz
RUN cd lua-5.4.4 && make all test && make install

RUN apk add git make automake autoconf libtool
RUN git clone https://github.com/swig/swig --branch v4.1.0

RUN apk add pcre2-dev
RUN apk add bison flex
RUN apk add gcc g++

RUN cd swig && ./autogen.sh && ./configure && make
RUN cd swig && make install

RUN apk add cmake

# Check installed programs
RUN luac -v
RUN swig -version

ENTRYPOINT ["/bin/sh"]
```

To build and run this container, we can run the following command in our shell:

```shell
# Host machine
$ docker build -t swig . && docker run -v $PWD:/app -it swig

... # Build output

# Docker container shell starts!
/app # ls
Dockerfile   example1     ...
```

Now, using this Docker image we can parse Lua scripts, compile Lua to bytecode, generate bindings with SWIG, and compile C/C++ programs using CMake!

To make things as easy for you to replicate what I'm doing in this post, I've created shell scripts for each example, such as `example1.sh`, that can be run from your host machine with Docker installed.

## First binding between C and Lua

Time to get our hands dirty with SWIG. Let’s get something simple running. For reference this is `example1`.

Imagine that we have a function that we want to use from Lua that multiplies two fixed-size integers in C with this signature:

```c
int32_t multiply(int32_t x, int32_t y);
```

Let’s write a SWIG interface file, `bindings.i` and annotate the above function for SWIG:

```
%module bindings

%include "stdint.i"

%inline %{
extern int32_t multiply(int32_t x, int32_t y);
%}
```

We declared `multiply` as `extern` introducing a linkage dependency. Now we’ll run SWIG and let it work its magic:

```
/app # cd example1
/app/example1 # swig -lua -o swig.c bindings.i
```

SWIG generated a huge C file we called `swig.c`, which consists of all the boilerplate code. Looking into it for `multiply` we can find this auto-generated code:

```c
extern int32_t multiply(int32_t x, int32_t y);

#ifdef __cplusplus
extern "C" {
#endif
static int _wrap_multiply(lua_State* L) {
  int SWIG_arg = 0;
  int32_t arg1 ;
  int32_t arg2 ;
  int32_t result;

  SWIG_check_num_args("multiply",2,2)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("multiply",1,"int32_t");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("multiply",2,"int32_t");
  arg1 = (int32_t)lua_tonumber(L, 1);
  arg2 = (int32_t)lua_tonumber(L, 2);
  result = (int32_t)multiply(arg1,arg2);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;

  fail: SWIGUNUSED;
  lua_error(L);
  return 0;
}
```

It’s a neat wrapper for our `multiply` C function! This module contains a library that we need to load into our Lua instance to be able to access this functionality. Since we named our module `bindings`, we’ll need to call `luaopen_bindings` .

Now let’s shift focus to our C code. Here are the rough steps that we need to take to be able to call the `multiply` function from within Lua:

- Define our `multiply` function that Lua will call
- Create a new Lua instance
- Load library generated by SWIG
- Call Lua file

That’s it!

Putting this together, we end up with:

```c
int32_t multiply(int32_t x, int32_t y)
{
    int32_t result = x * y;
    printf("[C] Multiply %d * %d = %d\r\n", x, y, result);
    return result;
}

int main(void)
{
    lua_State *L = luaL_newstate();
    assert(L != NULL);

    luaL_openlibs(L);
    luaopen_bindings(L); // Load the wrapped module

    int r = luaL_loadfile(L, "example1.lua");
    assert(r == LUA_OK);

    printf("[C] Calling Lua\r\n");

    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        const char *msg = lua_tostring(L, -1);
        printf("[LUA] Error: %s\n", msg);
    }

    printf("[C] We're back from Lua\r\n");
    lua_close(L);
    printf("[C] Finished\r\n");
}
```

Let's now write our Lua script. From this script, we want to call `multiply` from C world. To do that we will use a `bindings` library that we generated and loaded, which exposes `multiply` as `bindings.multiply`. The call will be as simple as:

```lua
print("[Lua] Result of multiply -2 * 5 = " .. bindings.multiply(-2, 5))
```

We can also add a check if the `bindings` module is indeed loaded in Lua by adding an `assert`:

```lua
assert(type(bindings) == 'table', "Binding module not loaded")
```

Time to compile and run our first binding. Remember to do that from a directory that contains `example1.lua` so that the program can find this file.

```
/app # cd example1
/app/example1 # cmake -S . -B build
/app/example1 # cmake --build build
/app/example1 # ./build/example1
[C] Calling Lua
[C] Multiply -2 * 5 = -10
[Lua] Result of multiply -2 * 5 = -10.0
[C] We're back from Lua
[C] Finished
```

Our first binding between Lua and C worked! However, this is a trivial hello-world type of example.

Time to try something more complicated.

## Passing a custom C struct to Lua

We have called a simple C function from Lua, but what about passing struct data? As our application evolves, we’ll most likely add custom types and complicated data structures - the source of the massive amount of wrapper code that exists in many repositories that use Lua from C host program. For reference this is `example2`.

Let’s start with defining our data types in C header `types.h`:

```c
typedef enum
{
    LEVEL_NONE = 0,
    LEVEL_LOW,
    LEVEL_MEDIUM,
    LEVEL_HIGH,

} LEVEL_enum_t;

typedef struct
{
    LEVEL_enum_t level;
    uint8_t priority;
    char message[64];
    bool isReady;
} my_struct_t;
```

And a SWIG interface, `bindings.i`, that will include the above C header:

```
%module bindings

%{
#include "types.h"
%}

%include "types.h"
```

If we look into the generated C file from SWIG, we’ll see setters and getters for each of the fields of the structure, as well as a constructor and destructor for the struct.

Assuming we’d not only like to receive a structure passed from C in Lua but also modify it, we’ll write a Lua script that will drive our C implementation:

```lua
function processStruct(struct)
    -- Print our structure
    print("[Lua] struct.level = " .. tostring(struct.level))
    print("[Lua] struct.priority = " .. tostring(struct.priority))
    print("[Lua] struct.message = " .. struct.message)
    print("[Lua] struct.isReady = " .. tostring(struct.isReady))

    -- Now let's modify the structure
    struct.level = bindings.LEVEL_NONE
    struct.priority = 99
    struct.message = "Hey from Lua"
    struct.isReady = false
end
```

We introduced `processStruct` function, which we will call from C and pass a C structure to it, and expect the structure to be modified by Lua.

To call this Lua function from C we need to write a piece of C code using APIs from SWIG runtime header. First, we will generate the runtime header:

```
/app/example2 # swig -lua -external-runtime swig_runtime.h
```

Then write a C function to call Lua’s `processStruct` function:

```c
void callProcessStruct(lua_State *L, my_struct_t *my_struct)
{
    lua_getglobal(L, "processStruct");
    assert(lua_isfunction(L, -1));

    swig_type_info *type = SWIG_TypeQuery(L, "my_struct_t *");
    assert(type != NULL);
    SWIG_NewPointerObj(L, my_struct, type, false);

    // Call function with 1 arguments and no result
    assert(lua_pcall(L, 1, 0, 0) == LUA_OK);
}
```

Here, in `callProcessStruct` we first grab Lua’s `processStruct` function on top of the stack. Then, we get type info for our C structure using `SWIG_TypeQuery` , followed by pushing the pointer on the stack with `SWIG_NewPointerObj`. That call creates a userdata on Lua’s stack containing our C structure. Finally, we call our Lua function.

The rest of the application is just setting up the C structure with initial values, and printing the structure after the call to Lua:

```c
my_struct_t my_struct;
my_struct.isReady = true;
my_struct.level = LEVEL_MEDIUM;
strncpy(my_struct.message, "This is example message", sizeof(my_struct.message));
my_struct.priority = 100;

printf("[C] Calling processStruct\r\n");
callProcessStruct(L, &my_struct);
printf("[C] We're back from Lua\r\n");

printf("[C] Printing my_struct\r\n");
printf("\tmy_struct.level = %d\r\n", my_struct.level);
printf("\tmy_struct.priority = %d\r\n", my_struct.priority);
printf("\tmy_struct.message = %s\r\n", my_struct.message);
printf("\tmy_struct.isReady = %s\r\n", my_struct.isReady ? "true" : "false");
```

Here’s the output:

```
[C] Calling processStruct
[Lua] struct.level = 2.0
[Lua] struct.priority = 100.0
[Lua] struct.message = This is example message
[Lua] struct.isReady = true
[C] We're back from Lua
[C] Printing my_struct
        my_struct.level = 0
        my_struct.priority = 99
        my_struct.message = Hey from Lua
        my_struct.isReady = false
[C] Finished
```

It worked! Lua was able to modify our enum, integer, string, and boolean. To get a peek under the hood, let’s use a well-known Lua [inspect](https://github.com/kikito/inspect.lua) library, which gives us information about objects.

After importing the library with `require`, we add two lines to `processStruct`:

```lua
print("[Lua] struct: " .. inspect(struct))
print("[Lua] struct metatable: " .. inspect(getmetatable(struct)))
```

And get the following print (truncated):

```
[Lua] struct: <userdata 1>
[Lua] struct metatable: <1>{
...
  [".get"] = {
    isReady = <function 2>,
    level = <function 3>,
    message = <function 4>,
    priority = <function 5>
  },
	[".set"] = {
    isReady = <function 6>,
    level = <function 7>,
    message = <function 8>,
    priority = <function 9>
  },
...
```

You can see from Lua's point of view, the object is Lua’s [userdata](https://www.lua.org/pil/28.1.html), which means it’s raw memory. Normally userdata is managed by Lua meaning it will be garbage collected. However, SWIG gives us flexibility here with its `SWIG_NewPointerObj` API. The last argument to that function specified the owner of the object, and since we decided to own the object the garbage collector won’t affect it.

The interesting bit is our struct’s metatable. As you can see from the print, the metatable contains function pointers including getters and setters. This is the main part SWIG took care of for us.

And if you’re wondering whether a known sized buffer, like the string buffer `char message[64];` can overflow, SWIG handles that and would just truncate the string as seen in the generated `swig.c`:

```c
strncpy((char*)arg1->message, (const char *)arg2, 64-1);
arg1->message[64-1] = 0;
```

## SWIG shortfalls - what it doesn’t automate

Ok, that’s great so far! But what about arrays? We passed a string, an array of `char`s but what about an array of numbers or custom types? Here’s where things don’t sail as smoothly.

Consider a modification to our structure:

```c
typedef struct
{
    uint8_t priorities[3];

} my_struct_t;
```

Now the structure contains a known size array of numbers. Following the previous example, let’s inspect `struct.priorities` from Lua using:

```lua
print("[Lua] struct.priorities: " .. inspect(struct.priorities))
print("[Lua] struct.priorities metatable: " .. getmetatable(struct.priorities))
```

Output:

```
[Lua] struct.priorities: <userdata 1>
[Lua] struct.priorities metatable: nil
```

Our array `priorities` is userdata and it doesn’t have an assigned metatable. Therefore Lua cannot do anything with it! That’s not something we expected from a generator tool, after all we’re talking about handling the simplest possible array.

And it is a common problem, even SWIG documentation states:

> Arrays present a challenge for SWIG, because like pointers SWIG does not know whether these are input or output values, nor does SWIG have any indication of how large an array should be.
>

To address this problem we can write a typemap in our SWIG interface file, which will include our custom code to push the array onto Lua stack:

```
%typemap(out) uint8_t my_struct_t::priorities[3]
{
    lua_newtable(L);
    for (uint32_t i = 0; i < 3; i++)
    {
        lua_pushnumber(L, (lua_Number)$1[i]);
        lua_rawseti(L, -2, i + 1); // Lua indexes start from 1
    }

    SWIG_arg++;
}
```

However, this becomes tedious as now we’re writing the exact boilerplate code we were trying to avoid!

This time, inspection of `priorities` array gives:

```
[Lua] struct.priorities: { 1.0, 1.0, 1.0 }
[Lua] struct.priorities metatable: nil
```

Now Lua knows it’s a table, but we’re limited to read-only access. To have the ability to modify the array members we’d, unfortunately, need to write a custom modifier code.

## Closing thoughts

After choosing Lua for an embedded project it quickly became clear how much time we’ll need to spend binding two different worlds of Lua and C together. Looking at an inspiring example of [nodemcu firmware](https://github.com/nodemcu/nodemcu-firmware), which provides a Lua-based firmware for Espressif processors, it became clear we’ll have lots of code that glues functions and data structures for both languages to understand each other.

As we started to write wrapper code, translating C arrays into Lua tables and naming structure fields manually, we found out about SWIG which promised to help with this task. At first, things were smooth as we were learning how to write a SWIG interface file, and getting simple examples to work. However, foreseeing what we’ll need to do in our application in the future and demoing passing of structures with arrays of custom types, the magic of SWIG quickly disappeared and we realized we’ll still have to write all that boilerplate code anyways, just in a SWIG interface file.

We also noticed that SWIG sneakily added dynamic memory allocation like `calloc` calls, which we want to have full control over in the embedded domain. To address this issue the solution is, again, to extend the interface file and take control of the object creation. Instead, we would have hoped one could just specify a custom memory allocator to configure SWIG with.

To sum up, there’s no doubt SWIG is a useful tool that aids with code generation, but it didn’t fill our needs in an embedded application, and we would rather take control over the whole boilerplate code than use a tool that still needs to be helped with.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- <https://www.lua.org/manual/5.4/>
- <https://www.swig.org/Doc4.1/SWIGDocumentation.html>
- <https://github.com/swig/swig>
- <https://github.com/kikito/inspect.lua>
- <https://stackoverflow.com/questions/15175859/how-to-inspect-userdata-in-lua>
- <https://stackoverflow.com/questions/4329643/what-is-userdata-and-lightuserdata-in-lua>
- <https://code.activestate.com/lists/python-dev/109281>

<!-- prettier-ignore-end -->
