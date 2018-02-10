// tftlcd_mm16.cpp

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
