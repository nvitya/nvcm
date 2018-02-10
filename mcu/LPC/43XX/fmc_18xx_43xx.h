/*
 * @brief LPC18xx/43xx FLASH Memory Controller (FMC) driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

#ifndef __FMC_18XX_43XX_H_
#define __FMC_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup FMC_18XX_43XX CHIP: LPC18xx/43xx FLASH Memory Controller driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief FLASH Memory Controller Unit register block structure
 */
typedef struct {		/*!< FMC Structure */
	__I  uint32_t  RESERVED1[8];
	__IO uint32_t  FMSSTART;
	__IO uint32_t  FMSSTOP;
	__I  uint32_t  RESERVED2;
	__I  uint32_t  FMSW[4];
	__I  uint32_t  RESERVED3[1001];
	__I  uint32_t  FMSTAT;
	__I  uint32_t  RESERVED5;
	__O  uint32_t  FMSTATCLR;
	__I  uint32_t  RESERVED4[5];
} LPC_FMC_T;

/* Flash signature start and busy status bit */
#define FMC_FLASHSIG_BUSY       (1UL << 17)

/* Flash signature clear status bit */
#define FMC_FLASHSIG_STAT       (1 << 2)

#ifdef __cplusplus
}
#endif

#endif /* __FMC_18XX_43XX_H_ */
