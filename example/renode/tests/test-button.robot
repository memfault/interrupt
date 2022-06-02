*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    Reset Emulation
Resource                      ${RENODEKEYWORDS}

*** Test Cases ***
Should Handle Button Press
    Execute Command         mach create
    Execute Command         machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
    Execute Command         machine LoadPlatformDescription @${PATH}/add-ccm.repl
    Execute Command         sysbus LoadELF @${PATH}/renode-example.elf

    Create Terminal Tester  sysbus.usart2

    Start Emulation

    Wait For Line On Uart   hello world
    Test If Uart Is Idle    3
    Execute Command         sysbus.gpioPortA.UserButton Press
    Test If Uart Is Idle    3
    Execute Command         sysbus.gpioPortA.UserButton Release
    Wait For Line On Uart   button pressed
