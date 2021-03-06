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
 *  file:     boards_builtin.h (XMC)
 *  brief:    Built-in XMC board definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// Infineon
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_BOOT_XMC1200)

  #define BOARD_NAME "XMC1200 Boot Kit"
  #define MCU_XMC1200T38X0200

#elif defined(BOARD_RELAX_XMC4300)

  #define BOARD_NAME "XMC4300 Relax Kit"
  #define MCU_XMC4300F100X256
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_MIBO48_XMC4100)

  #define BOARD_NAME "XMC4100 QFN48 Minimal Board"
  #define MCU_XMC4108Q48X0064
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
