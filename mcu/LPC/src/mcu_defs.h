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
 *  file:     mcu_defs.h (LPC)
 *  brief:    LPC MCU Family definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_43XX)

  #define MAX_CLOCK_SPEED  204000000

#endif

#define HW_DMA_MAX_COUNT  2048

#define INDEPENDENT_GPIO
#define HW_GPIO_REGS  LPC_GPIO_T

#define HW_UART_REGS  LPC_USART_T
#define HW_SPI_REGS   LPC_SSP_T
#define HW_DMA_REGS 	GPDMA_CH_T

// The register definition of the SPIFI is missing from the official LPC43xx header...

typedef struct SPIFI_REGS_T
{
	volatile    uint32_t CTRL;				/**< SPIFI control register */
	volatile    uint32_t CMD;					/**< SPIFI command register */
	volatile    uint32_t ADDR;				/**< SPIFI address register */
	volatile    uint32_t IDATA;				/**< SPIFI intermediate data register */
	volatile    uint32_t CLIMIT;			/**< SPIFI cache limit register */
	union
	{
		volatile    uint8_t  DATA8;			/**< SPIFI 8 bit data */
		volatile    uint16_t DATA16;		/**< SPIFI 16 bit data */
		volatile    uint32_t DATA;			/**< SPIFI 32 bit data */
	};
	volatile    uint32_t MCMD;				/**< SPIFI memory command register */
	volatile    uint32_t STAT;				/**< SPIFI status register */
//
} SPIFI_REGS_T;

#define HW_QSPI_REGS  SPIFI_REGS_T

inline void __attribute__((always_inline)) mcu_preinit_code()
{
}

#endif // __MCU_DEFS_H
