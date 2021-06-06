// charlcd.cpp

#include "charlcd.h"
#include "clockcnt.h"

bool TCharLcd::Init(TCharLcdCtrlType atype, unsigned acols, unsigned arows, uint8_t * ascreenbuf, uint8_t * achangemap)
{
	ctrltype = atype;

	TTextScreen::InitCharBuf(acols, arows, ascreenbuf, achangemap);

	initialized = false;

	if (!InitInterface())
	{
		return false;
	}

	InitPanel();

	initialized = true;
	return true;
}

bool TCharLcd::InitInterface()
{
	// should be overridden when needed
	return true;
}

void TCharLcd::WriteCmd(uint8_t adata)
{
	// should be overridden
}

void TCharLcd::WriteData(uint8_t adata)
{
	// should be overridden
}

void TCharLcd::DrawChar(unsigned acol, unsigned arow, char ach)
{
	uint8_t offs = acol;
	if (arow > 0)  offs += 0x40;

	WriteCmd(0x80 | offs); // DDRAM address
	WriteData(ach);
}

void TCharLcd::SetCursor()
{
}

void TCharLcd::InitPanel()
{
	uint8_t tmp;

	if (CHLCD_CTRL_HD44780 == ctrltype)
	{
		// set # lines, font size, etc.

		tmp = 0; // bit2: 0 = 5x8 font, 1 = 5x10 font
		if (!interface_4bit)  tmp |= (1 << 4); // bit4: 0 = 4 bit interface, 1 = 8 bit interface
		if (rows > 1)         tmp |= (1 << 3); // bit3: 1 = two row mode
		WriteCmd(0x20 | tmp);

		WriteCmd(0x08); // clear display

		delay_us(2000);

		WriteCmd(0x0C); // set the display on without cursor

		delay_us(2000);
	}
}

