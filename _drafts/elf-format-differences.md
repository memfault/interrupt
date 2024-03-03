---
title: Differences Between ELF-32 and ELF-64
description: A quick reference post for the differences between ELF-32 and ELF-64 object file formats.
author: ericj
tags: [toolchain, symbol-files]
---

Have you ever wondered, is ELF portable between 32-bit and 64-bit targets? Probably not, but this might be a common scenario for you if you work on 32-bit embedded devices but use a 64-bit host. Or maybe you've developed tooling for 32-bit MCUs and are transitioning to working on 64-bit targets.

<!-- excerpt start -->

The ELF object file format is one of the most commonly used today. Most build systems provide an output to this format and ELF is commonly used to output coredumps. The format varies in subtle ways for 32-bit and 64-bit targets though, which can present problems for tools supporting both. This post will highlight the main differences and is intended as a quick reference for these differences.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## ELF Background

If you’re running a program on a UNIX-ish system, you’re probably using an ELF file. If you’re building firmware for your MCU, it’s probably an ELF file targeted for your MCU architecture. MacOS and Windows of course have their own slightly different formats, mach-o and Portable Executable. This format contains information on how to load your program into memory or to link with other object files to form an executable object. Static libraries are ELFs. The result of compiling a source file produces an ELF. Coredumps can be collected as ELFs. You might even be an ELF 😱!

## ELF Internals

This post won’t detail the complete specification, for that task I’ll recommend these:

- [https://blog.k3170makan.com/2018/09/introduction-to-elf-format-elf-header.html](https://blog.k3170makan.com/2018/09/introduction-to-elf-format-elf-header.html) Excellent series with some great illustrations highlighting the specific bytes belonging to parts of the ELF
- [https://cpu.land/becoming-an-elf-lord](https://cpu.land/becoming-an-elf-lord) A very fun overview focusing on how a program is actually run. Chapter 4 focuses on ELFs
- [https://uclibc.org/docs/elf.pdf](https://uclibc.org/docs/elf.pdf) The original ELF-32 specification
- [https://uclibc.org/docs/elf-64-gen.pdf](https://uclibc.org/docs/elf-64-gen.pdf) The newer ELF-64 specification

## ELF-64 vs ELF-32

At a high level, the two formats are structurally very similar but will differ in the following ways:

- Data Representation: data types for addresses, offsets, and sizes will match their native sizes and alignments
- Padding: Equivalent data types in ELF-64 may be packed differently compared to ELF-32 to minimize padding

## Data Representation Differences

The main place where the two formats differ is in how they represent data. One difficult part of looking at either standard is that data types are not defined with a naming scheme that makes the size of each type obvious, e.g. `uint32_t`. To help summarize, see these tables:

| Type | Size | Alignment |
| --- | --- | --- |
| Elf32_Addr | 4 | 4 |
| Elf32_Off | 4 | 4 |
| Elf32_Half | 2 | 2 |
| Elf32_Word | 4 | 4 |
| Elf32_Sword | 4 | 4 |
| unsigned char | 1 | 1 |
| - | - | - |
| - | - | - |

| Type | Size | Alignment |
| --- | --- | --- |
| Elf64_Addr | 8 | 8 |
| Elf64_Off | 8 | 8 |
| Elf64_Half | 2 | 2 |
| Elf64_Word | 4 | 4 |
| Elf64_Sword | 4 | 4 |
| unsigned char | 1 | 1 |
| Elf64_Xword | 8 | 8 |
| Elf64_Sxword | 8 | 8 |

The differences are that address and offset fields match their native sizes of 32 vs 64 bits and ELF-64 introduces a long type with signed and unsigned variants.

## File Header

File headers differ in two ways:

- Overall size due to data type size differences
- `e_ident` byte specification

### e_ident Bytes

The `e_ident` field is composed of 16 bytes which differ very minimally between the two formats. You very likely would not encounter an issue here but there is a slight difference. ELF-64 specifies byte 7 as `EI_OSABI` and byte 8 as `EI_ABIVERSION` with padding starting at byte 9. ELF-32 lacks these components and instead starts padding at byte 7. ELF-32 specifies padding bytes be 0, so we at least have well defined values for forward compatibility.

The other difference that is noticeable in practice is byte 4, `EI_CLASS`. For ELF-32 this byte is 0x1, while for ELF-64 it is 0x2.

## Section Header

Section headers differ only by total size due to data type sizes.

## Symbol Table Entry

| Field | Elf-32 Type |
| --- | --- |
| st_name | Word |
| st_value | Addr |
| st_size | Word |
| st_info | unsigned char |
| st_other | unsigned char |
| st_shndx | Half |

| Field | Elf-64 Type |
| --- | --- |
| st_name | Word |
| st_info | unsigned char |
| st_other | unsigned char |
| st_shndx | Half |
| st_value | Addr |
| st_size | Xword |

Symbol Table Entries differ by:

- Field order
- Overall entry size due to data type sizes

## Relocation Entries

Both entry types, with and without addends, only differ in terms of overall size due to data type differences.

## Program Headers

Program headers differ in 3 ways:

- Field order
- Field types
- Overall header size due to data type differences

| Field | ELF-32 Type |
| --- | --- |
| p_type | Word |
| p_offset | Off |
| p_vaddr | Addr |
| p_paddr | Addr |
| p_filesz | Word |
| p_memsz | Word |
| p_flags | Word |
| p_align | Word |

| Field | ELF-64 Type |
| --- | --- |
| p_type | Word |
| p_flags | Word |
| p_offset | Off |
| p_vaddr | Addr |
| p_paddr | Addr |
| p_filesz | Xword |
| p_memsz | Xword |
| p_align | Xword |

## Other Differences

- Slight differences between the two for “None” values of certain fields. ELF-32 generally more consistent and specifies them while ELF-64 only sometimes does.
- ELF-64 provides some reserved ranges for OS-specific values. These reserved ranges are not specified for ELF-32.

If you’ve made it this far, first apologies because that probably means you’ve hit a nasty bug or problem with a tool. Second, thanks for sticking around and I hope this provides a quick and easy resource to compare ELF-32 vs ELF-64