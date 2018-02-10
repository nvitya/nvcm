/*  NVCM default linker script for STM32F03xx6

It is very simple, it uses only a few continous sections.
If you need more, you have to create your own custom version.

*/

MEMORY
{
  FLASH_VECTORS (rx)  : ORIGIN = 0x08000000, LENGTH = 0x200
  FLASH_SPECIALS (rx) : ORIGIN = 0x08000000, LENGTH = 0  /* no special (flash protection section */
  FLASH (rx)          : ORIGIN = 0x08000200, LENGTH = 32K - 0x200

  RAM (xrw)           : ORIGIN = 0x20000000, LENGTH = 4K
  CCMRAM (xrw)        : ORIGIN = 0x00000000, LENGTH = 0
}

INCLUDE src/nvcm_default.ld
