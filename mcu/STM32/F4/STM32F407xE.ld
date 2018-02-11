MEMORY
{
  FLASH (rx) :    ORIGIN = 0x08000000, LENGTH = 512K
  DTCRAM (xrw) :  ORIGIN = 0x10000000, LENGTH = 64K
  RAM2 (xrw) :    ORIGIN = 0x20000000, LENGTH = 128K
}

REGION_ALIAS("TEXT", FLASH);
REGION_ALIAS("ITCRAM", RAM2);  /* no code execute possible in DTCRAM ! */
REGION_ALIAS("RAM", RAM2);     /* The DMA does not work with DTCRAM ! */

INCLUDE nvcm_default_flash.ld

