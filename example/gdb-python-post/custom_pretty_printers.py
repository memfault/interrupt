from gdb.printing import PrettyPrinter, register_pretty_printer
import gdb
import uuid


class UuidPrettyPrinter(object):
    """Print 'struct Uuid' as 'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        # gdb.Value can be accessed just like they would be in C. Since
        # this is a Uuid struct we can index into "bytes" and grab the
        # value from each entry in the array
        array = self.val["bytes"]
        # Format the byte array as a hex string so Python uuid module can
        # be used to get the string
        uuid_bytes = "".join(
            ["%02x" % int(array[i]) for i in range(0, array.type.sizeof)]
        )
        return str(uuid.UUID(uuid_bytes))


class CustomPrettyPrinterLocator(PrettyPrinter):
    """Given a gdb.Value, search for a custom pretty printer"""

    def __init__(self):
        super(CustomPrettyPrinterLocator, self).__init__(
            "my_pretty_printers", []
        )

    def __call__(self, val):
        """Return the custom formatter if the type can be handled"""

        typename = gdb.types.get_basic_type(val.type).tag
        if typename is None:
            typename = val.type.name

        if typename == "Uuid":
            return UuidPrettyPrinter(val)


register_pretty_printer(None, CustomPrettyPrinterLocator(), replace=True)


class UuidListDumpCmd(gdb.Command):
    """Prints the ListNode from our example in a nice format!"""

    def __init__(self):
        super(UuidListDumpCmd, self).__init__(
            "uuid_list_dump", gdb.COMMAND_USER
        )

    def _uuid_list_to_str(self, val):
        """Walk through the UuidListNode list.

        We will simply follow the 'next' pointers until we encounter NULL
        """
        idx = 0
        node_ptr = val
        result = ""
        while node_ptr != 0:
            uuid = node_ptr["uuid"]
            result += "\n%d: Addr: 0x%x, uuid: %s" % (idx, node_ptr, uuid)
            node_ptr = node_ptr["next"]
            idx += 1
        result = ("Found a Linked List with %d nodes:" % idx) + result
        return result

    def complete(self, text, word):
        # We expect the argument passed to be a symbol so fallback to the
        # internal tab-completion handler for symbols
        return gdb.COMPLETE_SYMBOL

    def invoke(self, args, from_tty):
        # We can pass args here and use Python CLI utilities like argparse
        # to do argument parsing
        print("Args Passed: %s" % args)

        node_ptr_val = gdb.parse_and_eval(args)
        if str(node_ptr_val.type) != "UuidListNode *":
            print("Expected pointer argument of type (UuidListNode *)")
            return

        print(self._uuid_list_to_str(node_ptr_val))


UuidListDumpCmd()
