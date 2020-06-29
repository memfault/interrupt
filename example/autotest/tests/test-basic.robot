*** Settings ***
Suite Setup       Setup
Suite Teardown    Teardown
Test Setup        Reset Emulation
Resource          ${RENODEKEYWORDS}

*** Variables ***
${SHELL_PROMPT}    shell>

*** Keywords ***
Start Test
    Execute Command    mach create
    Execute Command    machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
    Execute Command    machine LoadPlatformDescription @${PWD_PATH}/add-ccm.repl
    Execute Command    sysbus LoadELF @${PWD_PATH}/build/renode-example.elf
    Create Terminal Tester    sysbus.uart2
    Start Emulation

*** Test Cases ***
Help Menu
    [Documentation]           Ping Pong
    [Tags]                    critical

    Start Test

    Wait For Prompt On Uart    ${SHELL_PROMPT}
    Write Line To Uart    help
    Wait For Line On Uart    help: Lists all commands


Ping
    [Documentation]           Prints help menu of the command prompt
    [Tags]                    non_critical

    Start Test

    Wait For Prompt On Uart    ${SHELL_PROMPT}
    Write Line To Uart    ping
    Wait For Line On Uart    PONG

Read Var
    [Documentation]           Reads prompt output into variable
    [Tags]                    critical

    Start Test

    Wait For Prompt On Uart    ${SHELL_PROMPT}
    Write Line To Uart    coredump
    ${p}=  Wait For Line On Uart  coredump:(.*)  timeout=10 treatAsRegex=true
    Log    We got ${p}!

