// oleddisp.cpp

#include "oleddisp.h"
#include "clockcnt.h"

bool TOledDisp::Init(TOledCtrlType atype, uint16_t awidth, uint16_t aheight, uint8_t * adispbuf)
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

bool TOledDisp::InitInterface()
{
	// should be overridden when needed
	return true;
}

void TOledDisp::WriteCmd(uint8_t adata)
{
	// should be overridden
}

void TOledDisp::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
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

void TOledDisp::SetAddrWindowStart(uint16_t x0, uint16_t y0)
{
	aw_x0 = x0;
	aw_y0 = y0;
	aw_x = x0;
	aw_y = y0;
}

void TOledDisp::FillColor(uint16_t acolor, unsigned acount)
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

void TOledDisp::Run()
{
	// should be overridden
}

void TOledDisp::SetDisplayOn(bool aon)
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

void TOledDisp::InitPanel()
{
	WriteCmd(0xAE); // display off
	WriteCmd(0xD5); WriteCmd(0x80); // set display clock div

	WriteCmd(0xA8); WriteCmd(hwheight - 1);  // set multiplex

	WriteCmd(0xD3); WriteCmd(0);  // set display offset
	WriteCmd(0x40 | 0); // set start line

	WriteCmd(0x8D); WriteCmd(externalvcc ? 0x10 : 0x14); // chargepump

	WriteCmd(0x20); WriteCmd(0x00); // memory mode

	if (hwheight == 64)
	{
		WriteCmd(0xDA); WriteCmd(0x12); // setcompins
		contrast = (externalvcc ? 0x9F : 0xCF);
	}
	else
	{
		WriteCmd(0xDA); WriteCmd(0x02); // setcompins
		contrast = 0x8F;
	}

	WriteCmd(0x81); WriteCmd(contrast); // setcontrast

	WriteCmd(0xD9); WriteCmd(externalvcc ? 0x22 : 0xF1); // setprecharge
	WriteCmd(0xDB); WriteCmd(0x40); // setvcomdetect
	WriteCmd(0xA4); // displayon_resume
	WriteCmd(0xA6); // normal display

	WriteCmd(0x2E); // deactivate scroll

	SetRotation(rotation);

	WriteCmd(0xAF); // display on
}

void TOledDisp::SetRotation(uint8_t m)
{
	rotation = m;

  switch (rotation)
  {
  case 0:
  default:
  	width = hwwidth;
  	height = hwheight;
		WriteCmd(0xC8);  // set com scan direction to remap
		WriteCmd(0xA1);  // set segment re-map to remap
  	break;
  case 2:
  	width = hwwidth;
  	height = hwheight;
		WriteCmd(0xC0);  // set com scan direction to normal
		WriteCmd(0xA0);  // set segment re-map to normal
  	break;
  }
}

void TOledDisp::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
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

void TOledDisp::DrawPixel(int16_t x, int16_t y, uint16_t color)  // faster draw-pixel
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

bool TOledDisp::UpdateFinished()
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
