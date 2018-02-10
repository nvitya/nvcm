// tftlcd_mm16.cpp

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
