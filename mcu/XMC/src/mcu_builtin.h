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
 *  file:     mcu_builtin.h (XMC)
 *  brief:    Built-in XMC MCU definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

#if 0

//----------------------------------------------------------------------
// Infineon
//----------------------------------------------------------------------

#elif defined(MCU_XMC1404F64X0064)

	#define MCUF_XMC
  #define MCUSF_1000

  #define XMC1404_F064x0064

  #include "xmc_device.h"

#elif defined(MCU_XMC1404Q48X0064)

	#define MCUF_XMC
  #define MCUSF_1000

  #define XMC1404_Q048x0064

  #include "xmc_device.h"

#elif defined(MCU_XMC1200T38X0200)

	#define MCUF_XMC
  #define MCUSF_1000

  #define XMC1200_T038x0200
  #include "xmc_device.h"

#elif defined(MCU_XMC1100Q40X0032)

	#define MCUF_XMC
  #define MCUSF_1000

  #define XMC1100_Q040x0032
  #include "xmc_device.h"

// XMC4000

#elif defined(MCU_XMC4108Q48X0064)

  #define MCUF_XMC
  #define MCUSF_4000

  #define XMC4108_Q48x64
  #include "xmc_device.h"

#elif defined(MCU_XMC4300F100X256)

  #define MCUF_XMC
  #define MCUSF_4000

  #define XMC4300_F100x256
  #include "xmc_device.h"

#else

  #error "Unknown MCU"

#endif

#endif
