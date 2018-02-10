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
 *  file:     textscreen.h
 *  brief:    Implementing text console on a TFT display
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
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
