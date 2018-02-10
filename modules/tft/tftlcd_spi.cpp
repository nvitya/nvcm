// tftlcd_spi.cpp

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
