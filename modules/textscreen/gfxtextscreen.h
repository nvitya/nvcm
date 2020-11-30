// gfxtextscreen.h

#ifndef GFXTEXTSCREEN_H_
#define GFXTEXTSCREEN_H_

#include "gfxbase.h"
#include "textscreen.h"

class TGfxTextScreen : public TTextScreen
{
public:

	TGfxBase *	  	disp;
	TGfxFont *      font;

	uint16_t        disp_x;
	uint16_t        disp_y;
	uint16_t        disp_w;
	uint16_t        disp_h;

	uint8_t         charwidth;

	uint8_t         lineheight;

	virtual         ~TGfxTextScreen() { }

	void            InitTextGfx(TGfxBase * adisp, uint16_t x, uint16_t y, uint16_t w, uint16_t h, TGfxFont * amonofont);

public: // textscreen mandatory

	virtual void    DrawChar(unsigned acol, unsigned arow, char ach);
	virtual void    SetCursor();

protected:
	uint16_t        cursor_prev_x = 0;
	uint16_t        cursor_prev_y = 0;
	uint16_t        cursor_prev_on = 0;
	bool            cursor_blink_on = false;

};

#endif /* GFXTEXTSCREEN_H_ */
