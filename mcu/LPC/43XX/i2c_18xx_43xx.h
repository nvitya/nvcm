/*
 * @brief LPC18xx/43xx I2C driver
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

#ifndef __I2C_18XX_43XX_H_
#define __I2C_18XX_43XX_H_
#include "i2c_common_18xx_43xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	Return values for SLAVE handler
 * @note
 * Chip drivers will usally be designed to match their events with this value
 */
#define RET_SLAVE_TX    6	/**< Return value, when 1 byte TX'd successfully */
#define RET_SLAVE_RX    5	/**< Return value, when 1 byte RX'd successfully */
#define RET_SLAVE_IDLE  2	/**< Return value, when slave enter idle mode */
#define RET_SLAVE_BUSY  0	/**< Return value, when slave is busy */

/**
 * @brief I2C state handle return values
 */
#define I2C_STA_STO_RECV            0x20

/*
 * @brief I2C return status code definitions
 */
#define I2C_I2STAT_NO_INF                       ((0xF8))/*!< No relevant information */
#define I2C_I2STAT_BUS_ERROR                    ((0x00))/*!< Bus Error */

/*
 * @brief I2C status values
 */
#define I2C_SETUP_STATUS_ARBF   (1 << 8)	/**< Arbitration false */
#define I2C_SETUP_STATUS_NOACKF (1 << 9)	/**< No ACK returned */
#define I2C_SETUP_STATUS_DONE   (1 << 10)	/**< Status DONE */

/*
 * @brief I2C state handle return values
 */
#define I2C_OK                      0x00
#define I2C_BYTE_SENT               0x01
#define I2C_BYTE_RECV               0x02
#define I2C_LAST_BYTE_RECV          0x04
#define I2C_SEND_END                0x08
#define I2C_RECV_END                0x10
#define I2C_STA_STO_RECV            0x20

#define I2C_ERR                     (0x10000000)
#define I2C_NAK_RECV                (0x10000000 | 0x01)

#define I2C_CheckError(ErrorCode)   (ErrorCode & 0x10000000)

/*
 * @brief I2C monitor control configuration defines
 */
#define I2C_MONITOR_CFG_SCL_OUTPUT  I2C_I2MMCTRL_ENA_SCL		/**< SCL output enable */
#define I2C_MONITOR_CFG_MATCHALL    I2C_I2MMCTRL_MATCH_ALL		/**< Select interrupt register match */

/**
 * @brief	I2C Slave Identifiers
 */
typedef enum {
	I2C_SLAVE_GENERAL,	/**< Slave ID for general calls */
	I2C_SLAVE_0,		/**< Slave ID fo Slave Address 0 */
	I2C_SLAVE_1,		/**< Slave ID fo Slave Address 1 */
	I2C_SLAVE_2,		/**< Slave ID fo Slave Address 2 */
	I2C_SLAVE_3,		/**< Slave ID fo Slave Address 3 */
	I2C_SLAVE_NUM_INTERFACE	/**< Number of slave interfaces */
} I2C_SLAVE_ID;

/**
 * @brief	I2C transfer status
 */
typedef enum {
	I2C_STATUS_DONE,	/**< Transfer done successfully */
	I2C_STATUS_NAK,		/**< NAK received during transfer */
	I2C_STATUS_ARBLOST,	/**< Aribitration lost during transfer */
	I2C_STATUS_BUSERR,	/**< Bus error in I2C transfer */
	I2C_STATUS_BUSY,	/**< I2C is busy doing transfer */
	I2C_STATUS_SLAVENAK,/**< NAK received after SLA+W or SLA+R */
} I2C_STATUS_T;

/**
 * @brief	I2C interface IDs
 * @note
 * All Chip functions will take this as the first parameter,
 * I2C_NUM_INTERFACE must never be used for calling any Chip
 * functions, it is only used to find the number of interfaces
 * available in the Chip.
 */
typedef enum I2C_ID {
	I2C0,				/**< ID I2C0 */
	I2C1,				/**< ID I2C1 */
	I2C_NUM_INTERFACE	/**< Number of I2C interfaces in the chip */
} I2C_ID_T;

/**
 * @brief	I2C master events
 */
typedef enum {
	I2C_EVENT_WAIT = 1,	/**< I2C Wait event */
	I2C_EVENT_DONE,		/**< Done event that wakes up Wait event */
	I2C_EVENT_LOCK,		/**< Re-entrency lock event for I2C transfer */
	I2C_EVENT_UNLOCK,	/**< Re-entrency unlock event for I2C transfer */
	I2C_EVENT_SLAVE_RX,	/**< Slave receive event */
	I2C_EVENT_SLAVE_TX,	/**< Slave transmit event */
} I2C_EVENT_T;

#ifdef __cplusplus
}
#endif

#endif /* __I2C_18XX_43XX_H_ */
