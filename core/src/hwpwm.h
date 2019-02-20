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
 *  file:     hwpwm.h
 *  brief:    Generic PWM Channel Definition
 *  version:  1.00
 *  date:     2019-02-15
 *  authors:  nvitya
*/

#ifndef HWPWM_H_PRE_
#define HWPWM_H_PRE_

#include "platform.h"

class THwPwmChannel_pre
{
public: // parameters

	uint32_t      frequency = 10000;

public:

	uint32_t      periodclocks = 0;
	uint32_t      cpu_clock_shifts = 0;

public: // utility
	uint8_t       devnum = 0;
	uint8_t       chnum = 0;
	uint8_t       outnum = 0;  // 0 = A, 1 = B

	bool          initialized = false;
};

#endif // ndef HWPWM_H_PRE_

#ifndef HWPWM_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWPWM_H_
#define HWPWM_H_

#include "mcu_impl.h"

#ifndef HWPWM_IMPL

class THwPwmChannel_noimpl : public THwPwmChannel_pre
{
public: // mandatory
	bool Init(int adevnum, int chnum)     { return false; }  // might vary from hw to hw

	void          SetOnClocks(uint16_t aclocks) { }
	void          Enable()  { }
	void          Disable() { }
	inline bool   Enabled() { return false; }
};

#define HWPWM_IMPL   THwPwmChannel_noimpl

#endif // ndef HWPWM_IMPL

//-----------------------------------------------------------------------------

class THwPwmChannel : public HWPWM_IMPL
{
public:

};

#endif /* HWPWM_H_ */

#else
  #undef HWPWM_PRE_ONLY
#endif
