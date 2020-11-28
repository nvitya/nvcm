// monolcd.cpp

#include "monolcd.h"
#include "clockcnt.h"

bool TMonoLcd::Init(TMonoLcdCtrlType atype, uint16_t awidth, uint16_t aheight, uint8_t * adispbuf)
{
	InitGfx();

	ctrltype = atype;
	hwwidth = awidth;
	hwheight = aheight;

	initialized = false;

	pdispbuf = adispbuf;

	if (!adispbuf)
	{
		return false;
	}

	if (!InitInterface())
	{
		return false;
	}

	InitPanel();

	initialized = true;
	return true;
}

bool TMonoLcd::InitInterface()
{
	// should be overridden when needed
	return true;
}

void TMonoLcd::WriteCmd(uint8_t adata)
{
	// should be overridden
}

void TMonoLcd::WriteData(uint8_t adata)
{
	// should be overridden
}

void TMonoLcd::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
	aw_x0 = x0;
	aw_y0 = y0;
	aw_x1 = x0 + w - 1;
	aw_y1 = y0 + h - 1;

	if (aw_x1 > width)   aw_x1 = width;
	if (aw_y1 > height)  aw_y1 = height;

	aw_x = x0;
	aw_y = y0;
}

void TMonoLcd::SetAddrWindowStart(uint16_t x0, uint16_t y0)
{
	aw_x0 = x0;
	aw_y0 = y0;
	aw_x = x0;
	aw_y = y0;
}

void TMonoLcd::FillColor(uint16_t acolor, unsigned acount)
{
	while (acount > 0)
	{
		uint32_t byteidx = width * (aw_y >> 3) + aw_x;
		uint8_t  bit = (1 << (aw_y & 7));
		if (acolor & 1)
		{
			pdispbuf[byteidx] |= bit;
		}
		else
		{
			pdispbuf[byteidx] &= ~bit;
		}

		++aw_x;
		if (aw_x > aw_x1)
		{
			aw_x = aw_x0;
			++aw_y;
			if (aw_y > aw_y1)  aw_y = aw_y0;
		}
		--acount;
	}
	++updatecnt;
}

void TMonoLcd::Run()
{
	// should be overridden
}

void TMonoLcd::SetDisplayOn(bool aon)
{
	if (aon)
	{
		WriteCmd(0xAF); // display on
	}
	else
	{
		WriteCmd(0xAE); // display off
	}
}

void TMonoLcd::InitPanel()
{
	if (MLCD_CTRL_UC1701 == ctrltype)
	{
		WriteCmd(0xE2);  //System Reset
		WriteCmd(0x40); // Set display start line to 0
		//WriteCmd(0xA1); //Set SEG Direction
		//WriteCmd(0xC0); //Set COM Direction
		WriteCmd(0xA2); //Set Bias = 1/9
		WriteCmd(0x2C);  //Boost ON
		WriteCmd(0x2E); //Voltage Regular On
		WriteCmd(0x2F); //Voltage Follower On
		WriteCmd(0xF8); //Set booster ratio to
		WriteCmd(0x00); //4x
		WriteCmd(0x23); //Set Resistor Ratio = 3
		WriteCmd(0x81);
		WriteCmd(0x28); //Set Electronic Volume = 40
		WriteCmd(0xAC);//Set Static indicator off
		WriteCmd(0x00);
		WriteCmd(0XA6); // Disable inverse
		WriteCmd(0xAF); //Set Display Enable
		delay_us(1000);
		WriteCmd(0xA5); //display all points
		delay_us(2000);
		WriteCmd(0xA4); //normal display
	}
	else if (MLCD_CTRL_PCD8544 == ctrltype)
	{
		WriteCmd(0x21); // get into the EXTENDED mode!
		WriteCmd(0x10 | 4); // LCD bias select (4 is optimal?)
	  WriteCmd(0x80 | contrast); // set VOP (contrast)
	  WriteCmd(0x20); // normal mode
	  // Set display to Normal
	  WriteCmd(0x08 | 0x4);  // PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL
	}
	else if (MLCD_CTRL_HX1230 == ctrltype)
	{
    WriteCmd(0x2f);
    WriteCmd(0x90);
    WriteCmd(0xa6);
    WriteCmd(0xa4);
    WriteCmd(0xaf);  // power on
    WriteCmd(0x40);
    WriteCmd(0xb0);
    WriteCmd(0x10);
    WriteCmd(0x00);

    // set contrast:
	  WriteCmd(0x80 | (contrast & 31));
	}
	else if (MLCD_CTRL_ST75256 == ctrltype)
	{

#if 1

	  WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0x94);				/* sleep out */

	  WriteCmd(0xae);				/* display off */

	  WriteCmd(0x31);				/* select 01 commands */
	  WriteCmd(0xd7);  WriteData(0x9f);		/* disable auto read */

	  //WriteCmd(0x31);				/* select 01 commands */
	  WriteCmd(0x32);				/* analog circuit set */
	  WriteData(0x00);				/* code example: OSC Frequency adjustment */
	  WriteData(0x01);				/* Frequency on booster capacitors 1 = 6KHz? */
	  WriteData(0x00);				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */

#if 0
	  //WriteCmd(0x31);				/* select 01 commands */
	  WriteCmd(0x20);				/* gray levels */
	  WriteData(0x01);
	  WriteData(0x03);
	  WriteData(0x05);
	  WriteData(0x07);
	  WriteData(0x09);
	  WriteData(0x0b);
	  WriteData(0x0d);
	  WriteData(0x10);
	  WriteData(0x11);
	  WriteData(0x13);
	  WriteData(0x15);
	  WriteData(0x17);
	  WriteData(0x19);
	  WriteData(0x1b);
	  WriteData(0x1d);
	  WriteData(0x1f);
#endif


	  WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0x75);  WriteData(0);  WriteData(0x28);		/* row range */
	  WriteCmd(0x15);  WriteData(0);  WriteData(0xFF);		/* col range */

	  //WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0xbc);  WriteData(0x00);			/* data scan dir */
	  //WriteData(0xa6);				/* ??? */

	  //WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0x0c);				/* data format LSB top */

	  //WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0xca);				/* display control, 3 args follow  */
	  WriteData(0x00);				/* 0x00: no clock division, 0x04: devide clock */
	  WriteData(hwheight-1);				/* 1/160 duty value from the DS example code */
	  WriteData(0x20);				/* nline off */

	  //WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0xf0);  WriteData(0x10);		/* monochrome mode  = 0x010*/
	  //WriteCmd(0xf0);  WriteData(0x11);		/* 0x11 = grayscale */

	    //WriteCmd(0x30);				/* select 00 commands */

