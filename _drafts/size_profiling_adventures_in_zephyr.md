---
title: Size Profiling Adventures in Zephyr
description:
  Post Description (~140 words, used for discoverability and SEO)
author: tyler
---

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

Optional motivation to continue onwards

{% include newsletter.html %}

{% include toc.html %}

I recently upgraded an application to Zephyr 3.7 on the esp32c3, and was greeted with a build error:

```
/Users/gminnehan/.local/zephyr-sdk-0.16.1-rc1/riscv64-zephyr-elf/bin/../lib/gcc/riscv64-zephyr-elf/12.2.0/../../../../riscv64-zephyr-elf/bin/ld.bfd: zephyr/zephyr_pre0.elf section `.dram0.bss' will not fit in region `dram0_0_seg'
/Users/gminnehan/.local/zephyr-sdk-0.16.1-rc1/riscv64-zephyr-elf/bin/../lib/gcc/riscv64-zephyr-elf/12.2.0/../../../../riscv64-zephyr-elf/bin/ld.bfd: DRAM segment data does not fit.
/Users/gminnehan/.local/zephyr-sdk-0.16.1-rc1/riscv64-zephyr-elf/bin/../lib/gcc/riscv64-zephyr-elf/12.2.0/../../../../riscv64-zephyr-elf/bin/ld.bfd: region `dram0_0_seg' overflowed by 71104 bytes
```

71 kB overflow?? What could possible have made the image jump 71 kB?

I rebuilt the old image, and checked the previous memory usage was:

```
[496/497] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
     mcuboot_hdr:          32 B         32 B    100.00%
        metadata:          32 B         32 B    100.00%
             ROM:      731062 B    4194240 B     17.43%
     iram0_0_seg:      153008 B       320 KB     46.69%
     irom0_0_seg:      665590 B         4 MB     15.87%
     drom0_0_seg:      146228 B    4194240 B      3.49%
     dram0_0_seg:      327228 B       320 KB     99.86% <----
    rtc_iram_seg:          64 B         8 KB      0.78%
        IDT_LIST:          0 GB         8 KB      0.00%
```

I was already very close to the top of the region that overflowed so but this new image seemly added 452 + 71104. That seems very unlikely so something else is going on here.

## Sample app

Checked an example net app before & after to see if there is anything that might have drastically changed:

```
cd zephyr-rtos-workspace
cd zephyr
git checkout v3.6-zephyr-branch tags/v3.6
west update && west blobs fetch hal_espressif
west build -b esp32c3_devkitm --pristine always zephyr/samples/net/wifi
[ ... ]
Memory region         Used Size  Region Size  %age Used
     mcuboot_hdr:          32 B         32 B    100.00%
        metadata:          28 B         32 B     87.50%
             ROM:      453640 B    4194240 B     10.82%
     iram0_0_seg:       98320 B       320 KB     30.00%
     irom0_0_seg:      407716 B    4194272 B      9.72%
     drom0_0_seg:       71356 B    4194240 B      1.70%
     dram0_0_seg:      222340 B       320 KB     67.85% <--
    rtc_iram_seg:          0 GB         8 KB      0.00%
        IDT_LIST:          0 GB         8 KB      0.00%
```

and then I did the same for 3.7

```
git checkout v3.7-zephyr-branch tags/v3.7
west update && west blobs fetch hal_espressif
west build -b esp32c3_devkitm --pristine always zephyr/samples/net/wifi

Memory region         Used Size  Region Size  %age Used
           FLASH:      604632 B    4194048 B     14.42%
     iram0_0_seg:      135280 B     306688 B     44.11%
     dram0_0_seg:      256768 B     306688 B     83.72% <--
     irom0_0_seg:      305790 B         4 MB      7.29%
     drom0_0_seg:      408024 B         4 MB      9.73%
    rtc_iram_seg:          0 GB         8 KB      0.00%
        IDT_LIST:          0 GB         8 KB      0.00%
```

The image size went up ~34kb, which is fairly significant. Additionally, the DRAM segment size dropped by 20 KB!

### DRAM segment drop

