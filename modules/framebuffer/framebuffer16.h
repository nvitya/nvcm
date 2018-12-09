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
 *  file:     framebuffer16.h
 *  brief:    16 bit/pixel framebuffer (usually used with integrated LCD controller)
 *  version:  1.00
 *  date:     2018-12-09
 *  authors:  nvitya
*/

#ifndef FRAMEBUFFER16_H_
#define FRAMEBUFFER16_H_

#include "platform.h"
#include "gfxbase.h"

class TFrameBuffer16 : public TGfxBase
{
public:
	typedef TGfxBase super;

	bool            initialized = false;

	uint16_t        aw_x = 0;
	uint16_t        aw_y = 0;

	uint16_t *      framebuffer = nullptr;

public:
	bool Init(uint16_t awidth, uint16_t aheight, void * abuf);

	virtual void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	virtual void DrawPixel(int16_t x, int16_t y, uint16_t color);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void SetAddrWindowStart(uint16_t x0, uint16_t y0);
	virtual void FillColor(uint16_t acolor, unsigned acount);

};

#endif /* FRAMEBUFFER16_H_ */
