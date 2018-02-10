MEMORY
{
  FLASH (rx) :    ORIGIN = 0x1A000000, LENGTH = 256K
  FLASH2 (rx) :   ORIGIN = 0x1B000000, LENGTH = 256K

  /* in fact the ITCRAM and DTCRAM are not specific to I and D buses, but it is better for the compatibility so */
  ITCRAM (rwx) :  ORIGIN = 0x10000000, LENGTH = 32K
  DTCRAM (xrw) :  ORIGIN = 0x10080000, LENGTH = 40K
  RAM2 (xrw) :    ORIGIN = 0x20000000, LENGTH = 64K
}

REGION_ALIAS("RAM", DTCRAM);
REGION_ALIAS("TEXT", FLASH);

INCLUDE nvcm_default_flash.ld