The linker script used is at `zephyr/soc/espressif/esp32c3/default.ld` and some `git blame`-ing revealed this was
updated recently, and `memory.h` was created.

Some quick maths revealed what I found to be true, the dram0_0_seg shrunk in this recent change:

```
dram0_0_seg length = user_dram_end - user_dram_seg_org
= (DRAM_BUFFERS_START + IRAM_DRAM_OFFSET) - IRAM_DRAM_OFFSET - SRAM1_DRAM_START
= (0x3fccae00 + 0x700000) - 0x700000 - 0x3fc80000
= 0x3fccae00 - 0x3fc80000
= 0x4ae00
= 299kB
```

[TODO: Insert why / a zephyr ticket?]

### Image jump

Let's see where that extra 34 kB got added in the example app with `west build -t ram_report` run on both images.

Note - I learned here that you have to disable sysbuild when building your images, otherwise it doesn't know where to look for the binary.

The diff is pretty annoying - I'd love to see a tool that could ingest both and tell you where the greatest increases in size were!
I want to see just the lines where we are adding to the dram segment, specifically dram, so I'll grep by that and then diff
with the meld tool. There is a lot of output in the 3.7 one related to symbols not being assigned to a section -- that is interesting.
I put that aside for now. I actually delete that output because it is confusing the diff algorithm.

I then jump down to the WORKSPACE and ZEPHYR_BASE sections because that is where I'll be able to see what specific libraries may have bloated
the image. 

WORKSPACE looks normal -- not much different there. 9613 vs 6268 so we actually went down.

Same with ZEPHYR_BASE --> we went down. So where did the 34 kB come from? It looks like in the `(hidden)`
section, I jumped from 234153 to 535141 B. What is the hidden section? It is mentioned in the `size_report.py` script:

```
# Need to account for code and data where there are not emitted
# symbols associated with them.
```

I found that the `size_report.py` script has a verbose mode to print out additional data about the sections. That might give me
a better idea of which sections the image size changed in. Doing `west -v build -t ram_report` gave me the full invocation
of the `size_report.py` script that I could modify with the added `-v` argument:

```
cd /Users/gminnehan/projects/zephyr-rtos-workspace/build && \
/Users/gminnehan/projects/zephyr-rtos-workspace/.venv/bin/python3.11 \
/Users/gminnehan/projects/zephyr-rtos-workspace/zephyr/scripts/footprint/size_report \
-k /Users/gminnehan/projects/zephyr-rtos-workspace/build/zephyr/zephyr.elf \
-z /Users/gminnehan/projects/zephyr-rtos-workspace/zephyr \
-o /Users/gminnehan/projects/zephyr-rtos-workspace/build --workspace=/Users/gminnehan/projects/zephyr-rtos-workspace \
-d 99 \
-v \ # verbose mode
ram \
> ../ram-report-debug-3.7.txt && \ # save off
cd .. \ # return to our original folder
```

For 3.6, this doesn't print any helpful debug prints, but they added a debug prints about each section for 3.7:

