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
 *  file:     ledandkey.cpp
 *  brief:    8 Led + 8 Keys + 8 Seven Segment display support using TM1638 chip
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "ledandkey.h"

/*      01
 *    20  02
 *      40
 *    10  04
 *      08
 *          80 (dot)
 */

const unsigned char seg7_hexdigits[16] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

void TLedAndKey::Init()
{
	controller.Init();

	leds = 0;
	prevleds = 1;
	prevscancnt = 0xFFFF;
}

void TLedAndKey::Run()
{
	int i;

	prevkeys = keys;

	if (leds != prevleds)
	{
		// setting leds
		for (i = 0; i < 8; ++i)
		{
			controller.outregs[1 + (i << 1)] = (leds >> (7-i)) & 1;
		}
		prevleds = leds;
	}

	controller.Run();

	if (controller.scancounter != prevscancnt)
	{
		// process keys

		keys = 0;
		keys |= ((controller.inregs[0] & 1) << 7);
		keys |= ((controller.inregs[1] & 1) << 6);
		keys |= ((controller.inregs[2] & 1) << 5);
		keys |= ((controller.inregs[3] & 1) << 4);
		keys |= ((controller.inregs[0] & 16) >> 1);
		keys |= ((controller.inregs[1] & 16) >> 2);
		keys |= ((controller.inregs[2] & 16) >> 3);
		keys |= ((controller.inregs[3] & 16) >> 4);

		prevscancnt = controller.scancounter;
	}
}

void TLedAndKey::Set7Seg(int apos, int avalue)
{
	int i = ((7 - (apos & 7)) << 1);
	controller.outregs[i] = avalue;
}

void TLedAndKey::SetHexDigit(int apos, int avalue)
{
	Set7Seg(apos, seg7_hexdigits[avalue & 0x0F]);
}

void TLedAndKey::DisplayDirect(uint32_t avaluel, uint32_t avalueh)
{
  int i;
  unsigned v;
  v = avaluel;
  for (i = 0; i < 4; ++i)
  {
  	Set7Seg(i, v & 0xFF);
  	v = (v >> 8);
  }

  v = avalueh;
  for (i = 4; i < 8; ++i)
  {
  	Set7Seg(i, v & 0xFF);
  	v = (v >> 8);
  }
}

void TLedAndKey::DisplayHexNum(int avalue)
{
  int i;
  unsigned v = avalue;
  for (i = 0; i < 8; ++i)
  {
  	SetHexDigit(i, v & 0x0F);
  	v = (v >> 4);
  }
}

void TLedAndKey::DisplayDecNum(int avalue)
{
  int i;
  unsigned v = avalue;
  int m ;
  for (i = 0; i < 8; ++i)
  {
  	m = v % 10;
  	SetHexDigit(i, m);
  	v = v  / 10;
  }
}
