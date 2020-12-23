# mtb_decode.py
try:
    import gdb
except ImportError:
    raise Exception("This script can only be run within gdb!")

import argparse
import struct


def gdb_address_to_function(address):
    try:
        block = gdb.current_progspace().block_for_pc(address)
        while block is not None:
            if block.function:
                return str(block.function)
            block = block.superblock
    except RuntimeError:
        pass
    return hex(address)


def gdb_address_to_function_file_line(address):
    function = gdb_address_to_function(address)
    sal = gdb.find_pc_line(address)
    line = "?" if sal.symtab is None else sal.line
    filename = "?" if sal.symtab is None else sal.symtab.filename
    return (filename, line, function)


def gdb_read_word(address):
    reg_data = gdb.selected_inferior().read_memory(address, 4)
    reg_val = struct.unpack("<I", reg_data)[0]
    return reg_val


class GdbArgumentParseError(Exception):
    pass


class GdbArgumentParser(argparse.ArgumentParser):
    """Wrap argparse so gdb doesn't exit when a command fails """

    def exit(self, status=0, message=None):
        if message:
            self._print_message(message)
        # Don't call sys.exit()
        raise GdbArgumentParseError()


class Mtb(gdb.Command):
    def __init__(self):
        super(Mtb, self).__init__("mtb", gdb.COMMAND_STATUS, gdb.COMPLETE_NONE)

    def decode(self, limit=None):
        MTB_PERIPHERAL_ADDR = 0xE0043000
        MTB_POSITION = MTB_PERIPHERAL_ADDR + 0x0
        MTB_MASTER = MTB_PERIPHERAL_ADDR + 0x4
        MTB_BASE = MTB_PERIPHERAL_ADDR + 0xC

        mtb_position_val = gdb_read_word(MTB_POSITION)
        write_offset = mtb_position_val & 0xFFFFFFF8
        wrap = mtb_position_val & (1 << 2)

        mtb_base_val = gdb_read_word(MTB_BASE)

        mtb_master_val = gdb_read_word(MTB_MASTER)
        mask = mtb_master_val & 0x1F

        mtb_sram_size = 2 ** (mask + 4)
        valid_size = mtb_sram_size if wrap else write_offset
        oldest_pkt = write_offset if wrap else 0

        start = 0 if not limit else max(0, valid_size - limit * 8)

        for offset in range(start, valid_size, 8):
            pkt_addr = mtb_base_val + (oldest_pkt + offset) % mtb_sram_size

            # Read the source and destination addresses
            s_addr = gdb_read_word(pkt_addr)
            d_addr = gdb_read_word(pkt_addr + 4)

            bit_a = s_addr & 0x1
            s_addr &= 0xFFFFFFFE

            bit_s = d_addr & 0x1
            d_addr &= 0xFFFFFFFE

            if bit_s:
                print("Begin Trace Session")

            # For every valid packet, display src, dst and instruction info, i.e
            #
            # >S: 0x20000488 - HardFault_Handler @ ./main.c:107
            #  D: 0x200004e0 - mtb_disable @ ./mtb.c:41

            file_s, line_s, func_s = gdb_address_to_function_file_line(s_addr)
            file_d, line_d, func_d = gdb_address_to_function_file_line(d_addr)

            if bit_a:  # Exception or Debug State Entry / Exit
                if s_addr & 0xFFFFFF00 == 0xFFFFFF00:
                    print(">S: {} - Exception Return".format(hex(s_addr)))
                else:
                    print(
                        ">S: {} - Exception (or debug) Entry from {} @ {}:{}".format(
                            hex(s_addr), func_s, file_s, line_s
                        )
                    )
            else:  # Normal branch took place
                print(">S: {} - {} @ {}:{}".format(hex(s_addr), func_s, file_s, line_s))

            print(" D: {} - {} @ {}:{}".format(hex(d_addr), func_d, file_d, line_d))

    def invoke(self, unicode_args, from_tty):
        parser = GdbArgumentParser(description="MTB decode utility")

        parser.add_argument(
            "-l", "--limit", type=int, help="The maximum number of packets to decode"
        )

        try:
            args = parser.parse_args(list(filter(None, unicode_args.split(" "))))
        except GdbArgumentParseError:
            return

        self.decode(limit=args.limit)


Mtb()