```
DEBUG: 0x00000000-0x00000000 unassigned section '': size=0, SHT_NULL, 0x00000000
DEBUG: 0x40380000-0x4039ff6f ROM txt section '.iram0.text': size=130928, SHT_PROGBITS, 0x00000006
DEBUG: 0x403a1070-0x403a1070 unassigned section '.iram0.data': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x3fca1080-0x3fca3443 ROM,RAM section '.dram0.data': size=9156, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4694-0x3fca4694 unassigned section '.dram0.end': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x50000000-0x50000000 unassigned section '.rtc.text': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x50000000-0x50000000 unassigned section '.rtc.dummy': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x50000000-0x50000000 unassigned section '.rtc.force_slow': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x4039ff70-0x403a106f ROM txt section '.loader.text': size=4352, SHT_PROGBITS, 0x00000006
DEBUG: 0x403a1070-0x403a1070 unassigned section '.iram0.text_end': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x403a1070-0x403a1070 unassigned section '.iram0.bss': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x3fc80000-0x3fca107f RAM bss section '.dram0.dummy': size=135296, SHT_NOBITS, 0x00000003
DEBUG: 0x3fca3448-0x3fca3f4f ROM r/o section '.loader.data': size=2824, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fca3f50-0x3fca413f ROM,RAM section 'sw_isr_table': size=496, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4140-0x3fca414b ROM,RAM section 'device_states': size=12, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca414c-0x3fca418b ROM,RAM section 'log_mpsc_pbuf_area': size=64, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca418c-0x3fca418f ROM,RAM section 'log_msg_ptr_area': size=4, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4190-0x3fca4227 ROM,RAM section 'log_dynamic_area': size=152, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4228-0x3fca427b ROM,RAM section 'k_mem_slab_area': size=84, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca427c-0x3fca428f ROM,RAM section 'k_heap_area': size=20, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4290-0x3fca4343 ROM,RAM section 'k_mutex_area': size=180, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4344-0x3fca4373 ROM,RAM section 'k_msgq_area': size=48, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4374-0x3fca43a3 ROM,RAM section 'k_sem_area': size=48, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca43a4-0x3fca43fb ROM,RAM section 'net_buf_pool_area': size=88, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4400-0x3fca4527 ROM,RAM section 'net_if_area': size=296, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4528-0x3fca4543 ROM,RAM section 'net_if_dev_area': size=28, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4544-0x3fca4553 ROM r/o section 'net_l2_area': size=16, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fca4554-0x3fca4683 ROM r/o section 'log_const_area': size=304, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fca4684-0x3fca4693 ROM r/o section 'log_backend_area': size=16, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fca46a0-0x3fcb8063 RAM bss section '.dram0.noinit': size=80324, SHT_NOBITS, 0x00000003
DEBUG: 0x3fcb8070-0x3fcbeaff RAM bss section '.dram0.bss': size=27280, SHT_NOBITS, 0x00000003
DEBUG: 0x00024680-0x0002ffff RAM bss section '.flash.text_dummy': size=47488, SHT_NOBITS, 0x00000003
DEBUG: 0x42000000-0x4204aa7d ROM txt section '.flash.text': size=305790, SHT_PROGBITS, 0x00000006
DEBUG: 0x0007aa7e-0x0007ffff RAM bss section '.flash.dummy': size=21890, SHT_NOBITS, 0x00000003
DEBUG: 0x3c000000-0x3c04ffff RAM bss section '.flash.rodata_dummy': size=327680, SHT_NOBITS, 0x00000003
DEBUG: 0x3c050000-0x3c06362b ROM r/o section '.flash.rodata': size=79404, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c06362c-0x3c0636b3 ROM r/o section 'initlevel': size=136, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0636b4-0x3c06372b ROM r/o section 'device_area': size=120, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c06372c-0x3c06375b ROM r/o section 'shell_area': size=48, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c06375c-0x3c06378b ROM r/o section 'shell_root_cmds_area': size=48, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c06378c-0x3c0639cf ROM r/o section 'shell_subcmds_area': size=580, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0639d0-0x3c0639d7 ROM r/o section 'shell_dynamic_subcmds_area': size=8, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0639d8-0x3c0639df RAM bss section 'tbss': size=8, SHT_NOBITS, 0x00000403
DEBUG: 0x3c0639e0-0x3c0639e0 unassigned section '.flash.rodata_end': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x00000000-0x00000054 unassigned section '.comment': size=85, SHT_PROGBITS, 0x00000030
DEBUG: 0x00000000-0x00006637 unassigned section '.debug_aranges': size=26168, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x002e5abc unassigned section '.debug_info': size=3037885, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x0003aca3 unassigned section '.debug_abbrev': size=240804, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x0011b03f unassigned section '.debug_line': size=1159232, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x00015737 unassigned section '.debug_frame': size=87864, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x0003930c unassigned section '.debug_str': size=234253, SHT_PROGBITS, 0x00000030
DEBUG: 0x00000000-0x0008d7ec unassigned section '.debug_loc': size=579565, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x0001a34f unassigned section '.debug_ranges': size=107344, SHT_PROGBITS, 0x00000000
DEBUG: 0x00000000-0x00000043 unassigned section '.riscv.attributes': size=68, SHT_RISCV_ATTRIBUTES, 0x00000000
DEBUG: 0x00000000-0x0002217f unassigned section '.symtab': size=139648, SHT_SYMTAB, 0x00000000
DEBUG: 0x00000000-0x000217cc unassigned section '.strtab': size=137165, SHT_STRTAB, 0x00000000
DEBUG: 0x00000000-0x000002fc unassigned section '.shstrtab': size=765, SHT_STRTAB, 0x00000000
```

