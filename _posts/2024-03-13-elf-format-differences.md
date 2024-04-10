---
title: Differences Between ELF-32 and ELF-64
description:
  A quick reference post for the differences between ELF-32 and ELF-64 object
  file formats.
author: ericj
tags: [toolchain, symbol-files]
---

Have you ever wondered if ELF is portable between 32-bit and 64-bit targets?
Probably not, but this might be a common scenario for you if you work on 32-bit
embedded devices but use a 64-bit host. Or maybe you've developed tooling for
32-bit MCUs and are transitioning to working on 64-bit targets.

<!-- excerpt start -->

The ELF object file format is one of the most commonly used today. Most build
systems provide an output to this format, and ELF is commonly used to output
coredumps. The format varies in subtle ways for 32-bit and 64-bit targets
though, which can present problems for tools supporting both. This post will
highlight the main differences and is intended as a quick reference for these
differences.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## ELF Background

If you’re running a program on a UNIX-ish system, you’re probably using an ELF
file. If you’re building firmware for your MCU, it’s probably an ELF file
targeted for your MCU architecture. MacOS and Windows of course have their own
slightly different formats, Mach-O and Portable Executable. This format contains
information on loading your program into memory or linking with other object
files to form an executable object. Static libraries are ELFs. Compiling a
source file produces an ELF. Coredumps can be collected as ELFs. You might even
be an ELF 😱!

## ELF Internals

This post won’t detail the complete specification. For that task, I recommend
these:

