try:
    import gdb
except ImportError:
    raise Exception("This script can only be run within gdb!")

import argparse
import math
import struct


class DwtWatchpointException(Exception):
    pass


DWT_CTRL = 0xE0001000
DWT_COMP_BASE = 0xE0001020
DWT_MASK_BASE = 0xE0001024
DWT_FUNC_BASE = 0xE0001028


class Dwt:
    @staticmethod
    def _write_memory(addr, value):
        gdb.selected_inferior().write_memory(addr, struct.pack("<I", value))

    @classmethod
    def _write_dwt_comp_reg(cls, reg_base, comp_id, value):
        addr = reg_base + 16 * comp_id
        cls._write_memory(addr, value)

    @classmethod
    def set_addr_watchpoint(cls, addr, size):
        if addr % size != 0:
            raise DwtWatchpointException("Address needs to be aligned by size")
        if size & (size - 1) != 0:
            raise DwtWatchpointException("size must be power of 2")

        mask = int(math.log(size, 2))

        # For example purposes always use comparator 0 and overwrite
        # what was there previously.
        comp_id = 0
        cls._write_dwt_comp_reg(DWT_COMP_BASE, comp_id, addr)
        cls._write_dwt_comp_reg(DWT_MASK_BASE, comp_id, mask)
        func = 0x6  # Install a write watchpoint
        cls._write__write_dwt_comp_reg(DWT_FUNC_BASE, comp_id, func)

    @classmethod
    def set_data_value_watchpoint(cls, value, size):
        if size not in [1, 2, 4]:
            raise DwtWatchpointException(
                "Pattern watch must be a byte, half-word, or word in size"
            )

        # For values with a size less than a word, the value must be copied
        # into the other parts of the comparator register
        if size == 2:
            value = value | (value << 16)
        elif size == 1:
            value = value | value << 8 | value << 16 | value << 24

        # For Cortex-M4's data value matching is only supported in comparator 1
        comp_id = 1
        cls._write_dwt_comp_reg(DWT_COMP_BASE, comp_id, value)
        cls._write_dwt_comp_reg(DWT_MASK_BASE, comp_id, 0)

        # The range scanned can be filtered with other comparators
        # or disabled by setting the value to the comparator being
        # used. For this example, let's just disable:
        #   DATAVADDR0 (bits[19:16]) = comp_id
        #   DATAVADDR1 (bits[15:12]) = comp_id
        #
        # Rest of Config:
        #   DATAVSIZE (bits[11:10]) = size >> 1
        #   DATAVMATCH (bit[8]) = 1
        #     Encoding is 00 (byte), 01 (half-word), 10 (word)
        #   FUNCTION = 0x6 for a write-only watchpoint
        datavsize = (size >> 1) << 10
        func = (comp_id << 16) | (comp_id << 12) | (1 << 8) | datavsize | 0x6
        cls._write_dwt_comp_reg(DWT_FUNC_BASE, comp_id, func)


class GdbArgumentParseError(Exception):
    pass


class GdbArgumentParser(argparse.ArgumentParser):
    """Wrap argparse so gdb doesn't exit when a command fails """

    def exit(self, status=0, message=None):
        if message:
            self._print_message(message)
        # Don't call sys.exit()
        raise GdbArgumentParseError()


class WatchpointExtensionCliCmd(gdb.Command):
    """Extension of  GDB's watch for Cortex-M """

    def __init__(self):
        super(WatchpointExtensionCliCmd, self).__init__("watch_ext", gdb.COMMAND_USER)

    def complete(self, text, word):
        # We expect the argument passed to be a symbol so fallback to the
        # internal tab-completion handler for symbols
        return gdb.COMPLETE_SYMBOL

    def invoke(self, unicode_args, from_tty):
        # We can pass args here and use Python CLI utilities like argparse
        # to do argument parsing
        parser = GdbArgumentParser(
            description="Additional Cortex-M Watchpoint Utilities"
        )

        parser.add_argument(
            "-c", "--comp", help="The memory address or value to watch for"
        )
        parser.add_argument("-s", "--size", help="The size to watch")
        parser.add_argument(
            "--compare-value",
            type=bool,
            help="Watch for value written instead of address written",
        )

        try:
            args = parser.parse_args(list(filter(None, unicode_args.split(" "))))
        except GdbArgumentParseError:
            return

        comp = int(args.comp, 0)
        size = int(args.size, 0)

        if not args.compare_value:
            print("Setting Address Watchpoint")
            Dwt.set_addr_watchpoint(comp, size)
        else:
            print("Setting Data Value Watchpoint")
            Dwt.set_data_value_watchpoint(comp, size)


WatchpointExtensionCliCmd()
