/*
 * @brief LPC18xx/43xx EEPROM driver
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

#ifndef _EEPROM_18XX_43XX_H_
#define _EEPROM_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup EEPROM_18XX_43XX CHIP: LPC18xx/43xx EEPROM driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/* FIX ME: Move to chip.h */
/** EEPROM start address */
#define EEPROM_START                    (0x20040000)
/** EEPROM byes per page */
#define EEPROM_PAGE_SIZE                (128)
/**The number of EEPROM pages. The last page is not writable.*/
#define EEPROM_PAGE_NUM                 (128)
/** Get the eeprom address */
#define EEPROM_ADDRESS(page, offset)     (EEPROM_START + (EEPROM_PAGE_SIZE * (page)) + offset)
#define EEPROM_CLOCK_DIV                 1500000
#define EEPROM_READ_WAIT_STATE_VAL       0x58
#define EEPROM_WAIT_STATE_VAL            0x232

/**
 * @brief EEPROM register block structure
 */
typedef struct {				/* EEPROM Structure */
	__IO uint32_t CMD;			/*!< EEPROM command register */
	uint32_t RESERVED0;
	__IO uint32_t RWSTATE;		/*!< EEPROM read wait state register */
	__IO uint32_t AUTOPROG;		/*!< EEPROM auto programming register */
	__IO uint32_t WSTATE;		/*!< EEPROM wait state register */
	__IO uint32_t CLKDIV;		/*!< EEPROM clock divider register */
	__IO uint32_t PWRDWN;		/*!< EEPROM power-down register */
	uint32_t RESERVED2[1007];
	__O  uint32_t INTENCLR;		/*!< EEPROM interrupt enable clear */
	__O  uint32_t INTENSET;		/*!< EEPROM interrupt enable set */
	__I  uint32_t INTSTAT;		/*!< EEPROM interrupt status */
	__I  uint32_t INTEN;		/*!< EEPROM interrupt enable */
	__O  uint32_t INTSTATCLR;	/*!< EEPROM interrupt status clear */
	__O  uint32_t INTSTATSET;	/*!< EEPROM interrupt status set */
} LPC_EEPROM_T;

/*
 * @brief Macro defines for EEPROM command register
 */
#define EEPROM_CMD_ERASE_PRG_PAGE       (6)		/*!< EEPROM erase/program command */

/*
 * @brief Macro defines for EEPROM Auto Programming register
 */
#define EEPROM_AUTOPROG_OFF     (0)		/*!<Auto programming off */
#define EEPROM_AUTOPROG_AFT_1WORDWRITTEN     (1)		/*!< Erase/program cycle is triggered after 1 word is written */
#define EEPROM_AUTOPROG_AFT_LASTWORDWRITTEN  (2)		/*!< Erase/program cycle is triggered after a write to AHB
														   address ending with ......1111100 (last word of a page) */

/*
 * @brief Macro defines for EEPROM power down register
 */
#define EEPROM_PWRDWN                   (1 << 0)

/*
 * @brief Macro defines for EEPROM interrupt related registers
 */
#define EEPROM_INT_ENDOFPROG            (1 << 2)

#ifdef __cplusplus
}
#endif

#endif /* _EEPROM_18XX_43XX_H_ */
