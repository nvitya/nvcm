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
 *  file:     tftlcd_spi.h
 *  brief:    SPI TFT LCD Display driver (SPI hw required)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef TFTLCD_SPI_H_
#define TFTLCD_SPI_H_

#include "tftlcd.h"
#include "hwpins.h"
#include "hwspi.h"
#include "hwdma.h"

#define TFTLCD_SPI_DMA_BUFSIZE  256

class TTftLcd_spi: public TTftLcd
{
public:
	typedef TTftLcd super;

	// required hw resources:
	THwSpi     spi;
	TGpioPin   pin_cs;
	TGpioPin   pin_cd;
	TGpioPin 	 pin_reset;  // unassigned

	THwDmaChannel   txdma;
	THwDmaTransfer  dmaxfer;

	uint8_t cmdbuf[4];
	uint8_t databuf[TFTLCD_SPI_DMA_BUFSIZE];

public:
	// interface dependent

	virtual bool InitInterface();
	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);


};

#endif /* TFTLCD_SPI_H_ */
