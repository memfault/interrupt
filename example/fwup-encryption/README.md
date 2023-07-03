# Introduction

A minimal Cortex-M startup enviornment targetting the B-L4S5I-IOT01A Dev kit written entirely in C written solely for illustrative purposes, MCU registers are accessed through CMSIS.

Generate two executable files:

- app.elf
- boot.elf

# Compiling

```bash
$ make
Compiling app.c
Compiling system_app.c
Compiling startup_app.s
Linking library app
Memory region         Used Size  Region Size  %age Used
             RAM:        2640 B       640 KB      0.40%
             ROM:        2176 B         4 KB     53.13%
Generated build/app.elf
Compiling boot.c
Compiling system_boot.c
Compiling Decrypting-EngineAES/aes_ctr.c
Compiling Decrypting-EngineAES/flash.c
Linking library boot
Memory region         Used Size  Region Size  %age Used
             RAM:        2672 B       640 KB      0.41%
             ROM:        2988 B         4 KB     72.95%
Generated build/boot.elf
```
# Encrypting firmware

```bash
$ make encryption
```

# Initialize J-link for executable download

```bash
$ make flash
JLink.exe -device STM32L4S5VI -speed 4000 -si swd -autoconnect 1
SEGGER J-Link Commander V7.80b (Compiled Sep 20 2022 17:11:36)
DLL version V7.80b, compiled Sep 20 2022 17:10:13
Connecting to J-Link via USB...O.K.
Firmware: J-Link STLink V21 compiled Aug 12 2019 10:29:20    
Hardware version: V1.00
J-Link uptime (since boot): N/A (Not supported by this model)
S/N: 775917048
VTref=3.300V
Device "STM32L4S5VI" selected.
Connecting to target via SWD  
Found SW-DP with ID 0x4BA01477
Found SW-DP with ID 0x4BA01477
DPv0 detected
CoreSight SoC-400 or earlier
Scanning AP map to find all available APs
AP[1]: Stopped AP scan as end of AP map has been reached
AP[0]: AHB-AP (IDR: 0x24770011)
Iterating through AP map to find AHB-AP to use
AP[0]: Core found
AP[0]: AHB-AP ROM base: 0xE00FF000
CPUID register: 0x410FC241. Implementer code: 0x41 (ARM)
Found Cortex-M4 r0p1, Little endian.
FPUnit: 6 code (BP) slots and 2 literal slots
CoreSight components:
ROMTbl[0] @ E00FF000
[0][0]: E000E000 CID B105E00D PID 000BB00C SCS-M7
[0][1]: E0001000 CID B105E00D PID 003BB002 DWT 
[0][2]: E0002000 CID B105E00D PID 002BB003 FPB 
[0][3]: E0000000 CID B105E00D PID 003BB001 ITM 
[0][4]: E0040000 CID B105900D PID 000BB9A1 TPIU
[0][5]: E0041000 CID B105900D PID 000BB925 ETM
Cortex-M4 identified.
J-Link>
```