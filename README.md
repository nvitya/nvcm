# NVCM ARM Cortex-M Microcontroller Framework

The project on the Github: [https://github.com/nvitya/nvcm](https://github.com/nvitya/nvcm)

The NVCM is an open source, object based, true multi-vendor C++ software framework for ARM Cortex-M microcontrollers. It allows to create easily MCU vendor-independent or multi-MCU software, while the code stays small and very effective.

# Currently Included MCU Families

Those microcontrollers, that share the same drivers belong to the same NVCM microcontroller family.

__Family__   | __Sub-Family__
-------------|------------------------
__ATSAM__    | [3X, 4S, E70/S70/V70](https://github.com/nvitya/nvcm/blob/master/mcu/ATSAM/src/mcu_builtin.h)
__ATSAM_V2__ | [D51/E5x, C2x, D10](https://github.com/nvitya/nvcm/blob/master/mcu/ATSAM_V2/src/mcu_builtin.h)
__IMXRT__    | [IMXRT1020](https://github.com/nvitya/nvcm/blob/master/mcu/IMXRT/src/mcu_builtin.h)
__KINETIS__  | [K20, KL03, KV30](https://github.com/nvitya/nvcm/blob/master/mcu/KINETIS/src/mcu_builtin.h)
__LPC__      | [LPC43xx](https://github.com/nvitya/nvcm/blob/master/mcu/LPC/src/mcu_builtin.h)
__LPC_V2__   | [LPC8xx](https://github.com/nvitya/nvcm/blob/master/mcu/LPC_V2/src/mcu_builtin.h)
__LPC_V3__   | [LPC546xx](https://github.com/nvitya/nvcm/blob/master/mcu/LPC_V3/src/mcu_builtin.h)
__STM32__    | [F0, L0, F1, F3, F4, F7](https://github.com/nvitya/nvcm/blob/master/mcu/STM32/src/mcu_builtin.h)
__XMC__      | [XMC1xxx, XMC4xxx](https://github.com/nvitya/nvcm/blob/master/mcu/XMC/src/mcu_builtin.h)


# Integrated Peripheral Drivers

The NVCM defines a simple, unified object interface for the MCU integrated peripherals. For the implemenatation no MCU vendor libraries are used, the drivers are directly manipulating the peripheral registers.

## Current Integrated Peripheral Driver Status

  Driver              |__ATSAM__|__STM32__|__LPC__|__XMC__|__ATSAM_V2__|__LPC_V2__|__LPC_V3__|__IMXRT__|__KINETIS__|
----------------------|---------|---------|-------|-------|------------|----------|----------|---------|-----------|
__CPU Speed__         | OK      | OK      | OK    | OK    | OK         | Partial  | OK       | OK      | Partial   |
__Pin Cfg. + GPIO__   | OK      | OK      | OK    | OK    | OK         | OK       | OK       | OK      | OK        |
__UART__              | OK      | OK      | OK    | OK    | OK         | OK       | OK       | OK      | OK        |
__DMA__               | OK      | OK      | OK    | -     | OK         | -        | OK       | -       | -         |
__SPI Master__        | OK      | OK      | OK    | OK    | OK         | -        | OK       | -       | -         |
__I2C Master__        | OK      | OK      | -     | OK    | OK         | -        | OK       | -       | -         |
__I2C Slave__         | -       | OK      | -     | -     | -          | -        | -        | -       | -         |
__Internal Flash__    | OK      | OK      | -     | -     | OK         | -        | -        | -       | -         |
__ADC__               | OK      | OK      | OK    | -     | -          | -        | -        | -       | -         |
__Simple PWM__        | -       | OK      | -     | -     | -          | -        | -        | -       | -         |
__QSPI__              | OK      | OK      | OK    | -     | OK         | -        | OK       | -       | -         |
__Ethernet__          | OK      | OK      | -     | -     | -          | -        | -        | -       | -         |
__CAN__               | OK      | OK      | -     | -     | OK         | -        | -        | -       | -         |
__USB Device__        | -       | exp.    | -     | -     | -          | -        | -        | -       | -         |
__SD Card__           | exp.    | -       | -     | -     | -          | -        | -        | -       | -         |

# Built-In Boards

Some Popular Development Boards, that are included into the NVCM source tree:

__Family__ | __Board Id.__          | __Name__
-----------|------------------------|-------------------------------------
ATSAM      | BOARD_ARDUINO_DUE      | [Arduino DUE (ATSAM3X8E)](https://store.arduino.cc/usa/arduino-due)
ATSAM      | BOARD_XPLAINED_SAME70  | [SAM E70 Xplained Evaluation Kit](http://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=ATSAME70-XPLD)
LPC        | BOARD_XPRESSO_LPC4337  | [LPCXpresso4337](https://www.nxp.com/support/developer-resources/hardware-development-tools/lpcxpresso-boards/lpcxpresso4337-development-board:OM13070)
LPC        | BOARD_XPLORER_LPC4330  | [LPC4330-Xplorer](https://www.nxp.com/support/developer-resources/hardware-development-tools/lpcxpresso-boards/lpc4330-xplorer-board:OM13027)
LPC_V3     | BOARD_XPRESSO_LPC54608 | [LPCXpresso54608](https://www.nxp.com/support/developer-resources/hardware-development-tools/lpcxpresso-boards/lpcxpresso-development-board-for-lpc5460x-mcus:OM13092)
STM32      | BOARD_MIN_F103         | [STM32F103C8 Minimum Development Board (breadboard friendly)](https://www.aliexpress.com/w/wholesale-stm32f103c8t6-minimum.html)
STM32      | BOARD_DISCOVERY_F746   | [Discovery kit with STM32F746NG](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)
STM32      | BOARD_NUCLEO_F746      | [NUCLEO-F746ZG](http://www.st.com/en/evaluation-tools/nucleo-f746zg.html)
STM32      | BOARD_NUCLEO_F446      | [NUCLEO-F446RE](http://www.st.com/en/evaluation-tools/nucleo-f446re.html)
STM32      | BOARD_DISCOVERY_F072   | [Discovery kit with STM32F072RB MCU](http://www.st.com/en/evaluation-tools/32f072bdiscovery.html)
STM32      | BOARD_DEV_STM32F407VG  | [STM32F407VG Minimal Board](https://www.aliexpress.com/item/STM32F4Discovery-STM32F407VGT6-ARM-Cortex-M4-32bit-MCU-Core-Development-Board/32757497307.html)
STM32      | BOARD_DEV_STM32F407ZE  | [STM32F407ZE Development Board](https://www.aliexpress.com/item/Free-shipping-STM32F407ZET6-development-board-M4-STM32F4-core-board-arm-development-board-cortex-M4/32689262341.html)
XMC        | BOARD_BOOT_XMC1200     | [XMC1200 Boot Kit](https://www.infineon.com/cms/de/product/evaluation-boards/kit_xmc12_boot_001/)

This list does not cover all built-in boards, and it is simple to define a board for a special project.

## Special Minimal Boards

__Family__ | __Board Id.__          | __Name__
-----------|------------------------|-------------------------------------
ATSAM      | BOARD_MIBO64_ATSAM4S   | [MIBO64-ATSAM4S](https://github.com/nvitya/minimal_boards/tree/master/mibo64_atsam4s)
ATSAM_V2   | BOARD_MIBO64_ATSAME5x  | [MIBO64-ATSAME5x](https://github.com/nvitya/minimal_boards/tree/master/mibo64-atsame5x)
STM32      | BOARD_MIBO20_STM32xxxx | [MIBO20-STM32](https://github.com/nvitya/minimal_boards/tree/master/mibo20_stm32)
STM32      | BOARD_MIBO48_STM32xxxx | [MIBO48-STM32](https://github.com/nvitya/minimal_boards/tree/master/mibo48_stm32)
STM32      | BOARD_MIBO64_STM32xxxx | [MIBO64-STM32](https://github.com/nvitya/minimal_boards/tree/master/mibo64_stm32)

# Module Drivers

This NVCM core project contains some useful external module drivers as well:
 * TFT LCD displays: SPI, Parallel
 * I2C OLED displays (SSD1306)
 * Some monochrome LCD displays
 * SPI, QSPI flash memories, I2C EEPROM
 * Led and Key module and some other serial 7 segment displays
 * Simple stepper motor

# Quick Start

There are several multi-board examples which reside in a separate repository: [https://github.com/nvitya/nvcmtests].
Download/clone this repository and follow the instructions there.

# Easy to Use

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
