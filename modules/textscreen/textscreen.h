// textscreen.h

#ifndef TEXTSCREEN_H_
#define TEXTSCREEN_H_

#include "stdint.h"

class TTextScreen
{
protected:
	// row[0] is always at the bottom
	uint8_t *       screenbuf = nullptr;
	uint32_t *      changemap = nullptr;
	bool            screenchanged = false;

	unsigned        buf_size = 0;

	void            SetScreenBufChar(unsigned aaddr, char ach);

public:
	virtual ~TTextScreen() { }

	unsigned 				cols = 0;
	unsigned 				rows = 0;

	unsigned        cposx;
	unsigned        cposy;

	bool            cursor_on = true;
	unsigned        cursor_x = 0;
	unsigned        cursor_y = 0;

	void            InitCharBuf(unsigned acols, unsigned arows, uint8_t * ascreenbuf, uint8_t * achangemap);

	void            WriteChar(char ach);

	void            SetPos(unsigned acol, unsigned arow);

	void            printf(const char * fmt, ...);

	void            Refresh();

	void            Clear();

	virtual void    Run();

public: // implemented in the child

	virtual void    DrawChar(unsigned acol, unsigned arow, char ach);
	virtual void    SetCursor();
};

#endif /* TEXTSCREEN_H_ */
