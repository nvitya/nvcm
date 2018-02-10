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

#ifndef __I2CM_18XX_43XX_H_
#define __I2CM_18XX_43XX_H_

#include "i2c_common_18xx_43xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup I2CM_18XX_43XX CHIP: LPC18xx/43xx I2C master-only driver
 * @ingroup I2C_18XX_43XX
 * This driver only works in master mode. To describe the I2C transactions
 * following symbols are used in driver documentation.
 *
 * Key to symbols
 * ==============
 * S     (1 bit) : Start bit
 * P     (1 bit) : Stop bit
 * Rd/Wr (1 bit) : Read/Write bit. Rd equals 1, Wr equals 0.
 * A, NA (1 bit) : Acknowledge and Not-Acknowledge bit.
 * Addr  (7 bits): I2C 7 bit address. Note that this can be expanded as usual to
 *                 get a 10 bit I2C address.
 * Data  (8 bits): A plain data byte. Sometimes, I write DataLow, DataHigh
 *                 for 16 bit data.
 * [..]: Data sent by I2C device, as opposed to data sent by the host adapter.
 * @{
 */

/** I2CM_18XX_43XX_OPTIONS_TYPES I2C master transfer options
 * @{
 */

/** Ignore NACK during data transfer. By default transfer is aborted. */
#define I2CM_XFER_OPTION_IGNORE_NACK     0x01
/** ACK last byte received. By default we NACK last byte we receive per I2C spec. */
#define I2CM_XFER_OPTION_LAST_RX_ACK     0x02

/**
 * @}
 */

/** I2CM_18XX_43XX_STATUS_TYPES I2C master transfer status types
 * @{
 */

#define I2CM_STATUS_OK              0x00		/*!< Requested Request was executed successfully. */
#define I2CM_STATUS_ERROR           0x01		/*!< Unknown error condition. */
#define I2CM_STATUS_NAK             0x02		/*!< No acknowledgement received from slave. */
#define I2CM_STATUS_BUS_ERROR       0x03		/*!< I2C bus error */
#define I2CM_STATUS_SLAVE_NAK       0x04		/*!< No device responded for given slave address during SLA+W or SLA+R */
#define I2CM_STATUS_ARBLOST         0x05		/*!< Arbitration lost. */
#define I2CM_STATUS_BUSY            0xFF		/*!< I2C transmitter is busy. */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_18XX_43XX_H_ */