// contrast

	  //WriteCmd(0x81);  WriteData(0x18);  WriteData(0x05);	/* Volume control */

	  uint16_t vop = 0x170;
	  WriteCmd(0x81);  WriteData(vop & 0x3F);  WriteData((vop >> 6) & 7);	/* Volume control */

	    //WriteCmd(0x30);				/* select 00 commands */
	  WriteCmd(0x20);  WriteData(0x0b);		/* Power control: Regulator, follower & booster on */

	  //U8X8_DLY(100),
	  delay_us(100);

	  WriteCmd(0xaf);				/* display on */



#endif

	}

  SetRotation(rotation);
}

void TMonoLcd::SetRotation(uint8_t m)
{
	if (MLCD_CTRL_UC1701 == ctrltype)
	{
		rotation = m;

		switch (rotation)
		{
		case 0:
		default:
			width = hwwidth;
			height = hwheight;
			WriteCmd(0xC8);  // set com scan direction to remap
			WriteCmd(0xA0);  // set segment re-map to remap
			break;
		case 2:
			width = hwwidth;
			height = hwheight;
			WriteCmd(0xC0);  // set com scan direction to normal
			WriteCmd(0xA1);  // set segment re-map to normal
			break;
		}
	}
	else if (MLCD_CTRL_HX1230 == ctrltype)
	{
		// somehow flipping segments does not work, only com scan

		rotation = m;

		switch (rotation)
		{
		case 0:
		default:
			width = hwwidth;
			height = hwheight;
			//WriteCmd(0xA0);  // set segment re-map to remap
			//WriteCmd(0xC8);  // set com scan direction to remap
			break;
		case 2:
			width = hwwidth;
			height = hwheight;
			//WriteCmd(0xC0);  // set com scan direction to normal
			//WriteCmd(0xA0);  // set segment re-map to normal
			break;
		}
	}
	else
	{
		rotation = 0;
		width = hwwidth;
		height = hwheight;
		return;
	}
}

void TMonoLcd::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	uint16_t endx = x + w;
	uint16_t endy = y + h;
	uint16_t lx, ly;

	if (endx > width)
	{
		if (x > width)  return;
		endx = width;
	}

	if (endy > height)
	{
		if (y > height)  return;
		endy = height;
	}

	ly = y;
	while (ly < endy)
	{
		lx = x;
		uint8_t  bitp = 0;

		while (ly < endy)
		{
			bitp |= (1 << (ly & 7));
			if ((ly & 7) == 7)
			{
				break;
			}
			++ly;
		}

		uint32_t byteidx = width * (ly >> 3) + lx;

		while (lx < endx)
		{
			if (color & 1)
			{
				pdispbuf[byteidx] |= bitp;
			}
			else
			{
				pdispbuf[byteidx] &= ~bitp;
			}
			++lx;
			++byteidx;
		}
		++ly;
	}

	++updatecnt;
}

void TMonoLcd::DrawPixel(int16_t x, int16_t y, uint16_t color)  // faster draw-pixel
{
	if ((x >= width) || (y >= height))
	{
		return;
	}
	uint32_t byteidx = width * (y >> 3) + x;
	uint8_t  bit = (1 << (y & 7));
	if (color & 1)
	{
		pdispbuf[byteidx] |= bit;
	}
	else
	{
		pdispbuf[byteidx] &= ~bit;
	}
	++updatecnt;
}

bool TMonoLcd::UpdateFinished()
{
	Run();
	if ((updatestate == 0) && (updatecnt == lastupdate))
	{
		return true;
	}
	else
	{
		return false;
	}
}
