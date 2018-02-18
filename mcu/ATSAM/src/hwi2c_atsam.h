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
 *  file:     hwi2c_atsam.h
 *  brief:    ATSAM I2C / TWI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWI2C_ATSAM_H_
#define HWI2C_ATSAM_H_

#define HWI2C_PRE_ONLY
#include "hwi2c.h"

// re-definition because of the Field name differences at TWI and TWIHS

typedef struct
{
  __O  uint32_t CR;        /**< \brief (Twi Offset: 0x00) Control Register */
  __IO uint32_t MMR;       /**< \brief (Twi Offset: 0x04) Master Mode Register */
  __IO uint32_t SMR;       /**< \brief (Twi Offset: 0x08) Slave Mode Register */
  __IO uint32_t IADR;      /**< \brief (Twi Offset: 0x0C) Internal Address Register */
  __IO uint32_t CWGR;      /**< \brief (Twi Offset: 0x10) Clock Waveform Generator Register */
  __I  uint32_t   _reserved1[3];
  __I  uint32_t SR;        /**< \brief (Twi Offset: 0x20) Status Register */
  __O  uint32_t IER;       /**< \brief (Twi Offset: 0x24) Interrupt Enable Register */
  __O  uint32_t IDR;       /**< \brief (Twi Offset: 0x28) Interrupt Disable Register */
  __I  uint32_t IMR;       /**< \brief (Twi Offset: 0x2C) Interrupt Mask Register */
  __I  uint32_t RHR;       /**< \brief (Twi Offset: 0x30) Receive Holding Register */
  __O  uint32_t THR;       /**< \brief (Twi Offset: 0x34) Transmit Holding Register */
//
} THwI2cRegs_atsam;

#define HW_I2C_REGS  THwI2cRegs_atsam

class THwI2c_atsam : public THwI2c_pre
{
public:
	HW_I2C_REGS * regs = nullptr;

	bool Init(int adevnum);

	int  StartReadData(uint8_t  adaddr, unsigned aextra, void * dstptr, unsigned len);
	int  StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len);
	void Run();

	unsigned       runstate = 0;
};

#define HWI2C_IMPL THwI2c_atsam

#endif // def HWI2C_ATSAM_H_
