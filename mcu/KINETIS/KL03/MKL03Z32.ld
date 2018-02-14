/*  NVCM default linker script

It is very simple, it uses only a few continous sections.
If you need more, you have to create your own custom version.

*/

MEMORY
{
  FLASH_VECTORS (rx)  : ORIGIN = 0x00000000, LENGTH = 0x200
  FLASH_SPECIALS (rx) : ORIGIN = 0x00000400, LENGTH = 16  /* flash protection section */
  FLASH (rx)          : ORIGIN = 0x00000420, LENGTH = 32K - 0x420
  RAM (xrw)           : ORIGIN = 0x1FFFFE00, LENGTH = 2K
  CCMRAM (xrw)        : ORIGIN = 0x00000000, LENGTH = 0K
}

INCLUDE src/nvcm_default.ld