with the dram segments being:

```
DEBUG: 0x3fca1080-0x3fca3443 ROM,RAM section '.dram0.data': size=9156, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fca4694-0x3fca4694 unassigned section '.dram0.end': size=0, SHT_PROGBITS, 0x00000001
DEBUG: 0x3fc80000-0x3fca107f RAM bss section '.dram0.dummy': size=135296, SHT_NOBITS, 0x00000003
DEBUG: 0x3fca46a0-0x3fcb8063 RAM bss section '.dram0.noinit': size=80324, SHT_NOBITS, 0x00000003
DEBUG: 0x3fcb8070-0x3fcbeaff RAM bss section '.dram0.bss': size=27280, SHT_NOBITS, 0x00000003
```

I added the new print function add the invocations to the 3.6 build as well:

```
diff --git a/scripts/footprint/size_report b/scripts/footprint/size_report
index 43e35f1d5fd..c487865ed88 100755
--- a/scripts/footprint/size_report
+++ b/scripts/footprint/size_report
@@ -72,6 +72,15 @@ def is_symbol_in_ranges(sym, ranges):

     return False

+def print_section_info(section, descr=""):
+  if args.verbose:
+      sec_size = section['sh_size']
+      sec_start = section['sh_addr']
+      sec_end = sec_start + (sec_size - 1 if sec_size else 0)
+      print(f"DEBUG: "
+            f"0x{sec_start:08x}-0x{sec_end:08x} "
+            f"{descr} '{section.name}': size={sec_size}, "
+            f"{section['sh_type']}, 0x{section['sh_flags']:08x}")

 def get_die_mapped_address(die, parser, dwarfinfo):
     """Get the bounding addresses from a DIE variable or subprogram"""
@@ -191,6 +200,7 @@ def get_section_ranges(elf):
             # BSS and noinit sections
             ram_addr_ranges.append(bound)
             ram_size += size
+            print_section_info(section, "RAM bss section")
         elif section['sh_type'] == 'SHT_PROGBITS':
             # Sections to be in flash or memory
             flags = section['sh_flags']
@@ -198,6 +208,7 @@ def get_section_ranges(elf):
                 # Text section
                 rom_addr_ranges.append(bound)
                 rom_size += size
+                print_section_info(section, "ROM txt section")
             elif (flags & SHF_WRITE_ALLOC) == SHF_WRITE_ALLOC:
                 # Data occupies both ROM and RAM
                 # since at boot, content is copied from ROM to RAM
@@ -206,10 +217,12 @@ def get_section_ranges(elf):

                 ram_addr_ranges.append(bound)
                 ram_size += size
+                print_section_info(section, "ROM,RAM section")
             elif (flags & SHF_ALLOC) == SHF_ALLOC:
                 # Read only data
                 rom_addr_ranges.append(bound)
                 rom_size += size
+                print_section_info(section, "ROM r/o section")

     ret = {'rom': rom_addr_ranges,
            'rom_total_size': rom_size,
```

which gave me:

