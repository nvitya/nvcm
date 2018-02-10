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
 *  file:     ledandkey.h
 *  brief:    8 Led + 8 Keys + 8 Seven Segment display support using TM1638 chip
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef SRC_LEDANDKEY_H_
#define SRC_LEDANDKEY_H_

#include "tm1638.h"

class TLedAndKey
{
public:
	Ttm1638        controller;

	unsigned char  leds;
	unsigned char  keys;
	unsigned char  prevkeys;

	void      Init();
	void      Run();

	void Set7Seg(int apos, int avalue);
	void SetHexDigit(int apos, int avalue);

	void DisplayHexNum(int avalue);
	void DisplayDecNum(int avalue);

protected:
	unsigned char  prevleds;
	unsigned       prevscancnt;

};

#endif /* SRC_LEDANDKEY_H_ */
