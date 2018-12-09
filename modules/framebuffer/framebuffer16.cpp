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
 *  file:     framebuffer16.cpp
 *  brief:    16 bit/pixel framebuffer (usually used with integrated LCD controller)
 *  version:  1.00
 *  date:     2018-12-09
 *  authors:  nvitya
*/

#include "framebuffer16.h"


bool TFrameBuffer16::Init(uint16_t awidth, uint16_t aheight, void * abuf)
{
	InitGfx();

	hwwidth = awidth;
	hwheight = aheight;

	width = awidth;
	height = aheight;

	initialized = false;

	framebuffer = (uint16_t *)abuf;

	if (!framebuffer)
	{
		return false;
	}

#if 0
	if (!InitInterface())
	{
		return false;
	}
#endif

	initialized = true;
	return true;
}

void TFrameBuffer16::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	uint16_t endx = x + w;
	uint16_t endy = y + h;
	uint16_t * pp;
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
		pp = framebuffer + ly * hwwidth + lx;
		while (lx < endx)
		{
			*(pp++) = color;
			++lx;
		}
		++ly;
	}
}

void TFrameBuffer16::DrawPixel(int16_t x, int16_t y, uint16_t color)
{
	uint16_t * pp = framebuffer + y * hwwidth + x;
	*pp = color;
}

void TFrameBuffer16::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
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

void TFrameBuffer16::SetAddrWindowStart(uint16_t x0, uint16_t y0)
{
	aw_x0 = x0;
	aw_y0 = y0;
	aw_x = x0;
	aw_y = y0;
}

void TFrameBuffer16::FillColor(uint16_t acolor, unsigned acount)
{
	uint16_t * pp = framebuffer + aw_y * hwwidth + aw_x;
	while (acount > 0)
	{
		*(pp++) = acolor;

		++aw_x;
		if (aw_x > aw_x1)
		{
			aw_x = aw_x0;
			++aw_y;
			if (aw_y > aw_y1)  aw_y = aw_y0;

			pp = framebuffer + aw_y * hwwidth + aw_x;
		}
		--acount;
	}
}
