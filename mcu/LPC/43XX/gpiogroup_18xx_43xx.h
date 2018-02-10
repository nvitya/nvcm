/*
 * @brief LPC18xx/43xx GPIO group driver
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

#ifndef __GPIOGROUP_18XX_43XX_H_
#define __GPIOGROUP_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup GPIOGP_18XX_43XX CHIP: LPC18xx/43xx GPIO group driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief GPIO grouped interrupt register block structure
 */
typedef struct {					/*!< GPIO_GROUP_INTn Structure */
	__IO uint32_t  CTRL;			/*!< GPIO grouped interrupt control register */
	__I  uint32_t  RESERVED0[7];
	__IO uint32_t  PORT_POL[8];		/*!< GPIO grouped interrupt port polarity register */
	__IO uint32_t  PORT_ENA[8];		/*!< GPIO grouped interrupt port m enable register */
	uint32_t       RESERVED1[1000];
} LPC_GPIOGROUPINT_T;

/**
 * LPC18xx/43xx GPIO group bit definitions
 */
#define GPIOGR_INT      (1 << 0)	/*!< GPIO interrupt pending/clear bit */
#define GPIOGR_COMB     (1 << 1)	/*!< GPIO interrupt OR(0)/AND(1) mode bit */
#define GPIOGR_TRIG     (1 << 2)	/*!< GPIO interrupt edge(0)/level(1) mode bit */

#ifdef __cplusplus
}
#endif

#endif /* __GPIOGROUP_18XX_43XX_H_ */
