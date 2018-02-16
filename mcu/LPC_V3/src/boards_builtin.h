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
 *  file:     boards_builtin.h (LPC_V3)
 *  brief:    Built-in LPC_V3 board definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// NXP
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_MIBO100_LPC546XX)

  #define BOARD_NAME "MIBO-100 LPC546xx by nvitya"
  #define MCU_LPC54606
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_XPRESSO_LPC54608)

  #define BOARD_NAME "LPCXpresso54608"
  #define MCU_LPC54608
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
