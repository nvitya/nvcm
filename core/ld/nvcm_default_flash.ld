/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     nvcm_default_flash.ld
 *  brief:    Generic NVCM linker script for MCUs with flash memory
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

/* memory definition required, like

  MEMORY
  {
    FLASH (rx) :    ORIGIN = 0x00000000, LENGTH = 1024K

    RAM (xrw)  :    ORIGIN = 0x10000000, LENGTH = 128K
    ITCRAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 128K
    DTCRAM (xrw) :  ORIGIN = 0x30000000, LENGTH = 128K
    RAM2 (xrw) :    ORIGIN = 0x40000000, LENGTH = 128K
  }

if some regions missing they must be presented as alias like:

  REGION_ALIAS("ITCRAM", RAM);

  REGION_ALIAS("TEXT", FLASH);

*/

INCLUDE nvcm_stack_predef.ld

ENTRY(_start)

SECTIONS
{
  .isr_vector : ALIGN(4)
  {
    FILL(0xFF)
    __vectors_start = ABSOLUTE(.) ;
    KEEP(*(.isr_vector))
    __vectors_end = ABSOLUTE(.) ;

  } > FLASH

  .inits : ALIGN(4)
  {
    /* DATA COPY REGIONS */

    __data_regions_array_start = .;

    LONG(LOADADDR(.data));         LONG(ADDR(.data));         LONG(ADDR(.data)+SIZEOF(.data));
    LONG(LOADADDR(.data_DTCRAM));  LONG(ADDR(.data_DTCRAM));  LONG(ADDR(.data_DTCRAM)+SIZEOF(.data_DTCRAM));
    LONG(LOADADDR(.data_ITCRAM));  LONG(ADDR(.data_ITCRAM));  LONG(ADDR(.data_ITCRAM)+SIZEOF(.data_ITCRAM));
    LONG(LOADADDR(.data_RAM2));    LONG(ADDR(.data_RAM2));    LONG(ADDR(.data_RAM2)+SIZEOF(.data_RAM2));

    LONG(LOADADDR(.text_ITCRAM));  LONG(ADDR(.text_ITCRAM));  LONG(ADDR(.text_ITCRAM)+SIZEOF(.text_ITCRAM));
    LONG(LOADADDR(.text_DTCRAM));  LONG(ADDR(.text_DTCRAM));  LONG(ADDR(.text_DTCRAM)+SIZEOF(.text_DTCRAM));
    LONG(LOADADDR(.text_RAM2));    LONG(ADDR(.text_RAM2));    LONG(ADDR(.text_RAM2)+SIZEOF(.text_RAM2));

    __data_regions_array_end = .;

    /* DATA CLEAR REGIONS */

    __bss_regions_array_start = .;

    LONG(ADDR(.bss));         LONG(ADDR(.bss)+SIZEOF(.bss));
    LONG(ADDR(.bss_DTCRAM));  LONG(ADDR(.bss_DTCRAM)+SIZEOF(.bss_DTCRAM));
    LONG(ADDR(.bss_ITCRAM));  LONG(ADDR(.bss_ITCRAM)+SIZEOF(.bss_ITCRAM));
    LONG(ADDR(.bss_RAM2));    LONG(ADDR(.bss_RAM2)+SIZEOF(.bss_RAM2));

    __bss_regions_array_end = .;

    INCLUDE nvcm_cppinit.ld

  } > FLASH

  .startup      : ALIGN(4)  {  KEEP(*(.startup))  } > FLASH

  .text_ITCRAM  : ALIGN(4)  { *(.text_ITCRAM) } > ITCRAM  AT > FLASH
  .text_DTCRAM  : ALIGN(4)  { *(.text_DTCRAM) } > DTCRAM  AT > FLASH
  .text_RAM2    : ALIGN(4)  { *(.text_RAM2) }   > RAM2    AT > FLASH
  .text : ALIGN(4)
  {
    *(.text .text.*)
    *(.rodata .rodata.* .constdata .constdata.*)
    *(vtable)

    KEEP(*(.eh_frame*))

    *(.glue_7)
    *(.glue_7t)

  } > TEXT

  . = ALIGN(4);
  _etext = .;
  __etext = .;

  .data_DTCRAM  : ALIGN(4)  { *(.data_DTCRAM) } > DTCRAM  AT > FLASH
  .data_ITCRAM  : ALIGN(4)  { *(.data_ITCRAM) } > ITCRAM  AT > FLASH
  .data_RAM2    : ALIGN(4)  { *(.data_RAM2) }   > RAM2    AT > FLASH
  .data : ALIGN(4)
  {
    *(.data_begin .data_begin.*)
    *(.data .data.*)
    *(.data_end .data_end.*)
    . = ALIGN(4);

  } > RAM AT > FLASH

  .bss_DTCRAM (NOLOAD) : ALIGN(4) { *(.bss_DTCRAM) } > DTCRAM
  .bss_ITCRAM (NOLOAD) : ALIGN(4) { *(.bss_ITCRAM) } > ITCRAM
  .bss_RAM2   (NOLOAD) : ALIGN(4) { *(.bss_RAM2) }   > RAM2
  .bss (NOLOAD) : ALIGN(4)
  {
    *(.bss_begin .bss_begin.*)
    *(.bss .bss.*)
    *(COMMON)
    *(.bss_end .bss_end.*)
  } > RAM

  .noinit_DTCRAM (NOLOAD) : ALIGN(4)  { *(.noinit_DTCRAM) } > DTCRAM
  .noinit_ITCRAM (NOLOAD) : ALIGN(4)  { *(.noinit_ITCRAM) } > ITCRAM
  .noinit_RAM2   (NOLOAD) : ALIGN(4)  { *(.noinit_RAM2) }   > RAM2
  .noinit (NOLOAD) : ALIGN(4)
  {
    _noinit = .;
    *(.noinit .noinit.*)
    . = ALIGN(4) ;
    _end_noinit = .;
  } > RAM

  INCLUDE nvcm_stack_heap.ld
  INCLUDE nvcm_debug_sections.ld
}
