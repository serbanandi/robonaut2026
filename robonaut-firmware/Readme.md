# RobonAUT 2026 Firmware Project

This repository provides a **template STM32 project** for participants of the **RobonAUT 2026 competition**. The competition is organized by BME-AUT. 
It is intended as a clean starting point to quickly set up and develop embedded software for STM32-based robotic platforms.  

## Features
- Ready-to-build STM32CubeIDE project structure for STM32N6 Nucleo-144 board
- Working linesensor driver
- Git-friendly structure 

## First-time setup
- Build the FSBL and the Application projects
- Set DEV boot mode: BOOT0 = 1-2, BOOT1 = 2-3
- Power on the board
- Program the FSBL to the external flash:
    - Start STM32CubeProgrammer, and connect to the board
    - Set the external loader (EL) in the STM32CubeProgrammer: STM32N6570-NUCLEO Board
    - Set 1.8 V for XSPI1: 
        - Select OTP programming
        - Find the OTP124 register (HCONF1)
        - Set HSLV_VDDIO3 bit to 1 and click Apply
        - Read back the bit value
    - Select the signed FSBL binary: robonaut_firmware_FSBL-Trusted.bin
    - Set the start address: 0x70000000
    - Click Start Programming
    - Exit the STM32CubeProgrammer
- Set FLASH boot mode: BOOT0 = 1-2, BOOT1 = 1-2
- Do a power cycle for the board
- Program the Application project with *RobonAUT FLASH prog* debug configuration

## Requirements
- **IDE**: [STM32CubeIDE 1.19.0](https://www.st.com/en/development-tools/stm32cubeide.html)
- **UTILITY**: [STM32CubeProgrammer 2.20.0](https://www.st.com/en/development-tools/stm32cubeprog.html)
- **Git** for version control 

## Known Limitations
- No automatic build for Flash Debug Config. If you enable the automatic build, it will build the project after flashing the device.
- Cannot set breakpoints before any debug session. You need to remove any breakpoints before debugging your code. Otherwise, the debug session fails, and you must reconnect the board.

## Useful Links
- CPU Reference Manual: [STM32N657xx](https://www.st.com/resource/en/reference_manual/rm0486-stm32n647657xx-armbased-32bit-mcus-stmicroelectronics.pdf)
- Nucleo User Manual: [STM32N6 Nucleo-144](https://www.st.com/resource/en/user_manual/um3417-stm32n6-nucleo144-board-mb1940-stmicroelectronics.pdf)
