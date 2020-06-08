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
 *  file:     i2c_eeprom.h
 *  brief:    I2C EEPROM (24LC16B) Implementation
 *  version:  1.00
 *  date:     2019-03-24
 *  authors:  nvitya
*/

#ifndef I2C_EEPROM_H_
#define I2C_EEPROM_H_

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"
#include "hwdma.h"
#include "errors.h"

#define I2C_STATE_READMEM   1
#define I2C_STATE_WRITEMEM  2

class TI2cEeprom
{
public:

	bool 					 initialized = false;
	bool           completed = true;
	int            errorcode = 0;

	THwI2c *       pi2c = nullptr;
	uint8_t        devaddr = 0x50;
	unsigned       bytesize = 0;

	bool           Init(THwI2c * ai2c, uint8_t aaddr, uint32_t abytesize);

	bool 					 StartReadMem(unsigned aaddr, void * adstptr, unsigned alen);
	bool 					 StartWriteMem(unsigned aaddr, void * asrcptr, unsigned alen);

	void           Run();
	int            WaitComplete();

protected:

	unsigned char  txbuf[16];
	unsigned char  rxbuf[16];

	// state machine
	int            state = 0;
	int            phase = 0;

	unsigned       chunksize = 0;
	unsigned       maxchunksize = HW_DMA_MAX_COUNT;

	uint8_t *      dataptr = nullptr;
	unsigned       datalen = 0;
	unsigned       address = 0;
	unsigned       remaining = 0;
};

#endif /* I2C_EEPROM_H_ */
