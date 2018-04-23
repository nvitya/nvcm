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
 *  file:     textscreen.cpp
 *  brief:    Implementing text console on a TFT display
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <textscreen.h>

void TTextScreen::Init(TGfxBase * adisp, uint16_t x, uint16_t y, uint16_t w, uint16_t h, TGfxFont * amonofont)
{
	disp = adisp;

	disp_x = x;
	disp_y = y;
	disp_w = w;
	disp_h = h;

	font = amonofont;

	lineheight = font->y_advance;
	charwidth = font->CharWidth('W');

	cols = disp_w / charwidth;
	rows = disp->height / lineheight;
	curcol = 0;

	// fill the text buffer with spaces
	memset(&screenbuf[0], 32, cols * rows);
	memset(&changemap[0], 0, (cols * rows + 7) >> 3);

	// initial clear
	disp->FillRect(disp_x, disp_y, disp_w, disp_h, disp->bgcolor);
}

void TTextScreen::SetScreenBufChar(uint16_t aaddr, char ach)
{
	if (screenbuf[aaddr] != ach)
	{
		screenbuf[aaddr] = ach;
		changemap[aaddr >> 3] |= (1 << (aaddr & 0x7));
		screenchanged = true;
	}
}

void TTextScreen::WriteChar(char ach)
{
	int  row, col;
	bool newline = false;

	if (ach == 10)
	{
		newline = true;
	}
	else if (uint8_t(ach) < 32)
	{
		// skip special chars
	}
	else
	{
		SetScreenBufChar(curcol, ach);
		++curcol;
		if (curcol >= cols)
		{
			newline = true;
		}
	}

	if (newline)
	{
		// scroll the screen
		for (row = rows - 2; row >= 0; --row)
		{
			for (col = 0; col < cols; ++col)
			{
				uint16_t saddr = (row * cols) + col;
				SetScreenBufChar(saddr + cols, screenbuf[saddr]);
			}
		}

		// fill the first row with spaces
		for (col = 0; col < cols; ++col)
		{
			SetScreenBufChar(col, 32);
		}

		curcol = 0;
	}
}

void TTextScreen::Update()
{
	if (!screenchanged)
	{
		return;
	}

	int  row, col;

	// run trough the change map and draw characters on the screen
	for (row = 0; row < rows; ++row)
	{
		for (col = 0; col < cols; ++col)
		{
			uint16_t saddr = (row * cols) + col;
			if (changemap[saddr >> 3] & (1 << (saddr & 0x7)))
			{
				// changed, draw the character
				disp->SetCursor(disp_x + col * charwidth, disp_y + lineheight * (rows - row - 1));
				TGfxGlyph * glyph = font->GetGlyph(screenbuf[saddr]);
				if (!glyph)  glyph = font->GetGlyph('.');  // replace non-existing characters with dot
				disp->DrawGlyph(font, glyph);
				changemap[saddr >> 3] &= ~(1 << (saddr & 0x7));
			}
		}
	}

	screenchanged = false;
}

#define FMT_BUFFER_SIZE  256

void TTextScreen::printf(const char* fmt, ...)
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
  	WriteChar(*pch);
    ++pch;
  }

  va_end(arglist);
}
