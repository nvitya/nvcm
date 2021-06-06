// charlcd_bb4.cpp

#include "charlcd_bb4.h"
#include "clockcnt.h"

bool TCharLcd_bb4::InitInterface()
{
	if (   !pin_rs.Assigned() || !pin_en.Assigned()
			|| !pin_data[0].Assigned()
			|| !pin_data[1].Assigned()
			|| !pin_data[2].Assigned()
			|| !pin_data[3].Assigned()
			)
	{
		return false;
	}

	interface_4bit = true;

	// initialize pins

	pin_rs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	pin_en.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	if (pin_rw.Assigned())
	{
		pin_rw.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	}

	for (unsigned n = 0; n < 4; ++n)
	{
		pin_data[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	}

	// initialize the 4-bit interface

	delay_us(100000); // wait for the start, min 40 ms

	// 1. put the LCD into 4 bit mode
	//    according to the hitachi HD44780 datasheet: figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	Write4Bits(0x03);
	delay_us(4500); // wait min 4.1ms

	// second try
	Write4Bits(0x03);
	delay_us(4500); // wait min 4.1ms

	// third go!
	Write4Bits(0x03);
	delay_us(150);

	// finally, set to 4-bit interface
	Write4Bits(0x02);

	// the rest of the setup is done at the initpanel

	return true;
}

void TCharLcd_bb4::WriteCmd(uint8_t acmd)
{
	pin_rs.Set0();
	Write4Bits(acmd >> 4);  // high first
	Write4Bits(acmd & 0xF); // low  second
}

void TCharLcd_bb4::WriteData(uint8_t adata)
{
	pin_rs.Set1();
	Write4Bits(adata >> 4);  // high first
	Write4Bits(adata & 0xF); // low  second
}

void TCharLcd_bb4::Write4Bits(uint8_t adata)
{
	SetDataBits(adata);
	delay_us(2);
	pin_en.Set1();
	delay_us(2);
	pin_en.Set0();
	delay_us(50); // some commands require 37 us to execute
}

void TCharLcd_bb4::SetDataBits(uint8_t adata)
{
	for (unsigned n = 0; n < 4; ++n)
	{
		if (adata & (1 << n))
		{
			pin_data[n].Set1();
		}
		else
		{
			pin_data[n].Set0();
		}
	}
}
