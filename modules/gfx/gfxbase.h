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
 *  file:     gfxbase.h
 *  brief:    Base Pixel graphics library for graphical displays
 *  version:  1.00
 *  date:     2018-04-14
 *  authors:  nvitya, Adafruit Industries
 *  note:
 *    Some ideas and settings were taken from the Adafruit_GFX library.
 *    Adafruit_GFX fonts can be used directly.
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

//

#ifndef GFXBASE_H_
#define GFXBASE_H_

#include "stdint.h"

// Use the Adafruit_GFX font definition
#include "gfxfont.h"

class TGfxBase
{
public:
	const GFXfont * pfont = nullptr;

	uint8_t     rotation = 0;

	uint16_t    bgcolor = 0x0000;
	uint16_t    color   = 0xFFFF;  // actual color

	uint16_t		width =  0;    // might change by rotation
	uint16_t		height = 0;    // might change by rotation

	uint16_t		hwwidth =  0;  // shows the hardware dimension, does not change by rotation
	uint16_t		hwheight = 0;  // shows the hardware dimension, does not change by rotation

	uint16_t    cursor_x = 0;
	uint16_t    cursor_y = 0;

	// last address window settings
	uint16_t    aw_x0 = 0;
	uint16_t    aw_y0 = 0;
	uint16_t    aw_x1 = 0;
	uint16_t    aw_y1 = 0;

	virtual ~TGfxBase() { } // virtual destructor

	virtual void DrawPixel(int16_t x, int16_t y, uint16_t color);
	virtual void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	virtual void FillScreen(uint16_t color);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void SetAddrWindowStart(uint16_t x0, uint16_t y0);
	virtual void FillColor(uint16_t acolor, unsigned acount);

	void         DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
	void         DrawRect(int16_t x0, int16_t y0, int16_t w, int16_t h);

public:
  void SetFont(const GFXfont * afont);

  uint16_t GetFontHeight();
  uint8_t  GetFontMetrics(const GFXfont * afont, uint8_t * rascend, uint8_t * rdescend);

  void SetCursor(int16_t x, int16_t y)  { cursor_x = x; cursor_y = y; }

  void DrawChar(char achar);
  void DrawString(char * astr);

	void printf(const char * fmt, ...);

  void DrawGlyph(GFXglyph * glyph);
};

#endif /* GFXBASE_H_ */