```
DEBUG: 0x00000000-0x0000001f ROM r/o section '.mcuboot_header': size=32, SHT_PROGBITS, 0x00000002
DEBUG: 0x00000020-0x0000003b ROM r/o section '.metadata': size=28, SHT_PROGBITS, 0x00000002
DEBUG: 0x40380000-0x4039800b ROM txt section '.iram0.text': size=98316, SHT_PROGBITS, 0x00000006
DEBUG: 0x3fcb47e8-0x3fcb5d2f ROM,RAM section '.dram0.data': size=5448, SHT_PROGBITS, 0x00000003
DEBUG: 0x3c000040-0x3c011363 ROM r/o section 'rodata': size=70436, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c011364-0x3c0113eb ROM r/o section 'initlevel': size=136, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0113ec-0x3c011463 ROM r/o section 'device_area': size=120, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c011464-0x3c011493 ROM r/o section 'shell_area': size=48, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c011494-0x3c0114c3 ROM r/o section 'shell_root_cmds_area': size=48, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0114c4-0x3c0116f3 ROM r/o section 'shell_subcmds_area': size=560, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0116f4-0x3c0116fb ROM r/o section 'shell_dynamic_subcmds_area': size=8, SHT_PROGBITS, 0x00000002
DEBUG: 0x3c0116fc-0x3c011703 RAM bss section 'tbss': size=8, SHT_NOBITS, 0x00000403
DEBUG: 0x3fc80000-0x3fc9800f RAM bss section '.dram0.dummy': size=98320, SHT_NOBITS, 0x00000003
DEBUG: 0x3fc98010-0x3fc9e28f RAM bss section 'bss': size=25216, SHT_NOBITS, 0x00000003
DEBUG: 0x3fc9e290-0x3fcb47e3 RAM bss section 'noinit': size=91476, SHT_NOBITS, 0x00000003
DEBUG: 0x3fcb5d30-0x3fcb5f1f ROM,RAM section 'sw_isr_table': size=496, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb5f20-0x3fcb5f2b ROM,RAM section 'device_states': size=12, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb5f2c-0x3fcb5f6b ROM,RAM section 'log_mpsc_pbuf_area': size=64, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb5f6c-0x3fcb5f6f ROM,RAM section 'log_msg_ptr_area': size=4, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb5f70-0x3fcb6003 ROM,RAM section 'log_dynamic_area': size=148, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6004-0x3fcb6057 ROM,RAM section 'k_mem_slab_area': size=84, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6058-0x3fcb606b ROM,RAM section 'k_heap_area': size=20, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb606c-0x3fcb611f ROM,RAM section 'k_mutex_area': size=180, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6120-0x3fcb617f ROM,RAM section 'k_msgq_area': size=96, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6180-0x3fcb61af ROM,RAM section 'k_sem_area': size=48, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb61b0-0x3fcb6207 ROM,RAM section 'net_buf_pool_area': size=88, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6208-0x3fcb631f ROM,RAM section 'net_if_area': size=280, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb6320-0x3fcb633b ROM,RAM section 'net_if_dev_area': size=28, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fcb633c-0x3fcb634b ROM r/o section 'net_l2_area': size=16, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fcb634c-0x3fcb6473 ROM r/o section 'log_const_area': size=296, SHT_PROGBITS, 0x00000002
DEBUG: 0x3fcb6474-0x3fcb6483 ROM r/o section 'log_backend_area': size=16, SHT_PROGBITS, 0x00000002
DEBUG: 0x4039800c-0x4039800f RAM bss section '.iram0.text_end': size=4, SHT_NOBITS, 0x00000003
DEBUG: 0x42020020-0x420638c3 ROM txt section '.flash.text': size=276644, SHT_PROGBITS, 0x00000006
DEBUG: 0x42000020-0x4202001f RAM bss section '.flash_text_dummy': size=131072, SHT_NOBITS, 0x00000003
```

with dram being:

```
DEBUG: 0x3fcb47e8-0x3fcb5d2f ROM,RAM section '.dram0.data': size=5448, SHT_PROGBITS, 0x00000003
DEBUG: 0x3fc80000-0x3fc9800f RAM bss section '.dram0.dummy': size=98320, SHT_NOBITS, 0x00000003
DEBUG: 0x3fc98010-0x3fc9e28f RAM bss section 'bss': size=25216, SHT_NOBITS, 0x00000003
DEBUG: 0x3fc9e290-0x3fcb47e3 RAM bss section 'noinit': size=91476, SHT_NOBITS, 0x00000003
```

The `.dram0.dummy` second jumped from 98320 to 135296, which is a 36 kB jump -- aligning witch the jump I saw in the overall dram0_0_seg.

