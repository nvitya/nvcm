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
 *  file:     tftlcd_mm16.cpp
 *  brief:    16 bit parallel TFT LCD Display driver using external memory controller
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "tftlcd_mm16.h"

bool TTftLcd_mm16::InitInterface()
{
	// should be overridden !
	return false;
}

void TTftLcd_mm16::WriteCmd(uint8_t adata)
{
	*cmdreg = adata;
}

void TTftLcd_mm16::WriteData8(uint8_t adata)
{
	*datareg = adata;
}

void TTftLcd_mm16::WriteData16(uint16_t adata)
{
	*datareg = adata;
}

void TTftLcd_mm16::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
	uint16_t x1 = x0 + w - 1;
	uint16_t y1 = y0 + h - 1;

	*cmdreg = 0x2a;
	*datareg = (x0 >> 8);
	*datareg = (x0);
	*datareg = (x1 >> 8);
	*datareg = (x1);

	*cmdreg = 0x2b;
	*datareg = (y0 >> 8);
	*datareg = (y0);
	*datareg = (y1 >> 8);
	*datareg = (y1);

	*cmdreg = 0x2c;
}

void TTftLcd_mm16::FillColor(uint16_t acolor, unsigned acount)
{
  while (acount > 0)
  {
  	*datareg = acolor;
  	--acount;
  }
}
