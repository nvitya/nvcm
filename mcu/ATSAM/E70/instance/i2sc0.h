/**
 * \file
 *
 * Copyright (c) 2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef _SAME70_I2SC0_INSTANCE_
#define _SAME70_I2SC0_INSTANCE_

/* ========== Register definition for I2SC0 peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define REG_I2SC0_CR                       (0x4008C000U) /**< \brief (I2SC0) Control Register */
  #define REG_I2SC0_MR                       (0x4008C004U) /**< \brief (I2SC0) Mode Register */
  #define REG_I2SC0_SR                       (0x4008C008U) /**< \brief (I2SC0) Status Register */
  #define REG_I2SC0_SCR                      (0x4008C00CU) /**< \brief (I2SC0) Status Clear Register */
  #define REG_I2SC0_SSR                      (0x4008C010U) /**< \brief (I2SC0) Status Set Register */
  #define REG_I2SC0_IER                      (0x4008C014U) /**< \brief (I2SC0) Interrupt Enable Register */
  #define REG_I2SC0_IDR                      (0x4008C018U) /**< \brief (I2SC0) Interrupt Disable Register */
  #define REG_I2SC0_IMR                      (0x4008C01CU) /**< \brief (I2SC0) Interrupt Mask Register */
  #define REG_I2SC0_RHR                      (0x4008C020U) /**< \brief (I2SC0) Receiver Holding Register */
  #define REG_I2SC0_THR                      (0x4008C024U) /**< \brief (I2SC0) Transmitter Holding Register */
  #define REG_I2SC0_VERSION                  (0x4008C028U) /**< \brief (I2SC0) Version Register */
#else
  #define REG_I2SC0_CR      (*(__O  uint32_t*)0x4008C000U) /**< \brief (I2SC0) Control Register */
  #define REG_I2SC0_MR      (*(__IO uint32_t*)0x4008C004U) /**< \brief (I2SC0) Mode Register */
  #define REG_I2SC0_SR      (*(__I  uint32_t*)0x4008C008U) /**< \brief (I2SC0) Status Register */
  #define REG_I2SC0_SCR     (*(__O  uint32_t*)0x4008C00CU) /**< \brief (I2SC0) Status Clear Register */
  #define REG_I2SC0_SSR     (*(__O  uint32_t*)0x4008C010U) /**< \brief (I2SC0) Status Set Register */
  #define REG_I2SC0_IER     (*(__O  uint32_t*)0x4008C014U) /**< \brief (I2SC0) Interrupt Enable Register */
  #define REG_I2SC0_IDR     (*(__O  uint32_t*)0x4008C018U) /**< \brief (I2SC0) Interrupt Disable Register */
  #define REG_I2SC0_IMR     (*(__I  uint32_t*)0x4008C01CU) /**< \brief (I2SC0) Interrupt Mask Register */
  #define REG_I2SC0_RHR     (*(__I  uint32_t*)0x4008C020U) /**< \brief (I2SC0) Receiver Holding Register */
  #define REG_I2SC0_THR     (*(__O  uint32_t*)0x4008C024U) /**< \brief (I2SC0) Transmitter Holding Register */
  #define REG_I2SC0_VERSION (*(__I  uint32_t*)0x4008C028U) /**< \brief (I2SC0) Version Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _SAME70_I2SC0_INSTANCE_ */
