# NVCM ARM Cortex-M Framework

The NVCM is an open source, object based, vendor independent C++ software framework for ARM Cortex-M microcontrollers. It makes easier to create software for ARM Cortex-M based boards, while the code stays small and effective.

## Quick Start

There are several examples which reside in a separate repository: [https://github.com/nvitya/nvcmtests].
Download/clone this repository and follow the instructions there.

## Microcontroller Support

The NVCM achieves the vendor independence in the hard way. It uses only official vendor peripheral definitions (some are included into the project) but none of the vendor libraries. NVCM has for every supported microcontroller and peripheral its own driver.

The NVCM drivers try to be small and simple, and usually do not cover every possibility of the underlying HW peripherals.

The NVCM has a set of built-in microcontrollers and there is a clear way how to extend with unsupported ones.

## Microcontroller Families

The microcontroller vendors produce a lot of different microcontrollers. They might differ in RAM size, Flash size, Package, Pin count, Periperals etc. Those microcontrollers, that share the same drivers belong to the same NVCM microcontroller family.

## Currently Included MCU Families

__Family__ | __Sub-Family__
-----------|------------------------
__ATSAM__  | 3X, 4S, E70/S70/V70
__LPC__    | LPC43xx
__STM32__  | F0, L0, F1, F3, F4, F7

From a former (unpublished) state of the project these failies will come soon:
 * ATSAM_V2 (D10, D20, D51/E5x)
 * KINETIS (K20, KL03)
 * LPC_V3 (LPC546xx)
 * XMC (XMC1xxx)

### Driver Status

  Family   | __PINCFG <br/>+ GPIO__ | __CPU SPEED__ | __DMA__ | __UART__ | __SPI__ | __QSPI__
-----------|-------------------|---------------|---------|----------|---------|---------
__ATSAM__  | OK                | OK            | OK      | OK       | OK      | OK
__LPC__    | OK                | OK            | OK      | OK       | OK      | OK
__STM32__  | OK                | OK            | OK      | OK       | OK      | -

### Planned Drivers
 * i2c
 * USB Device

## Modules
This NVCM core project contains some useful external module drivers as well. Currently these modules are included into the core:
 * TFT LCD displays: SPI, Parallel
 * SPI, QSPI flash memories
 * Led and Key module
Hopefully this list will grow too.

