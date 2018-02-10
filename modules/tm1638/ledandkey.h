/*
 * ledandkey.h
 *
 *  Created on: 2017. máj. 1.
 *      Author: vitya
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
