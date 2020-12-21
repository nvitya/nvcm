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
 *  file:     gfxbase.cpp
 *  brief:    Base Pixel graphics library for graphical displays
 *  version:  1.00
 *  date:     2018-04-14
 *  authors:  nvitya, Adafruit Industries
 *  note:
 *    The Implementation is similar to Adafruit_GFX, but not the same.
 *    Font definition it taken from Adafruit_GFX to be able to use the Adafruit fonts directly
 */

/* Adafruit Copyright message:

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "string.h"
#include <stdarg.h>
#include "mp_printf.h"
#include "gfxbase.h"
#include "math.h"

#include "stdmonofont.h"

TGfxFont font_gfx_standard(&stdmonofont);

bool TGfxFont::Load(const GFXfont * afontdata)
{
	bitmap = afontdata->bitmap;
	glyph = afontdata->glyph;
	firstchar = afontdata->first;
	lastchar = afontdata->last;
	y_advance = afontdata->yAdvance;

	uint16_t  charcode = afontdata->first;
	TGfxGlyph * glyph = &afontdata->glyph[0];

	uint8_t maxascend = 0;
	uint8_t maxdescend = 0;

	while (charcode <= afontdata->last)
	{
		int8_t ascend   = -glyph->yOffset;
		int8_t descend  = glyph->height - ascend;
		if (ascend > maxascend)    maxascend = ascend;
		if (descend > maxdescend)  maxdescend = descend;
		++glyph;
		++charcode;
	}

	ascend = maxascend;
	descend = maxdescend;
	height = ascend + descend;

	return true;
}

uint16_t TGfxFont::CharWidth(char achar)
{
	TGfxGlyph * glyph = GetGlyph(achar);
	if (glyph)
	{
	  uint8_t   w  = glyph->width;
	  int8_t    xo = glyph->xOffset;
	  uint16_t  cw  = glyph->xAdvance;
	  if (w + xo > cw)  cw = w + xo;
	  return cw;
	}
	else
	{
		return 0;
	}
}

uint16_t TGfxFont::TextWidth(const char * astr)
{
	uint16_t w = 0;

	while (*astr != 0)
	{
		w += CharWidth(*astr);
		++astr;
	}

	return w;
}

TGfxGlyph * TGfxFont::GetGlyph(char achar)
{
	uint8_t charcode = uint8_t(achar);

	if (charcode < firstchar)
	{
		return nullptr;
	}

	if (charcode > lastchar)
	{
		return nullptr;
	}

	return &glyph[charcode - firstchar];
}

//--------------------------------------------------------------------------------------------

void TGfxBase::InitGfx()
{
	SetFont(&font_gfx_standard);
}

void TGfxBase::DrawPixel(int16_t x, int16_t y, uint16_t color)
{
	// must be overridden
}

void TGfxBase::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// can be overridden

  if ((x >= width) || (y >= height))  return;

  if ((x + w - 1) >= width)  w = width  - x;
  if ((y + h - 1) >= height) h = height - y;

	SetAddrWindow(x, y, w, h);
	FillColor(color, w * h);
}

void TGfxBase::FillScreen(uint16_t color) // can be overridden
{
	FillRect(0, 0, width, height, color);
}

void TGfxBase::DrawChar(char achar)
{
	if (!font)
	{
		return;
	}

	TGfxGlyph * glyph = font->GetGlyph(achar);
	if (glyph)
	{
		DrawGlyph(font, glyph);
	}
}

void TGfxBase::DrawString(char * astr)
{
	if (!font)
	{
		return;
	}

	uint8_t charcode;

	while (true)
	{
		char charcode = uint8_t(*astr);
		if (charcode == 0)
		{
			return;
		}
		TGfxGlyph * glyph = font->GetGlyph(charcode);
		if (glyph)
		{
			DrawGlyph(font, glyph);
		}
		++astr;
	}
}

#define FMT_BUFFER_SIZE  128

void TGfxBase::printf(const char * fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	char * pch;

	// allocate format buffer on the stack:
	char fmtbuf[FMT_BUFFER_SIZE];

	pch = &fmtbuf[0];

	*pch = 0;

	mp_vsnprintf(pch, FMT_BUFFER_SIZE, fmt, arglist);

	while (*pch != 0)
	{
		DrawChar(*pch);
		++pch;
	}

	va_end(arglist);
}

void TGfxBase::DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if ((x0 == x1) || (y0 == y1))
  {
  	FillRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1, color);
  	return;
  }

  int16_t t;
  if (steep)
  {
  	t = x0; x0 = y0; y0 = t; // swap x0, y0
  	t = x1; x1 = y1; y1 = t; // swap x1, y1
  }

  if (x0 > x1)
  {
  	t = x0; x0 = x1; x1 = t; // swap x0, x1
  	t = y0; y0 = y1; y1 = t; // swap y0, y1
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
		if (steep)
		{
			DrawPixel(y0, x0, color);
		}
		else
		{
			DrawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
  }
}

void TGfxBase::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
  // must be overridden
}

void TGfxBase::SetAddrWindowStart(uint16_t x0, uint16_t y0)
{
  // must be overridden
}

void TGfxBase::FillColor(uint16_t acolor, unsigned acount)
{
	 // must be overridden
}

void TGfxBase::LineTo(int16_t x, int16_t y)
{
	DrawLine(cursor_x, cursor_y, x, y);
	cursor_x = x;
	cursor_y = y;
}

#if 0
void TGfxBase::DrawGlyph(TGfxGlyph * glyph)
{
  uint8_t  * bitmap = font->bitmap;

  uint16_t bo = glyph->bitmapOffset;
  uint8_t  w  = glyph->width,
           h  = glyph->height;
  int8_t   xo = glyph->xOffset,
           yo = glyph->yOffset + font->ascend;
  uint8_t  xx, yy;
  uint8_t  bits = 0, bit = 0;

  if ((cursor_x + xo < width) && (cursor_y + yo < height))
  {
  	uint16_t dw = w;
  	uint16_t dh = h;
  	if (cursor_x + xo + w > width)  dw = width - cursor_x - xo;
  	if (cursor_y + yo + h > height)  dh = height - cursor_y - yo;

		SetAddrWindow(cursor_x + xo, cursor_y + yo, dw, dh);

		uint16_t carr[2];
		carr[0] = bgcolor;
		carr[1] = color;

		for (yy = 0; yy < dh; ++yy)
		{
			for (xx = 0; xx < w; ++xx)
			{
				if ((bit & 7) == 0) // load the bits
				{
					bits = bitmap[bo++];
				}

				if (xx < dw)
				{
				  FillColor(carr[(bits >> 7) & 1], 1);
				}

				++bit;
				bits <<= 1;
			}
		}
  }
  cursor_x += glyph->xAdvance;
}

#else

void TGfxBase::DrawGlyph(TGfxFont * afont, TGfxGlyph * glyph)
{
  uint8_t  w  = glyph->width,
           h  = glyph->height;
  int8_t   xo = glyph->xOffset,
           yo = glyph->yOffset + afont->ascend;

  uint8_t  dw  = glyph->xAdvance;
  uint8_t  dh  = afont->height;

  if (w + xo > dw)  dw = w + xo;

  if ((cursor_x < width) && (cursor_y < height))
  {
  	if (cursor_x + dw > width)   dw = width - cursor_x;
  	if (cursor_y + dh > height)  dh = height - cursor_y;

		SetAddrWindow(cursor_x, cursor_y, dw, dh);

	  uint8_t  * bmptr = &afont->bitmap[glyph->bitmapOffset];
	  uint8_t  x, y;
	  uint8_t  bits = 0, bit = 0;

	  if (xo < 0) xo = 0;

		uint16_t carr[2];
		carr[0] = bgcolor;
		carr[1] = color;

		uint8_t endx = xo + w; // scan always minimum the full character width to follow the bits correctly
		if (endx < dw)  endx = dw;

		for (y = 0; y < dh; ++y)
		{
			for (x = 0; x < endx; ++x)
			{
				if (   (y >= yo) && (y < yo + h)
				    && (x >= xo) && (x < xo + w))
				{
					if ((bit & 7) == 0) // load the bits
					{
						bits = *bmptr++;
					}

					if (x < dw) // ignore pixels outside the address window
					{
						FillColor(carr[(bits >> 7) & 1], 1);
					}

					++bit;
					bits <<= 1;
				}
				else
				{
					if (x < dw)
					{
						FillColor(bgcolor, 1);
					}
				}
			}
		}
  }
  cursor_x += dw;
}

#endif

void TGfxBase::DrawRect(int16_t x0, int16_t y0, int16_t w, int16_t h)
{
	int16_t x1 = x0 + w - 1;
	int16_t y1 = y0 + h - 1;
	DrawLine(x0, y0, x1, y0);
	DrawLine(x1, y0, x1, y1);
	DrawLine(x0, y0, x0, y1);
	DrawLine(x0, y1, x1, y1);
}

