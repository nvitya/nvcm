/*
 * gfxtextscreen.cpp
 *
 *  Created on: 27 Nov 2020
 *      Author: vitya
 */

#include <gfxtextscreen.h>

void TGfxTextScreen::InitTextGfx(TGfxBase * adisp, uint16_t x, uint16_t y, uint16_t w, uint16_t h, TGfxFont * amonofont)
{
	// the buffers must be already initialized !

	if (!screenbuf || (buf_size == 0))
	{
		return;
	}

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

	// smaller buffer was given, reduce the cols / rows
	while (cols * rows > buf_size)
	{
		if (cols > rows)
		{
			--cols;
		}
		else
		{
			--rows;
		}
	}

	// initial clear
	disp->FillRect(disp_x, disp_y, disp_w, disp_h, disp->bgcolor);
}

void TGfxTextScreen::DrawChar(unsigned acol, unsigned arow, char ach)
{
	unsigned saddr = AddrFromColRow(acol, arow);
	disp->SetCursor(disp_x + acol * charwidth, disp_y + lineheight * arow);
	TGfxGlyph * glyph = font->GetGlyph(screenbuf[saddr]);
	if (!glyph)  glyph = font->GetGlyph('.');  // replace non-existing characters with dot

	uint16_t orig_bgcolor = disp->bgcolor;
	uint16_t orig_color = disp->color;

	if (cursor_on && (acol == cursor_x) && (arow == cursor_y))
	{
		disp->bgcolor = orig_color;
		disp->color   = orig_bgcolor;
	}

	disp->DrawGlyph(font, glyph);

	disp->color   = orig_color;
	disp->bgcolor = orig_bgcolor;
}

void TGfxTextScreen::SetCursor()
{
	if ((cursor_x != cursor_prev_x) || (cursor_y != cursor_prev_y))
	{
		unsigned addr;

		addr = AddrFromColRow(cursor_prev_x, cursor_prev_y);
		changemap[addr >> 5] |= (1 << (addr & 31));

		addr = AddrFromColRow(cursor_x, cursor_y);
		changemap[addr >> 5] |= (1 << (addr & 31));

		screenchanged = true;

		cursor_prev_x = cursor_x;
		cursor_prev_y = cursor_y;
	}
}
