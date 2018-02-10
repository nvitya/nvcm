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

#ifndef _SAME70_I2SC1_INSTANCE_
#define _SAME70_I2SC1_INSTANCE_

/* ========== Register definition for I2SC1 peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define REG_I2SC1_CR                       (0x40090000U) /**< \brief (I2SC1) Control Register */
  #define REG_I2SC1_MR                       (0x40090004U) /**< \brief (I2SC1) Mode Register */
  #define REG_I2SC1_SR                       (0x40090008U) /**< \brief (I2SC1) Status Register */
  #define REG_I2SC1_SCR                      (0x4009000CU) /**< \brief (I2SC1) Status Clear Register */
  #define REG_I2SC1_SSR                      (0x40090010U) /**< \brief (I2SC1) Status Set Register */
  #define REG_I2SC1_IER                      (0x40090014U) /**< \brief (I2SC1) Interrupt Enable Register */
  #define REG_I2SC1_IDR                      (0x40090018U) /**< \brief (I2SC1) Interrupt Disable Register */
  #define REG_I2SC1_IMR                      (0x4009001CU) /**< \brief (I2SC1) Interrupt Mask Register */
  #define REG_I2SC1_RHR                      (0x40090020U) /**< \brief (I2SC1) Receiver Holding Register */
  #define REG_I2SC1_THR                      (0x40090024U) /**< \brief (I2SC1) Transmitter Holding Register */
  #define REG_I2SC1_VERSION                  (0x40090028U) /**< \brief (I2SC1) Version Register */
#else
  #define REG_I2SC1_CR      (*(__O  uint32_t*)0x40090000U) /**< \brief (I2SC1) Control Register */
  #define REG_I2SC1_MR      (*(__IO uint32_t*)0x40090004U) /**< \brief (I2SC1) Mode Register */
  #define REG_I2SC1_SR      (*(__I  uint32_t*)0x40090008U) /**< \brief (I2SC1) Status Register */
  #define REG_I2SC1_SCR     (*(__O  uint32_t*)0x4009000CU) /**< \brief (I2SC1) Status Clear Register */
  #define REG_I2SC1_SSR     (*(__O  uint32_t*)0x40090010U) /**< \brief (I2SC1) Status Set Register */
  #define REG_I2SC1_IER     (*(__O  uint32_t*)0x40090014U) /**< \brief (I2SC1) Interrupt Enable Register */
  #define REG_I2SC1_IDR     (*(__O  uint32_t*)0x40090018U) /**< \brief (I2SC1) Interrupt Disable Register */
  #define REG_I2SC1_IMR     (*(__I  uint32_t*)0x4009001CU) /**< \brief (I2SC1) Interrupt Mask Register */
  #define REG_I2SC1_RHR     (*(__I  uint32_t*)0x40090020U) /**< \brief (I2SC1) Receiver Holding Register */
  #define REG_I2SC1_THR     (*(__O  uint32_t*)0x40090024U) /**< \brief (I2SC1) Transmitter Holding Register */
  #define REG_I2SC1_VERSION (*(__I  uint32_t*)0x40090028U) /**< \brief (I2SC1) Version Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _SAME70_I2SC1_INSTANCE_ */
