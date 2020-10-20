---
title: Advanced GDB
description: TODO
author: tyler
---

We are going to talk about GDB in depth in this post.

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

Optional motivation to continue onwards

{% include newsletter.html %}

{% include toc.html %}

## Help Menus

There's no better place to start than first teaching how to fish within GDB. Surprisingly, and maybe unsurprisingly, GDB has over 1500+ commands!

```
$ gdb --batch -ex 'help all' | grep '\-\-' | wc
    1512   16248  117306
```

With this in mind, the most useful command within GDB is one that searches all the "help" menus of each command.

```
(gdb) help apropos
Search for commands matching a REGEXP
```

To use it, simply type `apropos <regex>

```
(gdb) apropos symbol
add-symbol-file -- Load symbols from FILE, assuming FILE has been dynamically loaded.
add-symbol-file-from-memory -- Load the symbols out of memory from a dynamically loaded object file.
attach -- Attach to a process or file outside of GDB.
...
```

## Source Files

### Search Paths

Many times, the CI system builds with absolute paths instead of relative paths, which causes GDB not to be able to find the source files. 

```
(gdb) f 1
#2  cli_state_collect (p_cli=0x3ad00 <m_cli>) at ../nrf5_sdk/components/libraries/cli/nrf_cli.c:1952
1952	../nrf5_sdk/components/libraries/cli/nrf_cli.c: No such file or directory.
```

If you are proactive and want to fix this permanently in the build step, you can follow the steps in a previous post about [Reproducible Firmware Builds]({% post_url 2019-12-11-reproducible-firmware-builds %}) to make the paths relative. 

If you want to patch it up now in GDB, you can use a combination of the `set substitute-path` and `directory` commands in GDB, depending on how the paths are built.

To fix the above issue, all I needed to do was to add my local directory. After adding it, GDB can find the source code of the line in the frame.

```
(gdb) directory sdk/embedded/platforms/nrf5/nrf5_sdk/
Source directories searched: /[...]/sdk/embedded/platforms/nrf5/nrf5_sdk:$cdir:$cwd
(gdb) f 1
#1  0x000292a2 in cli_execute (p_cli=0x3ad00 <m_cli>) at ../nrf5_sdk/components/libraries/cli/nrf_cli.c:2554
2554	        p_cli->p_ctx->p_current_stcmd->handler(p_cli,
```

[Reference](https://sourceware.org/gdb/current/onlinedocs/gdb/Source-Path.html)

### `list` Command

To quickly view ten lines of source-code context within GDB, you can use the `list` command. 

```
(gdb) list
2549	                &p_static_entry,
2550	                &static_entry);
2551
2552	        p_cli->p_ctx->p_current_stcmd = p_static_entry;
2553
2554	        p_cli->p_ctx->p_current_stcmd->handler(p_cli,
2555	                                               argc - cmd_handler_lvl,
2556	                                               &argv[cmd_handler_lvl]);
2557	    }
2558	    else if (handler_cmd_lvl_0 != NULL)
```

If you want to set a larger or smaller default number of lines shown with this command, you can change the setting:

```
(gdb) set listsize 20
```

[Reference](https://sourceware.org/gdb/current/onlinedocs/gdb/List.html)

### Viewing Assembly With `disasseble`

It's useful to dive into the assembly of a specific function. GDB has this capability built-in and can even interleave the source code with the assembly (by using the `/s` option).

```
(gdb) disassemble /s nrf_cli_cmd_echo_on
Dump of assembler code for function nrf_cli_cmd_echo_on:
nrf_cli.c:
3511	{
3512	    if (nrf_cli_build_in_cmd_common_executed(p_cli, (argc != 1), NULL, 0))
   0x0002969c <+0>:	ldr	r3, [r0, #8]
   0x0002969e <+2>:	ldr.w	r2, [r3, #316]	; 0x13c

nrf_cli.h:
604	    return p_cli->p_ctx->internal.flag.show_help;
   0x000296a2 <+6>:	lsls	r2, r2, #30
   0x000296a4 <+8>:	bmi.n	0x296b8 <nrf_cli_cmd_echo_on+28>

nrf_cli.c:
3391	    if (arg_cnt_nok)
   0x000296a6 <+10>:	cmp	r1, #1
   0x000296a8 <+12>:	bne.n	0x296bc <nrf_cli_cmd_echo_on+32>
```

If you want something more powerful than `disassemble`, you can use `objdump` itself. Interrupt's [post on GNU Binutils]({% post_url 2020-04-08-gnu-binutils %}#objdump) is a great reference for this.

[Reference](http://sourceware.org/gdb/current/onlinedocs/gdb/Machine-Code.html)

## GDB Visual Interfaces

### GDB TUI

GDB has a built-in graphical interface for viewing source code, registers, assembly, and other various items, and it's quite simple to use!

![]({% img_url advanced-gdb/gdb-tui.png %})

You can start the TUI interface by running:

```
(gdb) tui enable
```

This should give you the source code viewer. You are now able to type `next`, `step`, `continue`, etc. and the TUI interface will update and follow along.

If you want to view the assembly as well, you can run:

```
(gdb) layout split
```

To show the registers at the top of the window, you can type:

```
(gdb) tui reg general
```

### Alternate GDB Interfaces

Although TUI is nice, I don't know many people who actually use it. It's great for quick observations and exploration, but I would suggest spending the 15-30 minutes to get something set up that is more powerful and permanent. 

I would suggest looking into using [gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard) or [Conque-GDB](https://github.com/vim-scripts/Conque-GDB) if you'd like to stick with GDB itself.

For a full list of GUI enhancements to GDB, you can check out the list of plugins from my [previous post on GDBundle]({% post_url 2020-04-14-gdbundle-plugin-manager %}#neat-gdb-script-repositories).

### gdbgui

[gdbgui](https://www.gdbgui.com/) is a browser-based frontend to GDB, built by Chad Smith, who also happens to be the author of another one of my favorite tools, [pipx](https://github.com/pipxproject/pipx).

![](https://raw.githubusercontent.com/cs01/gdbgui/master/screenshots/gdbgui.png)

It's a really great tool if you don't want to use a vendor-provided debugger but still want to have a visual way to interface with GDB.

### VSCode

VSCode has pretty good support for GDB baked in. If you are just using GDB to debug C/C++ code locally on your machine, you should be able to follow the [official instructions](https://code.visualstudio.com/docs/cpp/cpp-debug).

If you are looking to do remote debugging on devices connected with OpenOCD, PyOCD, or JLink, you'll want to look into the [Platform.io](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) or [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) extensions.

## Registers

### Printing Registers

You can print the register values within GDB using `info registers`. 

```
(gdb) info registers
r0             0x2                 2
r1             0x7530              30000
r2             0x10000194          268435860
r3             0x1                 1
r4             0x8001dd9           134225369
r5             0xa5a5a5a5          -1515870811
r6             0xa5a5a5a5          -1515870811
r7             0x20002ad8          536881880
r8             0xa5a5a5a5          -1515870811
r9             0xa5a5a5a5          -1515870811
r10            0xa5a5a5a5          -1515870811
r11            0xa5a5a5a5          -1515870811
r12            0xffffffff          -1
sp             0x20002ad8          0x20002ad8 <ucHeap1.14288+7812>
lr             0x8026563           134374755
pc             0x802f03c           0x802f03c <vPortEnterCritical+32>
xpsr           0x10d0000           17629184
fpscr          0x0                 0
msp            0x20017fe0          0x20017fe0
psp            0x20002ad8          0x20002ad8 <ucHeap1.14288+7812>
primask        0x0                 0
control        0x6                 6
```

You can also use `info registers all` if there are more registers on your system that aren't printed by default, but do note they may or may not be valid values (depending on your system and gdbserver).

### Register Variables

GDB provides you access to the values of the system registers in the form of variables, such as `$pc` or `$sp` for the program counter or stack pointer.

When we want to reference these registers directly, we can just use `$<register_name>`!

```
(gdb) p $sp
$2 = (void *) 0x20002ad8 <ucHeap1.14288+7812>
(gdb) p $pc
$3 = (void (*)()) 0x802f03c <vPortEnterCritical+32>
```

Notice these are the same values as above when we used `info registers`.

#### Conditional Breakpoints and Watchpoints

A conditional breakpoint in GDB follows the format `break WHERE if CONDITION`. It will only bubble up a breakpoint to the user if `CONDITION` is true.

Let's imagine we are trying to triage a reproducible memory corruption bug. You don't exactly know when or how a `num_samples` argument is being corrupted with the value of `0xdeadbeef` when the function `compute_fft` is called. We can improve our investigation using conditional breakpoints.

```
(gdb) break compute_fft if num_samples == 0xdeadbeef
```

You can also use register values (`$sp`, `$pc`, etc.) in the conditional expressions as well.

Note that the expressions are evaluated host side within GDB, so the system halts every time, but GDB will only prompt the user when the expression is true. Therefore, system performance might be impacted.

You can also do the same with watchpoints, which will only prompt the user in GDB if the conditional is true.

```
(gdb) watch i if i == 100

(gdb) info watchpoints
Num     Type           Disp Enb Address    What
1       hw watchpoint  keep y              i stop only if i == 100
```

#### Backtrace for All Threads

To quickly gain an understanding of all of the threads, you can print the backtrace of all threads using the following: 

```
(gdb) thread apply all bt

# Shortcut
(gdb) taa bt
```

In my personal `.gdbinit`, I have the following alias set to `btall`.

```
# Print backtrace of all threads
define btall
thread apply all backtrace
end
```
### Memory

#### Listing Memory Regions

It's possible to list the memory regions of the binary currently being debugging by running the `info files` command. 

```
(gdb) info files
Symbols from "symbols/zephyr.elf".
Local exec file:
	`symbols/zephyr.elf', file type elf32-littlearm.
	Entry point: 0x8009e94
	0x08000000 - 0x080001c4 is text
	0x080001d0 - 0x0802cb6e is _TEXT_SECTION_NAME_2
	0x0802cb70 - 0x0802cb78 is .ARM.exidx
	0x0802cb78 - 0x0802ce08 is sw_isr_table
	0x0802ce08 - 0x0802cfac is devconfig
	0x0802cfac - 0x0802cfb8 is net_socket_register
	0x0802cfb8 - 0x0802d098 is log_const_sections
	0x0802d098 - 0x0802d0a8 is log_backends_sections
	0x0802d0a8 - 0x0802d0e8 is shell_root_cmds_sections
	0x0802d0e8 - 0x08036e98 is rodata
	0x20000000 - 0x2000a1f5 is bss
	0x2000a200 - 0x2001112b is noinit
	0x2001112c - 0x200113f1 is datas
  ...
	0x200117c0 - 0x200117e0 is net_if_dev
```

This is especially helpful when you are trying to figure out exactly where a variable exists in memory. 

#### Examing Memory using `x`

Many developers know how to use GDB's `print`, but less know about the more powerful `x` (for "e__x__amine") command. The `x` command is used to examine memory using several formats. 

My most common use of `x` is looking at the stack memory of a thread that has faulted due to a stack overflow.

```
"HardFault_Handler () at memfault/sdk/embedded/src/memfault_fault_handling.c:123
123	memfault/sdk/embedded/src/memfault_fault_handling.c: No such file or directory.
(gdb) bt
#0  HardFault_Handler () at memfault/sdk/embedded/src/memfault_fault_handling.c:123
#1  <signal handler called>
#2  0x0badcafe in ?? ()
#3  0x000292a2 in cli_execute (p_cli=0x3ad00 <m_cli>) at ../nrf5_sdk/components/libraries/cli/nrf_cli.c:2554
#4  cli_state_collect (p_cli=<optimized out>) at ../nrf5_sdk/components/libraries/cli/nrf_cli.c:1952
#5  nrf_cli_process (p_cli=<optimized out>) at ../nrf5_sdk/components/libraries/cli/nrf_cli.c:2852
#6  0x22334454 in ?? ()
```
As you can see, we don't get much information about what happened during this hard fault. But we can print the contents of the stack by using `x/a $sp` 

```

```

#### Searching Memory using `find`

Sometimes you know a pattern that you are looking for in memory, and you want to quickly find out if it exists in memory on the system. Maybe it's a magic string or a specific 4 byte pattern, like `0xdeadbeef`. 

Let's search for the string `shell_uart`, which is the task name of a thread in my Zephyr system. I'll search the entire writeable RAM space, which can be found by running the `info files` command mentioned previously.

```
(gdb) find 0x20000000, 0x200117e0, "shell_uart"
0x2000121c <shell_uart_thread+104>
1 pattern found.
```

If we use `x/s` to examine that memory, we can see that it is indeed `shell_uart`.

```
(gdb) x/s 0x2000121c
0x2000121c <shell_uart_thread+104>:	"shell_uart"
```

The `find` command can also be useful for finding pointers pointing to arbitrary structs. For example, I want to find all the pointers that contain a reference to the variable `mgmt_thread_data`.

```
(gdb) find 0x20000000, 0x200117e0, &mgmt_thread_data
0x20002a18 <tx_classes+120>
0x2000d068 <mgmt_stack+648>
0x2000d074 <mgmt_stack+660>
0x2000d088 <mgmt_stack+680>
0x2001169c <network_event>
0x200116a0 <network_event+4>
6 patterns found.
```

It looks like there are some references on the `mgmt_stack`, and a few other references, which are actually pointers from linked lists. 

Using `find` can help track down memory leaks, memory corruption, and possible hard faults by seeing what pieces of system are continuing to reference memory or values when they shouldn't.

## Variables

### Searching for Variables

To print the local and argument varialbes, we can use `info locals` and `info args`.

```
(gdb) info locals
i = 400
tmp = {1, 222, 7, 84}

(gdb) info args
raw_samples = 0x3128115f
dft_out = 0x2000a900 <my_stack_area+1344>
num_samples = 536912536
```

You can also print all static and global variables in the system by using `info variables`, but that will print *a lot* of variables out to your screen. It's better to filter through them!

The command `info variables` can optional take a regular expression that will perform a search against the name of the variable.

```
(gdb) info variables coredump*
All variables matching regular expression "coredump":

File components/core/src/memfault_data_packetizer.c:
40:	const sMemfaultDataSourceImpl g_memfault_coredump_data_source;

File components/panics/src/memfault_coredump.c:
392:	const sMemfaultDataSourceImpl g_memfault_coredump_data_source;

File ports/panics/src/memfault_platform_ram_backed_coredump.c:
48:	static uint8_t s_ram_backed_coredump_region[700];
...
```

It's also possible to have the regular expression apply to the type as well! Just add `-t` before the regex.

```
(gdb) info variables -t k_spinlock
All defined variables with type matching regular expression "k_spinlock" :

File zephyr/drivers/timer/cortex_m_systick.c:
41:	static struct k_spinlock lock;

File zephyr/kernel/mem_slab.c:
17:	static struct k_spinlock lock;

File zephyr/kernel/mempool.c:
16:	static struct k_spinlock lock;
...
```

### Referencing Specific Variables

If you work on a large project with many modules, you'll likely have static and global variables with the same name. 

```
(gdb) info variables lock
File zephyr/zephyr/kernel/mempool.c:
16:	static struct k_spinlock lock;

File zephyr/zephyr/kernel/mutex.c:
46:	static struct k_spinlock lock;

File zephyr/zephyr/kernel/poll.c:
36:	static struct k_spinlock lock;

...
```

You can reference specific variables from specific **files** using the following syntax:

```
(gdb) p &'mempool.c'::lock
$4 = (struct k_spinlock *) 0x20002370 <lock>
(gdb) p &('mutex.c'::lock)
$8 = (struct k_spinlock *) 0x2000a0c4 <lock>
```

You can also reference specific variables from **functions** using a similar syntax. This is most helpful for referencing `static` variables within functions from the global context.

For example, in the Memfault Firmware SDK, we have a function ``memfault_platform_coredump_get_regions` which contains a static variable `s_coredump_regions`. [Source](https://github.com/memfault/memfault-firmware-sdk/blob/2fe6fc870fd75599243eb7068624d66176213e34/ports/panics/src/memfault_platform_ram_backed_coredump.c#L55)

In GDB, we are unable to print that variable directly:

```
(gdb) p s_coredump_regions
No symbol "s_coredump_regions" in current context.
```

But, if we reference the function directly, we can print the value:

```
(gdb) p memfault_platform_coredump_get_regions::s_coredump_regions
$3 = {{
    type = kMfltCoredumpRegionType_Memory,
    region_start = 0x0,
    region_size = 0
  }}
```

### History Variables

Every time you print an expression in GDB, it will print the value, but in the format of`$<integre> = <value>`. 

```
(gdb) p "hello"
$1 = "hello"
```

The `$1` is a variable, which you can use at any point in the debugging session in other expressions and functions. Every new print statement will get its own variable and the numbers will continuously increase.

```
(gdb) p $1
$2 = "hello"
```

### Convenience Variables

GDB also allows you to create and retrieve any number of variables within a debugging session. All you need to do is use `set $<name>` and then you can print these out or use them in expressions.

```
(gdb) set $test = 5
(gdb) p $test
$4 = 5
```

These are useful when you are using complex expressions, possibly involving casts and nested structs.

```
(gdb) set $wifi = mgmt_thread_data.next_thread.next_thread
(gdb) p $wifi
$11 = (struct k_thread *) 0x200022ac <eswifi_spi0+20>
```

You can also use convenience variables to help you print pieces of data with an array of structs.

```
(gdb) p &shell_wifi_commands
$37 = (const struct shell_static_entry (*)[5]) 0x8031650 <shell_wifi_commands>
```

The above array is a list of `shell_static_entry` structs, each of which have many fields. Browsing through lists of structs is sometimes cumbersome, especially if I'm only trying to look at a single field. 

Convenience variables can help with this.

Let's print the `help` element from struct in the array. I'll set `$i = 0` as my index counter and use `$i++` in each command so that it increments.

```
(gdb) set $i = 0
(gdb) print shell_wifi_commands[$i++]->help
$39 = 0x80314e0 "\"<SSID>\"\n<SSID length>\n<channel number (optional), 0 means all>\n<PSK (optional: valid only for secured SSIDs)>"
(gdb) <enter>
$40 = 0x803155c "Disconnect from Wifi AP"
...
```
Each time I press enter, the previous command is executed and `i` increments!


### The `$` Variable

In GDB, the value of `$` is the value returned from the previous command. 

```
(gdb) p "hello"
$63 = "hello"
(gdb) p $
$64 = "hello"
```

This is useful in it's own right, but it is especially useful for helping me with one of my least favorite tasks in GDB: iterating over linked lists.

You may have done something similar to the following:

```
(gdb) p mgmt_thread_data.next_thread
$65 = (struct k_thread *) 0x2000188c <eswifi0+48>
(gdb) p mgmt_thread_data.next_thread.next_thread
$66 = (struct k_thread *) 0x200022ac <eswifi_spi0+20>
```

Instead of adding `.next_thread` onto the end of the list each iteration, you can use the value of `$` and just keep pressing `<enter>`.

```
(gdb) p mgmt_thread_data.next_thread
$67 = (struct k_thread *) 0x2000188c <eswifi0+48>
(gdb) p $.next_thread
$68 = (struct k_thread *) 0x200022ac <eswifi_spi0+20>
(gdb) <enter>
$69 = (struct k_thread *) 0x2000a120 <k_sys_work_q+20>
```


## Artificial Arrays

It is often useful to print a contiguous region of memory as if it were an array. A common occurrence of this is when using `malloc` to allocate a buffer for a list of integers.

```
int num_elements = 100;
int *elements = malloc(num_elements * sizeof(int));
```

In GDB, if you try to print this, it will just print the pointer value, since it doesn't know it's an array.

```
(gdb) p num_elements
$1 = 100
(gdb) p elements
$2 = (int *) 0x5575e51f6260
```

We can print this entire array using one of two ways. First, we can cast it to a `int[100]` array and print that.

```
(gdb) p (int[100])*elements
$10 = {0, 1, 2, 3, 4, 5, ...
```

Or we can use GDB's artificial arrays! An artificial array is denoted by using the binary operator `@`. The left operand of `@` should be the first element in thee array. The right operand should be the desired length of the array. 

```
(gdb) p *elements@100
$11 = {0, 1, 2, 3, 4, 5, ...
```

## GDB History

By default, GDB does not save any history of the commands that were run during each sessions. This is especially annoying because GDB supports `CTRL+R` for history search!

To fix this, we can place the following in our `.gdbinit`.

```
set history save on
set history size 10000
set history filename ~/.gdb_history
```

With this in place, GDB will now keep the last 10,000 commands in a file ~/.gdb_history. 

[Reference](https://sourceware.org/gdb/current/onlinedocs/gdb/Command-History.html)


## Pretty Printers

GDB Pretty Printers are essentially printing plugins that you can register with GDB. Everytime a value is printed in GDB, GDB will check to see if there are any registered pretty printers for that type, and will use it to print instead. 

For instance, imagine we have a `struct Uuid` type in our codebase. 
`
```c
typedef struct Uuid {
  uint8_t bytes[16];
} Uuid;
```

If we print this, we'll get the following:

```
(gdb) p *(Uuid *)uuid
$6 = {
  bytes = "\235]D@\213Z#^\251\357\354?\221\234zA"
}
```

That's pretty useless to us as we can't read it like a uuid should be written. It would be great if it was printed in the form of `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`. Pretty printers to the rescue!

After [writing a custom pretty printer using GDB's Python API]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}), our UUID's will be printed in a human-readable format.

```
(gdb) p *(Uuid *)uuid
$6 = {
  bytes = 9d5d4440-8b5a-235e-a9ef-ec3f919c7a41
}
```

[Reference](https://sourceware.org/gdb/onlinedocs/gdb/Pretty-Printing.html)

## Struct Operations

### `sizeof`

You can easily get the size of any type using `sizeof` within GDB, as you would in C.

```
(gdb) p sizeof(struct k_thread)
$5 = 160
```

### `offsetof`

This isn't a built-in command, but it's easy enough to add a small macro for it.

```
(gdb) macro define offsetof(t, f) &((t *) 0)->f
```

This macro can also be placed directly within a `.gdbinit` file.

With this in place, we can now print the offset of any struct members.

```
(gdb) p/d offsetof(struct k_thread, next_thread)
$3 = 100
```

[Reference](https://stackoverflow.com/a/39663128)

## External Commands

### Run Make within GDB

Executing the command `make` directly in GDB will trigger Make from the current working directory.

```
(gdb) pwd
Working directory /Users/tyler/nrf5/apps/memfault_demo_app.
(gdb) make
Compiling file: arch_arm_cortex_m.c
Compiling file: memfault_batched_events.c
...
```

### Hex Dump with `xxd`

I love `xxd` for printing files in hexdump format, and we should be able to have the same within GDB. Below is a bit of a hack to bring `xxd` into GDB but it works perfectly. 

```
define xxd
  dump binary memory /tmp/dump.bin $arg0 ((char *)$arg0)+$arg1
  shell xxd /tmp/dump.bin
end
document xxd
  Runs xxd on a memory ADDR and LENGTH

  xxd ADDR LENTH
end
```

If I place the above in my `.gdbinit`, I should now have a command `xxd` in GDB that I can use to hexdump an address and length.

```
(gdb) xxd &shell_uart_out_buffer sizeof(shell_uart_out_buffer)
00000000: 1b5b 6d74 3a7e 2420 0000 0000 0000 0000  .[mt:~$ ........
00000010: 0000 0000 0000 0000 0000 0000 0000       ..............
```

### Running Shell Commands

You can also run arbitrary shell commands within GDB. This is especially useful to me when I'm working on my firmware projects because I always wrap Make with [an Invoke-based CLI]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}) for building and debugging my projects.

```
(gdb) shell invoke build
Compiling file: arch_arm_cortex_m.c
Compiling file: memfault_batched_events.c
```

## Embedded Specific Enhancements

### Thread Awareness

If you are using PyOCD, OpenOCD, or JLink, ensure you are using a gdbserver that is compatible with your RTOS so that you can get the backtraces for all threads. 

For OpenOCD and PyOCD, you can view their supported RTOS's and source code for them in Github ([OpenOCD](https://github.com/ntfreak/openocd/tree/master/src/rtos), [PyOCD](https://github.com/pyocd/pyOCD/tree/master/pyocd/rtos))

To integrate an RTOS with Jlink, you'll have to either use ChibiOS or FreeRTOS, which they support by default, or [write your own JLink RTOS plugin](https://www.segger.com/products/debug-probes/j-link/tools/j-link-gdb-server/thread-aware-debugging/). You can reference [Github](https://github.com/search?q=RTOS_GetCurrentThreadId&type=code) to gain an understanding about how to write one.

### SVD Files and Peripheral Registers

With the help of [PyCortexMDebug](https://github.com/bnahill/PyCortexMDebug), you can parse SVD files and read peripheral register values more easily. To get started, first acqure the .svd file from your vendor or look at the [cmsis-svd](https://github.com/posborne/cmsis-svd) repo on Github.

Once you have the file, clone the PyCortexMDebug repo. 

```
$ git clone https://github.com/bnahill/PyCortexMDebug
```

Next, in GDB, source the `svd_gdb.py` within the project.

```
(gdb) source <path/to/PyCortexMDebug>/cmdebug/svd_gdb.py
```

Finally, load your .svd file and start perusing!

```
(gdb) svd <path/to/svd>/nrf52840.svd
Loading SVD file ...
Done!
(gdb) svd
Available Peripherals:
	FICR:         Factory information configuration registers
	UICR:         User information configuration registers
	CLOCK:        Clock control
	...
```

## Conclusion


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
