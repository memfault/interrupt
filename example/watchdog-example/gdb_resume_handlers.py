try:
    import gdb
except ImportError:
    raise Exception("This script can only be run within gdb!")

import struct


class Nrf52Timer0:
    TIMER0_TASKS_START_ADDR = 0x40008000
    TIMER0_TASKS_STOP_ADDR = 0x40008004

    @staticmethod
    def _trigger_task(addr):
        gdb.selected_inferior().write_memory(addr, struct.pack("<I", 1))

    @classmethod
    def start(cls):
        print("Resuming Software Watchdog")
        cls._trigger_task(cls.TIMER0_TASKS_START_ADDR)

    @classmethod
    def stop(cls):
        print("Pausing Software Watchdog")
        cls._trigger_task(cls.TIMER0_TASKS_STOP_ADDR)


def nrf52_debug_stop_handler(event):
    Nrf52Timer0.stop()


def nrf52_debug_start_handler(event):
    Nrf52Timer0.start()


gdb.events.stop.connect(nrf52_debug_stop_handler)
gdb.events.cont.connect(nrf52_debug_start_handler)
