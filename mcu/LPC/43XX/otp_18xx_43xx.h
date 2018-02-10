/*
 * @brief LPC18xx/43xx OTP Controller driver
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

#ifndef __OTP_18XX_43XX_H_
#define __OTP_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup OTP_18XX_43XX CHIP: LPC18xx/43xx OTP Controller driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief	OTP Register block
 */
typedef struct {
	__IO uint32_t OTP0_0;				/*!< (@ 0x40045000) OTP content */
	__IO uint32_t OTP0_1;				/*!< (@ 0x40045004) OTP content */
	__IO uint32_t OTP0_2;				/*!< (@ 0x40045008) OTP content */
	__IO uint32_t OTP0_3;				/*!< (@ 0x4004500C) OTP content */
	__IO uint32_t OTP1_0;				/*!< (@ 0x40045010) OTP content */
	__IO uint32_t OTP1_1;				/*!< (@ 0x40045014) OTP content */
	__IO uint32_t OTP1_2;				/*!< (@ 0x40045018) OTP content */
	__IO uint32_t OTP1_3;				/*!< (@ 0x4004501C) OTP content */
	__IO uint32_t OTP2_0;				/*!< (@ 0x40045020) OTP content */
	__IO uint32_t OTP2_1;				/*!< (@ 0x40045024) OTP content */
	__IO uint32_t OTP2_2;				/*!< (@ 0x40045028) OTP content */
	__IO uint32_t OTP2_3;				/*!< (@ 0x4004502C) OTP content */
	__IO uint32_t OTP3_0;				/*!< (@ 0x40045030) OTP content */
	__IO uint32_t OTP3_1;				/*!< (@ 0x40045034) OTP content */
	__IO uint32_t OTP3_2;				/*!< (@ 0x40045038) OTP content */
	__IO uint32_t OTP3_3;				/*!< (@ 0x4004503C) OTP content */
} LPC_OTP_T;

/**
 * @brief	OTP Boot Source selection used in Chip driver
 */
typedef enum CHIP_OTP_BOOT_SRC {
	CHIP_OTP_BOOTSRC_PINS,		/*!< Boot source - External pins */
	CHIP_OTP_BOOTSRC_UART0,		/*!< Boot source - UART0 */
	CHIP_OTP_BOOTSRC_SPIFI,		/*!< Boot source - EMC 8-bit memory */
	CHIP_OTP_BOOTSRC_EMC8,		/*!< Boot source - EMC 16-bit memory */
	CHIP_OTP_BOOTSRC_EMC16,		/*!< Boot source - EMC 32-bit memory */
	CHIP_OTP_BOOTSRC_EMC32,		/*!< Boot source - EMC 32-bit memory */
	CHIP_OTP_BOOTSRC_USB0,		/*!< Boot source - DFU USB0 boot */
	CHIP_OTP_BOOTSRC_USB1,		/*!< Boot source - DFU USB1 boot */
	CHIP_OTP_BOOTSRC_SPI,		/*!< Boot source - SPI boot */
	CHIP_OTP_BOOTSRC_UART3		/*!< Boot source - UART3 */
} CHIP_OTP_BOOT_SRC_T;

#ifdef __cplusplus
}
#endif

#endif /* __OTP_18XX_43XX_H_ */
