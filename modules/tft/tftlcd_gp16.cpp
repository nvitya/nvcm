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
 *  file:     tftlcd_gp16.cpp
 *  brief:    16 bit parallel TFT LCD Display driver using GPIO
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "clockcnt.h"
#include "tftlcd_gp16.h"

bool TTftLcd_gp16::InitInterface()
{
	// should be overridden !
	return false;
}

void TTftLcd_gp16::ResetPanel()
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

void TTftLcd_gp16::SetData8(uint16_t adata)
{
	TGpioPin * pgpio = &pin_d[0];
	uint16_t bit = 1;

	do
	{
		if (adata & bit)
		{
			pgpio->Set1();
		}
		else
		{
			pgpio->Set0();
		}
		bit <<= 1;
		++pgpio;
	}
	while (bit & 0xFF);
}

void TTftLcd_gp16::SetData16(uint16_t adata)
{
	TGpioPin * pgpio = &pin_d[0];
	uint16_t bit = 1;

	do
	{
		if (adata & bit)
		{
			pgpio->Set1();
		}
		else
		{
			pgpio->Set0();
		}
		bit <<= 1;
		++pgpio;
	}
	while (bit & 0xFFFF);
}

void TTftLcd_gp16::WriteCmd(uint8_t adata)
{
	pin_cd.Set0();
	pin_cs.Set0();
	SetData8(adata);
	pin_wr.Set0();
	//delay_clocks(data_hold_clocks);
	pin_wr.Set1();
	pin_cs.Set1();
}

void TTftLcd_gp16::WriteData8(uint8_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	SetData8(adata);
	pin_wr.Set0();
	//delay_clocks(data_hold_clocks);
	pin_wr.Set1();
	pin_cs.Set1();
}

void TTftLcd_gp16::WriteData16(uint16_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	SetData16(adata);
	pin_wr.Set0();
	//delay_clocks(data_hold_clocks);
	pin_wr.Set1();
	pin_cs.Set1();
}

void TTftLcd_gp16::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
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

void TTftLcd_gp16::FillColor(uint16_t acolor, unsigned acount)
{
	pin_cd.Set1();
	pin_cs.Set0();
	SetData16(acolor);
  while (acount > 0)
  {
  	pin_wr.Set0();
  	//delay_clocks(data_hold_clocks);
  	pin_wr.Set1();
  	--acount;
  }
	pin_cs.Set1();
}
