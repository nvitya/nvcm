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
 *  file:     tftlcd_mm16c.cpp
 *  brief:    16 bit parallel TFT LCD Display driver using external memory controller
 *            Chacheing version (some Cortex M7 microcontrollers)
 *  version:  1.00
 *  date:     2018-09-01
 *  authors:  nvitya
*/

#include "tftlcd_mm16c.h"

bool TTftLcd_mm16c::InitInterface()
{
	// should be overridden !
	return false;
}

void TTftLcd_mm16c::WriteCmd(uint8_t adata)
{
	*cmdreg16 = adata;
	__DSB();
}

void TTftLcd_mm16c::WriteData8(uint8_t adata)
{
	*datareg16 = adata;
	__DSB();
}

void TTftLcd_mm16c::WriteData16(uint16_t adata)
{
	*datareg16 = adata;
	__DSB();
}

void TTftLcd_mm16c::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
	uint16_t x1 = x0 + w - 1;
	uint16_t y1 = y0 + h - 1;

#if 0
	__DSB();
	*cmdreg = 0x2a;
	__DSB();
	*datareg = (x0 >> 8);
	__DSB();
	*datareg = (x0);
	__DSB();
	*datareg = (x1 >> 8);
	__DSB();
	*datareg = (x1);
	__DSB();

	*cmdreg = 0x2b;
	__DSB();
	*datareg = (y0 >> 8);
	__DSB();
	*datareg = (y0);
	__DSB();
	*datareg = (y1 >> 8);
	__DSB();
	*datareg = (y1);
	__DSB();

	*cmdreg = 0x2c;
	__DSB();

#else

#if 1
	*cmdreg16 = 0x2A;
	__DSB();
	*datareg32 = ((x0 >> 8) & 0xFF) | ((x0 << 16) & 0x00FF0000); // x0 high + x0 low
	__DSB();
	*datareg32 = ((x1 >> 8) & 0xFF) | ((x1 << 16) & 0x00FF0000); // x1 high + x1 low
	__DSB();
	*cmdreg16 = 0x2B;
	__DSB();
	*datareg32 = ((y0 >> 8) & 0xFF) | ((y0 << 16) & 0x00FF0000); // y0 high + y0 low
	__DSB();
	*datareg32 = ((y1 >> 8) & 0xFF) | ((y1 << 16) & 0x00FF0000); // y1 high + y1 low
	__DSB();
	*cmdreg16 = 0x2C;
	__DSB();
#else
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
#endif
#endif
}

void TTftLcd_mm16c::FillColor(uint16_t acolor, unsigned acount)
{
	if (acount > 7)
	{
		uint32_t c32 = (acount >> 1);
		acount = acount & 1;
		uint32_t d32 = (acolor << 16) | acolor;
		while (c32 > 0)
		{
			*datareg32 = d32;
			__DSB();
			--c32;
		}
	}

	while (acount > 0)
	{
		*datareg16 = acolor;
		__DSB();
		--acount;
	}
}
