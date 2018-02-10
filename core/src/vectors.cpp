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
 *  file:     vectors.cpp
 *  brief:    Generic vector table for all Cortex-M MCUs
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *
 *  Notes:
 *     Some ideas are taken from Liviu Ionescu (https://github.com/micro-os-plus)
*/

#include "platform.h"

extern "C" // these functions are with C naming defined
{

// ----------------------------------------------------------------------------

extern "C" void _start(); // the application must define this

void __attribute__((weak))
Default_Handler(void);

void NMI_Handler        ( void );
void HardFault_Handler  ( void );
void MemManage_Handler  ( void );
void BusFault_Handler   ( void );
void UsageFault_Handler ( void );
void SVC_Handler        ( void );
void DebugMon_Handler   ( void );
void PendSV_Handler     ( void );
void SysTick_Handler    ( void );

/* Peripherals handlers */
void IRQ_Handler_00       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_01       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_02       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_03       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_04       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_05       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_06       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_07       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_08       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_09       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_10       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_11       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_12       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_13       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_14       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_15       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_16       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_17       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_18       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_19       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_20       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_21       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_22       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_23       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_24       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_25       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_26       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_27       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_28       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_29       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_30       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_31       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_32       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_33       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_34       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_35       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_36       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_37       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_38       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_39       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_40       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_41       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_42       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_43       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_44       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_45       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_46       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_47       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_48       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_49       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_50       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_51       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_52       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_53       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_54       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_55       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_56       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_57       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_58       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_59       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_60       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_61       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_62       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_63       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_64       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_65       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_66       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_67       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_68       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_69       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_70       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_71       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_72       ( void ) __attribute__ ((weak, alias("Default_Handler")));

extern unsigned int __stack;  // defined in the linker script
//extern int __valid_user_code_checksum;

typedef void (* pHandler)(void);

__attribute__ ((section(".isr_vector"),used))
pHandler __isr_vectors[] =
{
	// Configure Initial Stack Pointer, using linker-generated symbols
	(pHandler) (&__stack),
	(pHandler) _start, // application entry

	(pHandler) NMI_Handler,
	(pHandler) HardFault_Handler,
	(pHandler) MemManage_Handler,
	(pHandler) BusFault_Handler,
	(pHandler) UsageFault_Handler,
	(pHandler) (0UL),           // Valid user code checksum for NXP
	(pHandler) (0UL),           // Reserved
	(pHandler) (0UL),           // Reserved
	(pHandler) (0UL),           // Reserved
	(pHandler) SVC_Handler,
	(pHandler) DebugMon_Handler,
	(pHandler) (0UL),           // Reserved
	(pHandler) PendSV_Handler,
	(pHandler) SysTick_Handler,

	// Peripheral interrupts
	(pHandler) IRQ_Handler_00,
	(pHandler) IRQ_Handler_01,
	(pHandler) IRQ_Handler_02,
	(pHandler) IRQ_Handler_03,
	(pHandler) IRQ_Handler_04,
	(pHandler) IRQ_Handler_05,
	(pHandler) IRQ_Handler_06,
	(pHandler) IRQ_Handler_07,
	(pHandler) IRQ_Handler_08,
	(pHandler) IRQ_Handler_09,

	(pHandler) IRQ_Handler_10,
	(pHandler) IRQ_Handler_11,
	(pHandler) IRQ_Handler_12,
	(pHandler) IRQ_Handler_13,
	(pHandler) IRQ_Handler_14,
	(pHandler) IRQ_Handler_15,
	(pHandler) IRQ_Handler_16,
	(pHandler) IRQ_Handler_17,
	(pHandler) IRQ_Handler_18,
	(pHandler) IRQ_Handler_19,

	(pHandler) IRQ_Handler_20,
	(pHandler) IRQ_Handler_21,
	(pHandler) IRQ_Handler_22,
	(pHandler) IRQ_Handler_23,
	(pHandler) IRQ_Handler_24,
	(pHandler) IRQ_Handler_25,
	(pHandler) IRQ_Handler_26,
	(pHandler) IRQ_Handler_27,
	(pHandler) IRQ_Handler_28,
	(pHandler) IRQ_Handler_29,

	(pHandler) IRQ_Handler_30,
	(pHandler) IRQ_Handler_31,
	(pHandler) IRQ_Handler_32,
	(pHandler) IRQ_Handler_33,
	(pHandler) IRQ_Handler_34,
	(pHandler) IRQ_Handler_35,
	(pHandler) IRQ_Handler_36,
	(pHandler) IRQ_Handler_37,
	(pHandler) IRQ_Handler_38,
	(pHandler) IRQ_Handler_39,

	(pHandler) IRQ_Handler_40,
	(pHandler) IRQ_Handler_41,
	(pHandler) IRQ_Handler_42,
	(pHandler) IRQ_Handler_43,
	(pHandler) IRQ_Handler_44,
	(pHandler) IRQ_Handler_45,
	(pHandler) IRQ_Handler_46,
	(pHandler) IRQ_Handler_47,
	(pHandler) IRQ_Handler_48,
	(pHandler) IRQ_Handler_49,

	(pHandler) IRQ_Handler_50,
	(pHandler) IRQ_Handler_51,
	(pHandler) IRQ_Handler_52,
	(pHandler) IRQ_Handler_53,
	(pHandler) IRQ_Handler_54,
	(pHandler) IRQ_Handler_55,
	(pHandler) IRQ_Handler_56,
	(pHandler) IRQ_Handler_57,
	(pHandler) IRQ_Handler_58,
	(pHandler) IRQ_Handler_59,

	(pHandler) IRQ_Handler_60,
	(pHandler) IRQ_Handler_61,
	(pHandler) IRQ_Handler_62,
	(pHandler) IRQ_Handler_63,
	(pHandler) IRQ_Handler_64,
	(pHandler) IRQ_Handler_65,
	(pHandler) IRQ_Handler_66,
	(pHandler) IRQ_Handler_67,
	(pHandler) IRQ_Handler_68,
	(pHandler) IRQ_Handler_69,

	(pHandler) IRQ_Handler_70,
	(pHandler) IRQ_Handler_71,
	(pHandler) IRQ_Handler_72
};

// Processor ends up here if an unexpected interrupt occurs or a
// specific handler is not present in the application code.
// When in DEBUG, trigger a debug exception to clearly notify
// the user of the exception and help identify the cause.

extern void Default_Handler(void)
{
	#if defined(DEBUG)
	__DEBUG_BKPT();
	#endif
	while (1)
	{
	}
}

// ----------------------------------------------------------------------------
// Default exception handlers. Override the ones here by defining your own
// handler routines in your application code.
// ----------------------------------------------------------------------------

void __attribute__ ((section(".after_vectors"),weak))
NMI_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak,naked))
HardFault_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak,naked))
BusFault_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak,naked))
UsageFault_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak))
SVC_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak))
PendSV_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak))
MemManage_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak))
DebugMon_Handler (void)
{
	__DEBUG_BKPT();
  while (1)
  {
  }
}

void __attribute__ ((section(".after_vectors"),weak))
SysTick_Handler (void)
{
  // DO NOT loop, just return.
  // Useful in case someone inadvertently enables SysTick.
}

} // extern "C"

