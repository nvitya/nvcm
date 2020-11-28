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
	unsigned saddr = arow * cols + acol;
	disp->SetCursor(disp_x + acol * charwidth, disp_y + lineheight * arow);
	TGfxGlyph * glyph = font->GetGlyph(screenbuf[saddr]);
	if (!glyph)  glyph = font->GetGlyph('.');  // replace non-existing characters with dot
	disp->DrawGlyph(font, glyph);
}

void TGfxTextScreen::SetCursor()
{
	// not implemented yet
}
