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
 *  file:     tftlcd_spi.cpp
 *  brief:    SPI TFT LCD Display driver (SPI hw required)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "clockcnt.h"
#include "tftlcd_spi.h"

bool TTftLcd_spi::InitInterface()
{
	// should be overridden !
	return false;
}

void TTftLcd_spi::ResetPanel()
{
	if (!pin_reset.Assigned())
	{
		return;
	}

	// issue hardware reset
	pin_reset.Set1();
	pin_cs.Set0();

	delay_ms(150);

	pin_reset.Set0();

	delay_ms(150);

	pin_reset.Set1();

	delay_ms(10);

	pin_cs.Set1();
}

void TTftLcd_spi::WriteCmd(uint8_t adata)
{
	pin_cd.Set0();
	pin_cs.Set0();
	spi.SendData(adata);
	spi.WaitSendFinish();
	pin_cs.Set1();
}

void TTftLcd_spi::WriteData8(uint8_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	spi.SendData(adata);
	spi.WaitSendFinish();
	pin_cs.Set1();
}

void TTftLcd_spi::WriteData16(uint16_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	spi.SendData(adata >> 8);
	spi.SendData(adata);
	spi.WaitSendFinish();
	pin_cs.Set1();
}

void TTftLcd_spi::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
	uint16_t x1 = x0 + w - 1;
	uint16_t y1 = y0 + h - 1;

	WriteCmd(0x2A);
	WriteData8(x0 >> 8);
	WriteData8(x0);
	WriteData8(x1 >> 8);
	WriteData8(x1);

	WriteCmd(0x2b);
	WriteData8(y0 >> 8);
	WriteData8(y0);
	WriteData8(y1 >> 8);
	WriteData8(y1);

	WriteCmd(0x2c);
}

void TTftLcd_spi::FillColor(uint16_t acolor, unsigned acount)
{
	pin_cd.Set1();
	pin_cs.Set0();
  while (acount > 0)
  {
  	spi.SendData(acolor >> 8);
  	spi.SendData(acolor);
  	//while (!spi.TrySendData(acolor >> 8)) ;
  	//while (!spi.TrySendData(acolor)) ;

  	--acount;
  }
	spi.WaitSendFinish();
	pin_cs.Set1();
}