- [https://blog.k3170makan.com/2018/09/introduction-to-elf-format-elf-header.html](https://blog.k3170makan.com/2018/09/introduction-to-elf-format-elf-header.html)
  Excellent series with some great illustrations highlighting the specific bytes
  belonging to parts of the ELF
- [https://cpu.land/becoming-an-elf-lord](https://cpu.land/becoming-an-elf-lord)
  A very fun overview focusing on how a program is actually run. Chapter 4
  focuses on ELFs
- [https://uclibc.org/docs/elf.pdf](https://uclibc.org/docs/elf.pdf) The
  original ELF-32 specification
- [https://uclibc.org/docs/elf-64-gen.pdf](https://uclibc.org/docs/elf-64-gen.pdf)
  The newer ELF-64 specification

## ELF-64 vs ELF-32

At a high level, the two formats are structurally very similar but will differ
in the following ways:

- Data Representation: data types for addresses, offsets, and sizes will match
  their native sizes and alignments
- Padding: Equivalent data types in ELF-64 may be packed differently compared to
  ELF-32 to minimize padding

Throughout this post I will highlight the differences in the internal structures
in the following ways:

- If a data type differs or is a different native size, the data type will be
  bold (i.e. **Elf64_Addr**)
- If a field is in a different order within a structure, the field name will be
  bold (i.e. **st_size** )
- If both a data type and field order differ, both the data type _and_ field
  name will be bold

At the end of each structure section, I will also include a few bullet points to
summarize if sizes, field order, or both differ.

## Data Representation Differences

The main place where the two formats differ is in how they represent data. One
difficult part of looking at either standard is that data types are not defined
with a naming scheme that makes the size of each type obvious, e.g. `uint32_t`.
The following tables highlight the differences between the two:

| ELF-32 Type   | Size | Alignment | --  | ELF-64 Type      | Size  | Alignment |
| ------------- | ---- | --------- | --- | ---------------- | ----- | --------- |
| Elf32_Addr    | 4    | 4         | --  | **Elf64_Addr**   | **8** | **8**     |
| Elf32_Off     | 4    | 4         | --  | **Elf64_Off**    | **8** | **8**     |
| Elf32_Half    | 2    | 2         | --  | Elf64_Half       | 2     | 2         |
| Elf32_Word    | 4    | 4         | --  | Elf64_Word       | 4     | 4         |
| Elf32_Sword   | 4    | 4         | --  | Elf64_Sword      | 4     | 4         |
| unsigned char | 1    | 1         | --  | unsigned char    | 1     | 1         |
| -             | -    | -         | --  | **Elf64_Xword**  | **8** | **8**     |
| -             | -    | -         | --  | **Elf64_Sxword** | **8** | **8**     |

The differences are that address and offset fields match their native sizes of
32 vs 64 bits and ELF-64 introduces a long type with signed and unsigned
variants.

## File Header

The table below shows the structure file headers for both types.

| Field       | ELF-32 Type   | --  | Field           | ELF-64 Type    |
| ----------- | ------------- | --- | --------------- | -------------- |
| e_ident[16] | unsigned char | --  | **e_ident[16]** | unsigned char  |
| e_type      | Elf32_Half    | --  | e_type          | Elf64_Half     |
| e_machine   | Elf32_Half    | --  | e_machine       | Elf64_Half     |
| e_version   | Elf32_Word    | --  | e_version       | Elf64_Word     |
| e_entry     | Elf32_Addr    | --  | e_entry         | **Elf64_Addr** |
| e_phoff     | Elf32_Off     | --  | e_phoff         | **Elf64_Off**  |
| e_shoff     | ELf32_Off     | --  | e_shoff         | **Elf64_Off**  |
| e_flags     | Elf32_Word    | --  | e_flags         | Elf64_Word     |
| e_ehsize    | Elf32_Half    | --  | e_ehsize        | Elf64_Half     |
| e_phentsize | Elf32_Half    | --  | e_phentsize     | Elf64_Half     |
| e_phnum     | Elf32_Half    | --  | e_phnum         | Elf64_Half     |
| e_shentsize | Elf32_Half    | --  | e_shentsize     | Elf64_Half     |
| e_shnum     | Elf32_Half    | --  | e_shnum         | Elf64_Half     |
| e_shstrndx  | Elf32_Half    | --  | e_shstrndx      | Elf64_Half     |

The file headers differ in two ways:

- Header size
- `e_ident` byte specification

### e_ident Bytes

The `e_ident` field is composed of 16 bytes, which differ very minimally between
the two formats. You very likely would not encounter an issue here, but there is
a slight difference. ELF-64 specifies byte 7 as `EI_OSABI` and byte 8 as
`EI_ABIVERSION` with padding starting at byte 9. ELF-32 lacks these components
and instead starts padding at byte 7. ELF-32 specifies padding bytes as 0, so we
at least have well-defined values for forward compatibility.

| Byte Name         | ELF-32 Index | ELF-64 Index |
| ----------------- | ------------ | ------------ |
| EI_MAG0           | 0            | 0            |
| EI_MAG1           | 1            | 1            |
| EI_MAG2           | 2            | 2            |
| EI_MAG3           | 3            | 3            |
| EI_CLASS          | 4            | 4            |
| EI_DATA           | 5            | 5            |
| EI_VERSION        | 6            | 6            |
| **EI_OSABI**      | -            | **7**        |
| **EI_ABIVERSION** | -            | **8**        |
| EI_PAD            | 7            | **9**        |

The other noticeable difference in practice is byte 4, `EI_CLASS`. For ELF-32
this byte is 0x1, while for ELF-64 it is 0x2.

## Section Header

| Field        | ELF-32 Type | --  | Field        | ELF-64 Type    |
| ------------ | ----------- | --- | ------------ | -------------- |
| sh_name      | Elf32_Word  | --  | sh_name      | Elf64_Word     |
| sh_type      | Elf32_Word  | --  | sh_type      | Elf64_Word     |
| sh_flags     | Elf32_Word  | --  | sh_flags     | Elf64_Word     |
| sh_addr      | Elf32_Addr  | --  | sh_addr      | **Elf64_Addr** |
| sh_offset    | Elf32_Off   | --  | sh_offset    | **Elf64_Off**  |
| sh_size      | Elf32_Word  | --  | sh_size      | Elf64_Word     |
| sh_link      | Elf32_Word  | --  | sh_link      | Elf64_Word     |
| sh_info      | Elf32_Word  | --  | sh_info      | Elf64_Word     |
| sh_addralign | Elf32_Word  | --  | sh_addralign | Elf64_Word     |
| sh_entsize   | Elf32_Word  | --  | sh_entsize   | Elf64_Word     |

Section headers differ due to:

- Header size

## Symbol Table Entry

| Field    | ELF-32 Type   |     | Field        | ELF-64 Type     |
| -------- | ------------- | --- | ------------ | --------------- |
| st_name  | Elf32_Word    | --  | st_name      | Elf64_Word      |
| st_value | Elf32_Addr    | --  | **st_info**  | unsigned char   |
| st_size  | Elf32_Word    | --  | **st_other** | unsigned char   |
| st_info  | unsigned char | --  | **st_shndx** | Elf64_Half      |
| st_other | unsigned char | --  | **st_value** | **Elf64_Addr**  |
| st_shndx | Elf32_Half    | --  | **st_size**  | **Elf64_Xword** |

Symbol Table Entries differ by:

- Field order
- Entry size

## Relocation Entries

### Relocation Entry Without Addend

| Field    | ELF-32 Type |     | Field    | ELF-64 Type     |
| -------- | ----------- | --- | -------- | --------------- |
| r_offset | Elf32_Addr  | --  | r_offset | Elf64_Addr      |
| r_info   | Elf32_Word  | --  | r_info   | **Elf64_Xword** |

Relocation Entries without addend differ by:

- Entry size

### Relocation Entry With Addend

| Field    | ELF-32 Type |     | Field    | ELF-64 Type      |
| -------- | ----------- | --- | -------- | ---------------- |
| r_offset | Elf32_Addr  | --  | r_offset | Elf64_Addr       |
| r_info   | Elf32_Word  | --  | r_info   | **Elf64_Xword**  |
| r_addend | ELf32_Sword | --  | r_addend | **Elf64_Sxword** |

Relocation Entries with Addend differ by:

- Entry size

## Program Headers

| Field    | ELF-32 Type |     | Field        | ELF-64 Type     |
| -------- | ----------- | --- | ------------ | --------------- |
| p_type   | Elf32_Word  | --  | p_type       | Elf64_Word      |
| p_offset | Elf32_Off   | --  | **p_flags**  | Elf64_Word      |
| p_vaddr  | Elf32_Addr  | --  | **p_offset** | **Elf64_Off**   |
| p_paddr  | Elf32_Addr  | --  | **p_vaddr**  | **Elf64_Addr**  |
| p_filesz | Elf32_Word  | --  | **p_paddr**  | **Elf64_Addr**  |
| p_memsz  | Elf32_Word  | --  | **p_filesz** | **Elf64_Xword** |
| p_flags  | Elf32_Word  | --  | **p_memsz**  | **Elf64_Xword** |
| p_align  | Elf32_Word  | --  | **p_align**  | **Elf64_Xword** |

Program Headers differ due to the following:

- Field order
- Header size

## Other Differences

- Slight differences between the two for “None” values of certain fields. ELF-32
  generally more consistent and specifies them while ELF-64 only sometimes does.
- ELF-64 provides some reserved ranges for OS-specific values. These reserved
  ranges are not specified for ELF-32.

## Conclusion

If you’ve made it this far, first, apologies because that probably means you’ve
hit a nasty bug or problem with a tool. Second, thanks for sticking around, and
I hope this provides a quick and easy resource to compare ELF-32 vs ELF-64.
