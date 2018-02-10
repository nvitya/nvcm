/*
 * @brief LPC18xx/43xx clock driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licenser disclaim any and
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

#ifndef __CLOCK_18XX_43XX_H_
#define __CLOCK_18XX_43XX_H_

#include "cguccu_18xx_43xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CLOCK_18XX_43XX CHIP: LPC18xx/43xx Clock Driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/** @defgroup CLOCK_18XX_43XX_OPTIONS CHIP: LPC18xx/43xx Clock Driver driver options
 * @ingroup CLOCK_18XX_43XX CHIP_18XX_43XX_DRIVER_OPTIONS
 * The clock driver has options that configure it's operation at build-time.<br>
 *
 * <b>MAX_CLOCK_FREQ</b><br>
 * This macro defines the maximum frequency supported by the Chip [204MHz for LPC43xx
 * 180MHz for LPC18xx]. API Chip_SetupXtalClocking() and Chip_SetupIrcClocking() will
 * use this macro to set the CPU Core frequency to the maximum supported.<br>
 * To set a Core frequency other than the maximum frequency Chip_SetupCoreClock() API
 * must be used. <b>Using this macro to set the Core freqency is not recommended.</b>
 * @{
 */

/**
 * @}
 */

/* Internal oscillator frequency */
#define CGU_IRC_FREQ (12000000)

#ifndef MAX_CLOCK_FREQ
#if defined(CHIP_LPC43XX)
#define MAX_CLOCK_FREQ (204000000)
#else
#define MAX_CLOCK_FREQ (180000000)
#endif
#endif

#define PLL_MIN_CCO_FREQ 156000000  /**< Min CCO frequency of main PLL */
#define PLL_MAX_CCO_FREQ 320000000  /**< Max CCO frequency of main PLL */

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_18XX_43XX_H_ */
