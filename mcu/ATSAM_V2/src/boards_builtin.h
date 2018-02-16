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
 *  file:     boards_builtin.h (ATSAM_V2)
 *  brief:    Built-in ATSAM_V2 board definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// ATMEL v2
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_MIBO64_ATSAME5X)

  #define BOARD_NAME "MIBO-64 ATSAME5X by nvitya"
  #define MCU_ATSAMD51J20
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
