/*  NVCM default linker script for ATSAMD10C13

It is very simple, it uses only a few continous sections.
If you need more, you have to create your own custom version.

*/

MEMORY
{
  FLASH_VECTORS (rx)  : ORIGIN = 0x00000000, LENGTH = 0x200
  FLASH_SPECIALS (rx) : ORIGIN = 0x00000000, LENGTH = 0  /* no special (flash protection section */
  FLASH (rx)          : ORIGIN = 0x00000200, LENGTH = 8K - 0x200

  RAM (xrw)           : ORIGIN = 0x20000000, LENGTH = 4K
  CCMRAM (xrw)        : ORIGIN = 0x00000000, LENGTH = 0
}

INCLUDE src/nvcm_default.ld
