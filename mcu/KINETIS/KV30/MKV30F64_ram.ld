MEMORY
{
  ITCRAM (rwx) :  ORIGIN = 0x10000000, LENGTH = 0K
  DTCRAM (xrw) :  ORIGIN = 0x10000000, LENGTH = 0K

  CODERAM (xrw): ORIGIN = 0x1FFFE000, LENGTH = 8K
  RAM (xrw)    : ORIGIN = 0x20000000, LENGTH = 8K
}

REGION_ALIAS("TEXT", CODERAM);

INCLUDE nvcm_default_noflash.ld
