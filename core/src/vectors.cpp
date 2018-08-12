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
void IRQ_Handler_73       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_74       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_75       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_76       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_77       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_78       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_79       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_80       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_81       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_82       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_83       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_84       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_85       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_86       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_87       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_88       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_89       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_90       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_91       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_92       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_93       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_94       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_95       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_96       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_97       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_98       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_99       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_100       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_101       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_102       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_103       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_104       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_105       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_106       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_107       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_108       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_109       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_110       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_111       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_112       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_113       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_114       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_115       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_116       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_117       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_118       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_119       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_120       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_121       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_122       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_123       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_124       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_125       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_126       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_127       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_128       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_129       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_130       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_131       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_132       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_133       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_134       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_135       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_136       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_137       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_138       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_139       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_140       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_141       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_142       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_143       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_144       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_145       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_146       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_147       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_148       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_149       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_150       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_151       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_152       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_153       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_154       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_155       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_156       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_157       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_158       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_159       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_160       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_161       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_162       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_163       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_164       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_165       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_166       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_167       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_168       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_169       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_170       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_171       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_172       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_173       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_174       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_175       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_176       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_177       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_178       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_179       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_180       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_181       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_182       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_183       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_184       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_185       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_186       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_187       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_188       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_189       ( void ) __attribute__ ((weak, alias("Default_Handler")));

void IRQ_Handler_190       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_191       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_192       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_193       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_194       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_195       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_196       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_197       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_198       ( void ) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler_199       ( void ) __attribute__ ((weak, alias("Default_Handler")));


extern unsigned int __stack;  // defined in the linker script
//extern int __valid_user_code_checksum;

typedef void (* pHandler)(void);

