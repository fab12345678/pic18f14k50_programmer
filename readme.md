# PIC18F15K50 Programmer


This project implements a basic PIC18F15K50 iscp programmer on a STM32 microcontroller. The STM32 Standard Peripheral Library is used, so the code should be easy to adapt to other microcontrollers as well. The project is fully command line and Makefile based, so no user interface installation is required.

This repo consists of 3 folders:
- client
- pic_example_blink
- stm32


The purpose of this programmer was to reprogram the pic18 on the Numato Labs mimas spartan 6 fpga development board.

### Hardware setup

The pic18 is connected via the ICSP lines to the stm32. Five wires are required: PGD, PDC, GND, VCC, and MCLR. PDG and PDC are connected directly to digital pins on the STM32, GND and VCC to the supply lines. MCLR is tricky, because the controller requires a 9V supply here, if Low Voltage Programming is not used (which is not, because the LVP pins is used on the Mimas board). That means, that a simple additional circuit is required, that converts the 3.3V output of a stm32 pin to a switched 9V supply.

That can be reached for example with the following circuit:

![alt text](https://github.com/fab12345678/pic18f14k50_programmer/blob/main/schematic.png?raw=true)

### stm32

This folder contains the application code running on the stm32, that is used to program the pic18 microcontroller. Since this is a standalone project (no UI) required, it contains several files, along with the library. The installation of the toolchain for the stm32 is sufficient.
The stm32 receives the program data for the pic18 from the client software running on the computer. 

### client

This folder contains a program written in c, that takes the executable code for the pic microcontroller as a `.hex` file and sends it via serial over usb to the stm32 microcontroller.

### pic_example_blink

This folder contains an example code for the pic18f14k50 microcontroller, which blinks an LED connected to the microcontroller.

## Connections

## Getting started.

- Compile the STM32 code, and program the STM32 board (NUCLEO-F334R8 in my case).
- Program the STM32
- compile the client code
- compile the blink example
- use `make download` in the blink example folder to download the code to the pic18
- The led connected to the pic should now blink



## Further work that could be done

- use SPI on the stm32. PGD is input as well as output. Check if this is possible with the STM32
- adapt to work on arduino, or other common microcontrollers.