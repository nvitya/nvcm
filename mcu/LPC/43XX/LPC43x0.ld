MEMORY
{
  /* in fact the ITCRAM and DTCRAM are not specific to I and D buses, but for the compatibility so is better  */
  ITCRAM (rwx) :  ORIGIN = 0x10000000, LENGTH = 128K
  DTCRAM (xrw) :  ORIGIN = 0x10080000, LENGTH = 72K
  RAM2 (xrw) :    ORIGIN = 0x20000000, LENGTH = 64K
}

REGION_ALIAS("RAM", DTCRAM);
REGION_ALIAS("TEXT", ITCRAM);

INCLUDE nvcm_default_noflash.ld
