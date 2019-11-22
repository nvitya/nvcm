
MEMORY
{
  /* ! the first 512 bytes were not writeable ! */
  ITCRAM (rwx) :  ORIGIN = 0x00000400, LENGTH = 191K
  RAM2 (xrw) :    ORIGIN = 0x20000000, LENGTH = 160K
}

REGION_ALIAS("RAM", ITCRAM);
REGION_ALIAS("DTCRAM", ITCRAM);
REGION_ALIAS("TEXT", ITCRAM);

INCLUDE nvcm_default_noflash.ld
