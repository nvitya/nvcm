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
 *  file:     mcu_impl.h (STM32)
 *  brief:    STM32 list of implemented NVCM core peripherals
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifdef HWCLKCTRL_H_
  #include "hwclkctrl_stm32.h"
#endif

#ifdef HWPINS_H_
  #include "hwpins_stm32.h"
#endif

#ifdef HWUART_H_
  #include "hwuart_stm32.h"
#endif

#ifdef HWSPI_H_
  #include "hwspi_stm32.h"
#endif

#ifdef HWI2C_H_
  #include "hwi2c_stm32.h"
#endif

#ifdef HWDMA_H_
  #include "hwdma_stm32.h"
#endif

#ifdef HWADC_H_
  #include "hwadc_stm32.h"
#endif

#ifdef HWUSBCTRL_H_
  #include "hwusbctrl_stm32.h"
#endif

#ifdef HWETH_H_
  #include "hweth_stm32.h"
#endif

#if defined(QUADSPI) && defined(HWQSPI_H_)
  #include "hwqspi_stm32.h"
#endif

#if defined(FMC_SDCR1_CAS) && defined(HWSDRAM_H_)
  #include "hwsdram_stm32.h"
#endif
