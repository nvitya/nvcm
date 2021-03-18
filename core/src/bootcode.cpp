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
 *  file:     bootcode.cpp
 *  brief:    Special boot code for FLASH to RAM copy applications
 *            (e.g. using nvcm_default_coderam.ld script)
 *  version:  1.00
 *  date:     2021-03-14
 *  authors:  nvitya
*/

#include "platform.h"

extern "C" // these functions are with C naming defined
{

// ----------------------------------------------------------------------------

void _bootcode_start();

void BootCode_Default_Handler(void);

extern unsigned int __stack;  // defined in the linker script

typedef void (* pHandler)(void);

__attribute__ ((section(".bootcode_data"),used))
const pHandler __bootcode_vectors[] =
{
	// Configure Initial Stack Pointer, using linker-generated symbols
	(pHandler) (&__stack),
	(pHandler) _bootcode_start,           // application entry

	(pHandler) BootCode_Default_Handler,
	(pHandler) BootCode_Default_Handler,
	(pHandler) BootCode_Default_Handler,  // offset = 0x10
	(pHandler) BootCode_Default_Handler,
	(pHandler) BootCode_Default_Handler,
	(pHandler) (0UL),              // offset = 0x1C, Valid user code checksum for NXP
	(pHandler) (0UL),              // offset = 0x20, Reserved
	(pHandler) (IRQVECTAB_OFFS_24_VALUE),       // offset = 0x24, NXP boot marker
	(pHandler) (IRQVECTAB_OFFS_28_VALUE),       // offset = 0x28, NXP boot block offset
	(pHandler) BootCode_Default_Handler,
	(pHandler) BootCode_Default_Handler,
	(pHandler) (0UL),           // Reserved
	(pHandler) BootCode_Default_Handler,
	(pHandler) BootCode_Default_Handler,

};

__attribute__ ((section(".bootcode_code"),used))
void BootCode_Default_Handler(void)
{
	while (1)
	{
	}
}

// these are defined in the linker script
extern unsigned __ramcode_regions_array_start;
extern unsigned __ramcode_regions_array_end;

__attribute__ ((section(".bootcode_code"),used))
void _bootcode_start() // cold boot entry point
{
	mcu_disable_interrupts();

	// the stack might not be set properly so set it
	asm("ldr  r0, =__stack");
	asm("mov  sp, r0");

  mcu_preinit_code();        // might required for RAM preparation, must be all inline !

	unsigned * recp;
	unsigned * loadaddr;
	unsigned * destaddr;
	unsigned * destaddrend;

	// 1. Copy the main code regions to the RAM

	recp = 	(unsigned *)&__ramcode_regions_array_start;
	while (recp < &__ramcode_regions_array_end)
	{
		loadaddr = (unsigned *)(*recp);
		++recp;
		destaddr = (unsigned *)(*recp);
		++recp;
		destaddrend = (unsigned *)(*recp);
		++recp;

	  // It is assumed that the pointers are word aligned.
	  while (destaddr < destaddrend)
	  {
	    *destaddr++ = *loadaddr++;
	  }
	}

	// 2. Set the vector table

  mcu_init_vector_table();   // set the vector table address

  // 3. Start the real application from the RAM

	asm("ldr  r1, =_start");
	asm("movs r0, #0"); // signalize (Cold) ROM boot
	asm("bx   r1");
}

} // extern "C"

