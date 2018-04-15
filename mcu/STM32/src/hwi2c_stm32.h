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
 *  file:     hwi2c_stm32.h
 *  brief:    STM32 I2C / TWI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWI2C_STM32_H_
#define HWI2C_STM32_H_

#define HWI2C_PRE_ONLY
#include "hwi2c.h"

#define HW_I2C_REGS  I2C_TypeDef

#ifdef I2C_CR2_NBYTES
  #define I2C_HW_VER 2  // advanced HW on F0, L0, F7
#else
  #define I2C_HW_VER 1  // old HW on F1, F4
#endif
class THwI2c_stm32 : public THwI2c_pre
{
public:
	HW_I2C_REGS * regs = nullptr;

	bool Init(int adevnum);

	int  StartReadData(uint8_t  adaddr, unsigned aextra, void * dstptr, unsigned len);
	int  StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len);
	void Run();

	unsigned       runstate = 0;
	uint8_t        devaddr = 0;
	uint8_t        extradata[4];
	unsigned       extracnt = 0;
	unsigned       extraremaining = 0;
	bool           waitreload = false;
};

#define HWI2C_IMPL THwI2c_stm32

#endif // def HWI2C_STM32_H_
