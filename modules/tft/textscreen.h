/*
 * textscreen.h
 *
 *  Created on: 2017. szept. 22.
 *      Author: vitya
 */

#ifndef TEXTSCREEN_H_
#define TEXTSCREEN_H_

#include "inttypes.h"
#include "tftlcd.h"

// max supported screen: 480 * 320
#define TEXTSCREEN_BUF_SIZE  (80*40)
// it is quite big for the cortex-m devices: 3.2 kByte!

// we use fix character size: (5 + 1) x 8
// we add an extra line spacing

class TTextScreen
{
protected:
	// row[0] is always at the bottom
	char      screenbuf[TEXTSCREEN_BUF_SIZE];
	uint8_t   changemap[TEXTSCREEN_BUF_SIZE >> 3];
	bool      screenchanged;

	void      SetScreenBufChar(uint16_t aaddr, char ach);

public:

	TTftLcd *		  	lcd;

	uint8_t         xoffs;
	uint8_t         yoffs;
	uint8_t  				cols;
	uint8_t  				rows;

	uint8_t         lineheight;

	uint8_t         curcol;

	void Init(TTftLcd * alcd);

	void WriteChar(char ach);

	void printf(const char * fmt, ...);

	void Update();
};

#endif /* TEXTSCREEN_H_ */