What is the `.dummy` section? Per this print, it is a RAM bss section, so it holds uninitialized RAM data.

Not superhelpful for narrowing things down though.

 ## My app

To get a ram report of my application, I'll have to get it to compile first, which means I need to rip out a few things.

Need to get under to see where the bloat is. A few things come to mind:
- kill mbedtls memory heap (`CONFIG_MBEDTLS_HEAP_SIZE`), which is currently 30,000. This is necessary at runtime but I can remove it for now

```
region `dram0_0_seg' overflowed by 41616 bytes
```

Next, let's remove the shell. Note that you'll need to disable the shells for any subsystems -- many will `select SHELL`.

```
region `dram0_0_seg' overflowed by 2512 bytes
```

Nice! Almost there. Let's do something easy make the default log level 0 with `CONFIG_LOG_DEFAULT_LEVEL=0`

```
Memory region         Used Size  Region Size  %age Used
     mcuboot_hdr:          32 B         32 B    100.00%
        metadata:          32 B         32 B    100.00%
           FLASH:      653888 B    4194240 B     15.59%
     iram0_0_seg:      151120 B     301568 B     50.11%
     dram0_0_seg:      297904 B     301568 B     98.79%
     irom0_0_seg:      357032 B         4 MB      8.51%
     drom0_0_seg:      457344 B         4 MB     10.90%
    rtc_iram_seg:          64 B         8 KB      0.78%
        IDT_LIST:          0 GB         8 KB      0.00%
```

Crypto added 8834 - 161 = 8.5 kB. Poking around in the `aes.c` file, I found that the statically allocated
buffers were controlled with `MBEDTLS_AES_ROM_TABLES`, which in turn is mapped to the Kconfig `CONFIG_MBEDTLS_AES_ROM_TABLES=y`.
Looks like this used to be enabled by default, but isn't anymore (TODO confirm). I checked the 3.6 build's `.config` and confirmed this.

```
│       ├── crypto                                                                               8834   1.16%  - 
│       │   └── mbedtls                                                                          8834   1.16%  - 
│       │       └── library                                                                      8834   1.16%  - 
│       │           ├── aes.c                                                                    8748   1.15%  - 
│       │           │   ├── FSb                                                                   256   0.03%  0x3fcc77a0 .dram0.bss
│       │           │   ├── FT0                                                                  1024   0.13%  0x3fcc73a0 .dram0.bss
│       │           │   ├── FT1                                                                  1024   0.13%  0x3fcc6fa0 .dram0.bss
│       │           │   ├── FT2                                                                  1024   0.13%  0x3fcc6ba0 .dram0.bss
│       │           │   ├── FT3                                                                  1024   0.13%  0x3fcc67a0 .dram0.bss
│       │           │   ├── RSb                                                                   256   0.03%  0x3fcc66a0 .dram0.bss
│       │           │   ├── RT0                                                                  1024   0.13%  0x3fcc62a0 .dram0.bss
│       │           │   ├── RT1                                                                  1024   0.13%  0x3fcc5ea0 .dram0.bss
│       │           │   ├── RT2                                                                  1024   0.13%  0x3fcc5aa0 .dram0.bss
│       │           │   ├── RT3                                                                  1024   0.13%  0x3fcc56a0 .dram0.bss
│       │           │   ├── aes_init_done                                                           4   0.00%  0x3fcbeb38 .dram0.bss
│       │           │   └── round_constants                                                        40   0.01%  0x3fcc5678 .dram0.bss
```

I built again with that option and it saved me 8,752 B!

Looking through, it was getting hard to distinguish my actual savings because I compiled out the shell
and logging. So I'll disable those in 3.6 version as well.

Nothing else seemed to stand out so I decided to try enabling the shells I had in my app in the sample
app -- maybe the shell got bigger somehow?


## Extra

What is DRAM?
- https://www.hp.com/us-en/shop/tech-takes/what-is-dram-dynamic-random-access-memory
c3 memory map
- https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf
What is stored there?
- .data --> code
- .bss --> statically allocated data
What is stored in SRAM vs. DRAM?

CONFIG_NET_LOG=n removed nothing

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
