/*
 * @brief LPC18XX/43XX CREG control functions
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __CREG_18XX_43XX_H_
#define __CREG_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CREG_18XX_43XX CHIP: LPC18xx/43xx CREG driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief CREG Register Block
 */
typedef struct {						/*!< CREG Structure         */
	__I  uint32_t  RESERVED0;
	__IO uint32_t  CREG0;				/*!< Chip configuration register 32 kHz oscillator output and BOD control register. */
	__I  uint32_t  RESERVED1[62];
	__IO uint32_t  MXMEMMAP;			/*!< ARM Cortex-M3/M4 memory mapping */
#if defined(CHIP_LPC18XX)
	__I  uint32_t  RESERVED2[5];
#else
	__I  uint32_t  RESERVED2;
	__I  uint32_t  CREG1;				/*!< Configuration Register 1 */
	__I  uint32_t  CREG2;				/*!< Configuration Register 2 */
	__I  uint32_t  CREG3;				/*!< Configuration Register 3 */
	__I  uint32_t  CREG4;				/*!< Configuration Register 4 */
#endif
	__IO uint32_t  CREG5;				/*!< Chip configuration register 5. Controls JTAG access. */
	__IO uint32_t  DMAMUX;				/*!< DMA muxing control     */
	__IO uint32_t  FLASHCFGA;			/*!< Flash accelerator configuration register for flash bank A */
	__IO uint32_t  FLASHCFGB;			/*!< Flash accelerator configuration register for flash bank B */
	__IO uint32_t  ETBCFG;				/*!< ETB RAM configuration  */
	__IO uint32_t  CREG6;				/*!< Chip configuration register 6. */
#if defined(CHIP_LPC18XX)
	__I  uint32_t  RESERVED4[52];
#else
	__IO uint32_t  M4TXEVENT;			/*!< M4 IPC event register */
	__I  uint32_t  RESERVED4[51];
#endif
	__I  uint32_t  CHIPID;				/*!< Part ID                */
#if defined(CHIP_LPC18XX)
	__I  uint32_t  RESERVED5[191];
#else
	__I  uint32_t  RESERVED5[65];
	__IO uint32_t  M0SUBMEMMAP;         /*!< M0SUB IPC Event memory mapping */
	__I  uint32_t  RESERVED6[2];
	__IO uint32_t  M0SUBTXEVENT;        /*!< M0SUB IPC Event register */
	__I  uint32_t  RESERVED7[58];
	__IO uint32_t  M0APPTXEVENT;		/*!< M0APP IPC Event register */
	__IO uint32_t  M0APPMEMMAP;			/*!< ARM Cortex M0APP memory mapping */
	__I  uint32_t  RESERVED8[62];
#endif
	__IO uint32_t  USB0FLADJ;			/*!< USB0 frame length adjust register */
	__I  uint32_t  RESERVED9[63];
	__IO uint32_t  USB1FLADJ;			/*!< USB1 frame length adjust register */
} LPC_CREG_T;


#ifdef __cplusplus
}
#endif

#endif /* __CREG_18XX_43XX_H_ */
