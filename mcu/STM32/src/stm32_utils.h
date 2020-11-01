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
 *  file:     stm32_utils.h
 *  brief:    STM32 Utilities
 *  version:  1.00
 *  date:     2019-11-22
 *  authors:  nvitya
*/

#ifndef STM32_UTILS_H_
#define STM32_UTILS_H_

#include "platform.h"

#if defined(MCUSF_G4)
  #define APB1ENR_REGISTER  RCC->APB1ENR1
#elif defined(MCUSF_H7)
  #define APB1ENR_REGISTER  RCC->APB1LENR
#else
  #define APB1ENR_REGISTER  RCC->APB1ENR
#endif

// constants helping determine peripheral bus base frequencies
#define STM32_BUSID_AHB     0
#define STM32_BUSID_APB1    1
#define STM32_BUSID_APB2    2

uint32_t stm32_bus_speed(uint8_t abusid);

#endif /* STM32_UTILS_H_ */
