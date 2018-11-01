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
 *  file:     timedisp_tm1637.cpp
 *  brief:    4 x seven segment display with ":" using TM1637 chip
 *  version:  1.00
 *  date:     2018-11-01
 *  authors:  nvitya
*/

#include "timedisp_tm1637.h"

/*      01
 *    20  02
 *      40
 *    10  04
 *      08
 *          80 (dot)
 */

static const unsigned char seg7_hexdigits[16] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

void TTimeDisp_tm1637::Init()
{
	controller.Init();

	prevscancnt = 0xFFFF;
}

void TTimeDisp_tm1637::Run()
{
	int i;

	controller.Run();

	if (controller.scancounter != prevscancnt)
	{
		// no input here

		prevscancnt = controller.scancounter;
	}
}

void TTimeDisp_tm1637::Set7Seg(int apos, int avalue)
{
	int i = (3 - (apos & 3));
	controller.outregs[i] = avalue;

	if (i == 1)
	{
		if (colon_on) controller.outregs[1] |=  0x80;
		else          controller.outregs[1] &= ~0x80;
	}

}

void TTimeDisp_tm1637::SetHexDigit(int apos, int avalue)
{
	Set7Seg(apos, seg7_hexdigits[avalue & 0x0F]);
}

void TTimeDisp_tm1637::DisplayDirect(uint32_t avaluel)
{
  int i;
  unsigned v;
  v = avaluel;
  for (i = 0; i < 4; ++i)
  {
  	colon_on = ((avaluel & (1 << 15)) != 0);
  	Set7Seg(i, v & 0xFF);
  	v = (v >> 8);
  }
}

void TTimeDisp_tm1637::DisplayHexNum(int avalue)
{
  int i;
  unsigned v = avalue;
  for (i = 0; i < 4; ++i)
  {
  	SetHexDigit(i, v & 0x0F);
  	v = (v >> 4);
  }
}

void TTimeDisp_tm1637::DisplayDecNum(int avalue)
{
  int i;
  unsigned v = avalue;
  int m ;
  for (i = 0; i < 4; ++i)
  {
  	m = v % 10;
  	SetHexDigit(i, m);
  	v = v  / 10;
  }
}
