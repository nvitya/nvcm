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

void TTextScreen::Init(TTftLcd * alcd)
{
	lcd = alcd;

	lineheight = 9;
	cols = lcd->width / 6;
	xoffs = (lcd->width % 6) / 2;
	rows = lcd->height / lineheight;
	yoffs = (lcd->height % lineheight) / 2;
	curcol = 0;

	// fill the text buffer with spaces
	memset(&screenbuf[0], 32, cols * rows);
	memset(&changemap[0], 0, (cols * rows + 7) >> 3);

	// initial clear
	lcd->FillScreen(lcd->bgcolor);
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
				lcd->DrawChar(xoffs + col * 6, yoffs + lineheight * (rows - row - 1), screenbuf[saddr]);

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
