/*
 * ledandkey.cpp
 *
 *  Created on: 2017. m�j. 1.
 *      Author: vitya
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