__attribute__ ((section(".isr_vector"),aligned(1024),used))  // it must be properly aligned, otherwise it won't work !
pHandler __isr_vectors[] =
{
	// Configure Initial Stack Pointer, using linker-generated symbols
	(pHandler) (&__stack),
	(pHandler) _start,           // application entry

	(pHandler) NMI_Handler,
	(pHandler) HardFault_Handler,
	(pHandler) MemManage_Handler,  // offset = 0x10
	(pHandler) BusFault_Handler,
	(pHandler) UsageFault_Handler,
	(pHandler) (0UL),              // offset = 0x1C, Valid user code checksum for NXP
	(pHandler) (0UL),              // offset = 0x20, Reserved
	(pHandler) (IRQVECTAB_OFFS_24_VALUE),       // offset = 0x24, NXP boot marker
	(pHandler) (IRQVECTAB_OFFS_28_VALUE),       // offset = 0x28, NXP boot block offset
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

#if MAX_IRQ_HANDLER_COUNT > 32
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
#endif

#if MAX_IRQ_HANDLER_COUNT > 64
	(pHandler) IRQ_Handler_64,
	(pHandler) IRQ_Handler_65,
	(pHandler) IRQ_Handler_66,
	(pHandler) IRQ_Handler_67,
	(pHandler) IRQ_Handler_68,
	(pHandler) IRQ_Handler_69,

	(pHandler) IRQ_Handler_70,
	(pHandler) IRQ_Handler_71,
	(pHandler) IRQ_Handler_72,
	(pHandler) IRQ_Handler_73,
	(pHandler) IRQ_Handler_74,
	(pHandler) IRQ_Handler_75,
	(pHandler) IRQ_Handler_76,
	(pHandler) IRQ_Handler_77,
	(pHandler) IRQ_Handler_78,
	(pHandler) IRQ_Handler_79,

	(pHandler) IRQ_Handler_80,
	(pHandler) IRQ_Handler_81,
	(pHandler) IRQ_Handler_82,
	(pHandler) IRQ_Handler_83,
	(pHandler) IRQ_Handler_84,
	(pHandler) IRQ_Handler_85,
	(pHandler) IRQ_Handler_86,
	(pHandler) IRQ_Handler_87,
	(pHandler) IRQ_Handler_88,
	(pHandler) IRQ_Handler_89,

	(pHandler) IRQ_Handler_90,
	(pHandler) IRQ_Handler_91,
	(pHandler) IRQ_Handler_92,
	(pHandler) IRQ_Handler_93,
	(pHandler) IRQ_Handler_94,
	(pHandler) IRQ_Handler_95,
#endif
#if MAX_IRQ_HANDLER_COUNT > 96
	(pHandler) IRQ_Handler_96,
	(pHandler) IRQ_Handler_97,
	(pHandler) IRQ_Handler_98,
	(pHandler) IRQ_Handler_99,

	(pHandler) IRQ_Handler_100,
	(pHandler) IRQ_Handler_101,
	(pHandler) IRQ_Handler_102,
	(pHandler) IRQ_Handler_103,
	(pHandler) IRQ_Handler_104,
	(pHandler) IRQ_Handler_105,
	(pHandler) IRQ_Handler_106,
	(pHandler) IRQ_Handler_107,
	(pHandler) IRQ_Handler_108,
	(pHandler) IRQ_Handler_109,

	(pHandler) IRQ_Handler_110,
	(pHandler) IRQ_Handler_111,
	(pHandler) IRQ_Handler_112,
	(pHandler) IRQ_Handler_113,
	(pHandler) IRQ_Handler_114,
	(pHandler) IRQ_Handler_115,
	(pHandler) IRQ_Handler_116,
	(pHandler) IRQ_Handler_117,
	(pHandler) IRQ_Handler_118,
	(pHandler) IRQ_Handler_119,

	(pHandler) IRQ_Handler_120,
	(pHandler) IRQ_Handler_121,
	(pHandler) IRQ_Handler_122,
	(pHandler) IRQ_Handler_123,
	(pHandler) IRQ_Handler_124,
	(pHandler) IRQ_Handler_125,
	(pHandler) IRQ_Handler_126,
	(pHandler) IRQ_Handler_127,
#endif
#if MAX_IRQ_HANDLER_COUNT > 128
	(pHandler) IRQ_Handler_128,
	(pHandler) IRQ_Handler_129,

	(pHandler) IRQ_Handler_130,
	(pHandler) IRQ_Handler_131,
	(pHandler) IRQ_Handler_132,
	(pHandler) IRQ_Handler_133,
	(pHandler) IRQ_Handler_134,
	(pHandler) IRQ_Handler_135,
	(pHandler) IRQ_Handler_136,
	(pHandler) IRQ_Handler_137,
	(pHandler) IRQ_Handler_138,
	(pHandler) IRQ_Handler_139,

	(pHandler) IRQ_Handler_140,
	(pHandler) IRQ_Handler_141,
	(pHandler) IRQ_Handler_142,
	(pHandler) IRQ_Handler_143,
	(pHandler) IRQ_Handler_144,
	(pHandler) IRQ_Handler_145,
	(pHandler) IRQ_Handler_146,
	(pHandler) IRQ_Handler_147,
	(pHandler) IRQ_Handler_148,
	(pHandler) IRQ_Handler_149,

	(pHandler) IRQ_Handler_150,
	(pHandler) IRQ_Handler_151,
	(pHandler) IRQ_Handler_152,
	(pHandler) IRQ_Handler_153,
	(pHandler) IRQ_Handler_154,
	(pHandler) IRQ_Handler_155,
	(pHandler) IRQ_Handler_156,
	(pHandler) IRQ_Handler_157,
	(pHandler) IRQ_Handler_158,
	(pHandler) IRQ_Handler_159,
#endif
#if MAX_IRQ_HANDLER_COUNT > 160
	(pHandler) IRQ_Handler_160,
	(pHandler) IRQ_Handler_161,
	(pHandler) IRQ_Handler_162,
	(pHandler) IRQ_Handler_163,
	(pHandler) IRQ_Handler_164,
	(pHandler) IRQ_Handler_165,
	(pHandler) IRQ_Handler_166,
	(pHandler) IRQ_Handler_167,
	(pHandler) IRQ_Handler_168,
	(pHandler) IRQ_Handler_169,

	(pHandler) IRQ_Handler_170,
	(pHandler) IRQ_Handler_171,
	(pHandler) IRQ_Handler_172,
	(pHandler) IRQ_Handler_173,
	(pHandler) IRQ_Handler_174,
	(pHandler) IRQ_Handler_175,
	(pHandler) IRQ_Handler_176,
	(pHandler) IRQ_Handler_177,
	(pHandler) IRQ_Handler_178,
	(pHandler) IRQ_Handler_179,

	(pHandler) IRQ_Handler_180,
	(pHandler) IRQ_Handler_181,
	(pHandler) IRQ_Handler_182,
	(pHandler) IRQ_Handler_183,
	(pHandler) IRQ_Handler_184,
	(pHandler) IRQ_Handler_185,
	(pHandler) IRQ_Handler_186,
	(pHandler) IRQ_Handler_187,
	(pHandler) IRQ_Handler_188,
	(pHandler) IRQ_Handler_189,

	(pHandler) IRQ_Handler_190,
	(pHandler) IRQ_Handler_191,
	(pHandler) IRQ_Handler_192,
	(pHandler) IRQ_Handler_193,
	(pHandler) IRQ_Handler_194,
	(pHandler) IRQ_Handler_195,
	(pHandler) IRQ_Handler_196,
	(pHandler) IRQ_Handler_197,
	(pHandler) IRQ_Handler_198,
	(pHandler) IRQ_Handler_199
#endif
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

