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
 *  file:     timedisp_tm1637.h
 *  brief:    4 x seven segment display with ":" using TM1637 chip
 *  version:  1.00
 *  date:     2018-11-01
 *  authors:  nvitya
*/

#ifndef TIMEDISP_TM1637_H_
#define TIMEDISP_TM1637_H_

#include "tm1637.h"

class TTimeDisp_tm1637
{
public:
	Ttm1637        controller;

	bool           colon_on = false;

	void      Init();
	void      Run();

	void Set7Seg(int apos, int avalue);
	void SetHexDigit(int apos, int avalue);

	void DisplayDirect(uint32_t avaluel);  // sets every segment one by one
	void DisplayHexNum(int avalue);
	void DisplayDecNum(int avalue);

protected:
	unsigned       prevscancnt;

};

#endif /* TIMEDISP_TM1637_H_ */
