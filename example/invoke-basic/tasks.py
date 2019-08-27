import os, shutil, sys
from invoke import Collection, Config, Exit, task
from shutil import which

# Constants
ROOT_DIR = os.path.dirname(__file__)
NRF_SDK_DIR = os.path.join(ROOT_DIR, "nrf5_sdk")
BLINKY_DIR = os.path.join(NRF_SDK_DIR, "examples", "peripheral", "blinky", 
                          "pca10056", "blank", "armgcc")
BLINKY_ELF = os.path.join(BLINKY_DIR, "_build", "nrf52840_xxaa.out")

GDB_EXE = "arm-none-eabi-gdb-py"
GCC_EXE = "arm-none-eabi-gcc"

JLINK_GDB_PORT = 2331
JLINK_TELNET_PORT = 19021


def _check_exe(exe, instructions_url):
    exe_path = which(exe)
    if not exe_path:
        msg = (
            "Couldn't find `{}`.\n"
            "This tool can be found here:\n\n"
            ">  {}".format(exe, instructions_url)
        )
        raise Exit(msg)

@task
def check_toolchain(ctx):
    """Run as a `pre` task to check for the presence of the ARM toolchain"""
    url = "https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads"
    _check_exe(GCC_EXE, url)
    _check_exe(GDB_EXE, url)
    

@task
def check_segger_tools(ctx):
    """Run as a `pre` task to check for the presence of the Segger tools"""
    url = "https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack"
    _check_exe("JLinkGDBServer", url)


@task(pre=[check_toolchain])
def build(ctx, ccache=True):
    """Build the project"""
    with ctx.cd(BLINKY_DIR):
        # To get nrf52 SDK to pick up GCC in our $PATH
        env = {"GNU_INSTALL_ROOT": ""}
        if ccache:
            env.update({"CCACHE": which("ccache")})
        ctx.run("make -j4", env=env)


@task(pre=[check_toolchain], help={
    "elf": "The elf to flash",
    "port": "The GDB port to attach to"
})
def flash(ctx, elf=BLINKY_ELF, port=JLINK_GDB_PORT):
    """Spawn GDB and flash application & softdevice
    
    Examples:
        # Flash the default ELF file over JLink
        $ inv flash

        # Flash a given binary over a JLink on the given port
        $ inv flash --elf /path/to/file.elf --port 12345

    """
    cmd = f'{GDB_EXE} --eval-command="target remote localhost:{JLINK_GDB_PORT}"' \
          f' --ex="mon reset" --ex="load" --ex="mon reset" --se={BLINKY_ELF}'
    ctx.run(cmd)


@task(pre=[check_toolchain], help={
        "elf": "The ELF file to pull symbols from",
        "port": "The GDB port to attach to"
})
def debug(ctx, elf=BLINKY_ELF, port=JLINK_GDB_PORT):
    """Spawn GDB and attach to the device without flashing"""
    cmd = f'{GDB_EXE} --eval-command="target remote localhost:{JLINK_GDB_PORT}"' \
          f' --se={BLINKY_ELF}'
    ctx.run(cmd)


@task(pre=[check_segger_tools], help={
    "gdb": "The GDB port to publish to",
    "telnet": "The Telnet port to publish to"
})
def gdbserver(ctx, gdb=JLINK_GDB_PORT, telnet=JLINK_TELNET_PORT):
    """Start JLinkGDBServer"""
    ctx.run(f"JLinkGDBServer -if swd -device nRF52840_xxAA -speed auto "
            f"-port {gdb} -RTTTelnetPort {telnet}")


@task(pre=[check_segger_tools])
def console(ctx, telnet=JLINK_TELNET_PORT):
    """Start an RTT console session"""
    ctx.run(f"JLinkRTTClient -LocalEcho Off -RTTTelnetPort {telnet}")
    pdb.set_trace()


# Add all tasks to the namespace
ns = Collection(build, console, debug, flash, gdbserver)
# Configure every task to act as a shell command (will print colors, allow interactive CLI)
# Add our extra configuration file for the project
config = Config(defaults={"run": {"pty": True}})
ns.configure(config)
