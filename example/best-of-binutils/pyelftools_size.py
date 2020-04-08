def _get_allocated_section_sizes(elffile):
    from elftools.elf.constants import SH_FLAGS

    text = data = bss = 0
    for section in elffile.iter_sections():
        sh_flags = section["sh_flags"]
        if sh_flags & SH_FLAGS.SHF_ALLOC == 0:
            continue  # Section is not in final binary

        section_size = section["sh_size"]

        if sh_flags & SH_FLAGS.SHF_EXECINSTR != 0 or sh_flags & SH_FLAGS.SHF_WRITE == 0:
            text += section_size
        elif section["sh_type"] != "SHT_NOBITS":
            data += section_size
        else:
            bss += section_size

    return (text, data, bss)


def size_elf(elf_file_path):
    from elftools.elf.elffile import ELFFile

    with open(elf_file_path, "rb") as elf_file:
        text, data, bss = _get_allocated_section_sizes(ELFFile(elf_file))
    print(f"   text\t   data\t    bss\t    dec\t    hex\tfilename")
    total = text + data + bss
    print(f"{text:7d}\t{data:7d}\t{bss:7d}\t{total:7d}\t{total:7x}\t{elf_file_path}")


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 2:
        print("Please specify a path to an ELF")
        exit(-1)
    size_elf(sys.argv[1])
