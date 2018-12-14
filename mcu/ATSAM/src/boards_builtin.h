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
 *  file:     boards_builtin.h (ATSAM)
 *  brief:    Built-in ATSAM board definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// ATMEL
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_ARDUINO_DUE)

  #define BOARD_NAME "Arduino DUE"
  #define MCU_ATSAM3X8E
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_XPLAINED_SAME70)

  #define BOARD_NAME "SAME70 XPlained"
  #define MCU_ATSAME70Q21
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_MIBO100_ATSAME70)

  #define BOARD_NAME "MIBO-100 ATSAME70N by nvitya"
	#define MCU_ATSAME70N20
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_VERTIBO_A)

  #define BOARD_NAME "VERTIBO-A by nvitya"
	#define MCU_ATSAME70Q20
  #define MCU_INPUT_FREQ    12000000
  #define MCU_CLOCK_SPEED  288000000  // because the SDRAM shares the data bus with the FPGA

#elif defined(BOARD_ENEBO_A)

  #define BOARD_NAME "ENEBO-A by nvitya"
	#define MCU_ATSAME70N20
  #define MCU_INPUT_FREQ    12000000

#elif defined(BOARD_MIBO64_ATSAM4S)

  #define BOARD_NAME "MIBO-64 ATSAM4S by nvitya"
  #define MCU_ATSAM4S2B
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
