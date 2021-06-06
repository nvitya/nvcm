// charlcd_i2c.cpp

#include "charlcd_i2c.h"
#include "clockcnt.h"

bool TCharLcd_i2c::InitInterface()
{
	if (!i2c.initialized)
	{
		return false;
	}

	interface_4bit = true;

	base_bits = 0; // E, RS, R/W default low
	if (backlight_on)  base_bits |= pinmask_bl;

	delay_us(50000); // wait for the start

	ExpanderWrite(base_bits);

	delay_us(100000);

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

void TCharLcd_i2c::WriteCmd(uint8_t acmd)
{
	base_bits &= ~(pinmask_rs);
	Write4Bits(acmd >> 4);  // high first
	Write4Bits(acmd & 0xF); // low  second
}

void TCharLcd_i2c::WriteData(uint8_t adata)
{
	base_bits |= pinmask_rs;
	Write4Bits(adata >> 4);  // high first
	Write4Bits(adata & 0xF); // low  second
}

void TCharLcd_i2c::Write4Bits(uint8_t adata)
{
	PulseEn((adata << 4) | base_bits);
}

void TCharLcd_i2c::PulseEn(uint8_t abasedata)
{
	ExpanderWrite(abasedata); // prepare the data before
	ExpanderWrite(abasedata | pinmask_en); // rising edge on E samples
	ExpanderWrite(abasedata); // pull back the E
}

void TCharLcd_i2c::ExpanderWrite(uint8_t adata)
{
	cmdbuf[0] = adata;
	i2c.StartWriteData(i2caddr, 0, &cmdbuf[0], 1); // write byte with addressing
	i2c.WaitFinish();
}
