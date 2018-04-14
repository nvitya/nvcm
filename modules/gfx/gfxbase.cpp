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
#include <stdio.h>
#include <stdarg.h>
#include "gfxbase.h"

void TGfxBase::DrawPixel(int16_t x, int16_t y, uint16_t color)
{
	// must be overridden
}

void TGfxBase::SetFont(const GFXfont * afont)
{
	pfont = afont;
}

void TGfxBase::DrawChar(char achar)
{
	if (!pfont)
	{
		return;
	}

	uint8_t charcode = uint8_t(achar);

	if (charcode < pfont->first)
	{
		return;
	}

	if (charcode > pfont->last)
	{
		return;
	}

	GFXglyph * glyph = &pfont->glyph[charcode - pfont->first];
	DrawGlyph(glyph);
}

uint16_t TGfxBase::GetFontHeight()
{
	if (!pfont)
	{
		return 1;
	}

	return pfont->yAdvance;
}

void TGfxBase::DrawString(char * astr)
{
	if (!pfont)
	{
		return;
	}

	uint8_t charcode;

	while (true)
	{
		uint8_t charcode = uint8_t(*astr);
		if (charcode == 0)
		{
			return;
		}

		if (charcode < pfont->first)
		{
			continue;
		}

		if (charcode > pfont->last)
		{
			continue;
		}

		GFXglyph * glyph = &pfont->glyph[charcode - pfont->first];
		DrawGlyph(glyph);
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

	vsnprintf(pch, FMT_BUFFER_SIZE, fmt, arglist);

	while (*pch != 0)
	{
		DrawChar(*pch);
		++pch;
	}

	va_end(arglist);
}

void TGfxBase::DrawGlyph(GFXglyph * glyph)
{
  uint8_t  * bitmap = pfont->bitmap;

  uint16_t bo = glyph->bitmapOffset;
  uint8_t  w  = glyph->width,
           h  = glyph->height;
  int8_t   xo = glyph->xOffset,
           yo = glyph->yOffset;
  uint8_t  xx, yy;
  uint8_t  bits = 0, bit = 0;

  uint16_t carr[2];
  carr[0] = bgcolor;
  carr[1] = color;

  for (yy = 0; yy < h; ++yy)
  {
		for (xx = 0; xx < w; ++xx)
		{
			if ((bit & 7) == 0) // load the bits
			{
				bits = bitmap[bo++];
			}

			DrawPixel(cursor_x + xo + xx, cursor_y + yo + yy, carr[bits >> 7]);

			++bit;
			bits <<= 1;
		}
  }

  cursor_x += glyph->xAdvance;
}
