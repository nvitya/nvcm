# NVCM ARM Cortex-M Microcontroller Framework

The NVCM is an open source, object based, true multi-vendor C++ software framework for ARM Cortex-M microcontrollers. It allows to create easily MCU vendor-independent or multi-MCU software, while the code stays small and very effective.

## NVCM Focuses on Ease of Use

GPIO Setup on STM32F103 Minimum Development Board:
```C++
TGpioPin  led1pin(PORTNUM_C, 13, false);
void setup_board()
{
	led1pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
}
```

GPIO Setup on Arduino DUE (ATSAM3X8E):
```C++
TGpioPin  led1pin(1, 27, false); // D13
void setup_board()
{
	led1pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
}
```

GPIO Setup on LPC4330-XPlorer (independent Pin and GPIO numbering):
```C++
TGpioPin  led1pin(1, 12, true); // D2
TGpioPin  led2pin(1, 11, true); // D3
void setup_board()
{
	hwpinctrl.PinSetup(2, 12, PINCFG_OUTPUT | PINCFG_AF_0);  // D2: GPIO_1_12, pad B9
	hwpinctrl.PinSetup(2, 11, PINCFG_OUTPUT | PINCFG_AF_0);  // D3: GPIO_1_11, pad A9
	led1pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
	led2pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
}
```
GPIO Usage:
```C++
++hbcounter;
led1pin.SetTo(hbcounter >> 0);
led1pin.Set0();  // fast inline code on most MCUs
led1pin.Set1();
```
UART Setup (STM32F103) and Usage:
```C++
hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_0);  // USART1_RX

THwUart   conuart;
conuart.Init(1);  // USART1, use default settings: 115200, 8, 1, n
conuart.printf("Hello World %i !\r\n", 1);
```

## Quick Start

There are several multi-board examples which reside in a separate repository: [https://github.com/nvitya/nvcmtests].
Download/clone this repository and follow the instructions there.

## Microcontroller Support

The NVCM achieves the vendor independence in the hard way. It uses only official vendor peripheral definitions (some are included into the project) but none of the vendor libraries. NVCM has for every supported microcontroller and peripheral its own driver.

The NVCM drivers try to be small and simple, and usually do not cover every possibility of the underlying HW peripherals.

The NVCM has a set of built-in microcontrollers and there is a clear way how to extend with unsupported ones.

## Microcontroller Families

The microcontroller vendors produce a lot of different microcontrollers. They might differ in RAM size, Flash size, Package, Pin count, Periperals etc. Those microcontrollers, that share the same drivers belong to the same NVCM microcontroller family.

### Currently Included MCU Families

__Family__ | __Sub-Family__
-----------|------------------------
__ATSAM__  | 3X, 4S, E70/S70/V70
__LPC__    | LPC43xx
__LPC_V3__ | LPC546xx
__STM32__  | F0, L0, F1, F3, F4, F7

From a former (unpublished) state of the project these families will come soon:
 * ATSAM_V2 (D10, D20, D51/E5x)
 * KINETIS (K20, KL03)
 * XMC (XMC1xxx)

### Current Driver Status

  Family   | __PINCFG<br/>+ GPIO__ | __CPU<br/>SPEED__ | __DMA__ | __UART__ | __SPI__ | __QSPI__
-----------|-----------------------|-------------------|---------|----------|---------|---------
__ATSAM__  | OK                    | OK                | OK      | OK       | OK      | OK
__LPC__    | OK                    | OK                | OK      | OK       | OK      | OK
__LPC_V3__ | OK                    | OK                | soon    | OK       | soon    | -
__STM32__  | OK                    | OK                | OK      | OK       | OK      | -

### Planned Drivers
 * i2c
 * USB Device

## Included HW Module Drivers
This NVCM core project contains some useful external module drivers as well. Currently these modules are included into the core:
 * TFT LCD displays: SPI, Parallel
 * SPI, QSPI flash memories
 * Led and Key module


