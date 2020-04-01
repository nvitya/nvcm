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
 *  file:     hwextirq.h
 *  brief:    External Pin Interrupt vendor-independent definitions
 *  version:  1.00
 *  date:     2020-04-01
 *  authors:  nvitya
*/

#ifndef _HWEXTIRQ_H_PRE_
#define _HWEXTIRQ_H_PRE_

#include "platform.h"
#include "hwpins.h"
#include "errors.h"

#define HWEXTIRQ_RISING    1
#define HWEXTIRQ_FALLING   2

class THwExtIrq_pre
{
public:	// settings
};

#endif // ndef HWEXTIRQ_H_PRE_

#ifndef HWEXTIRQ_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWEXTIRQ_H_
#define HWEXTIRQ_H_

#include "mcu_impl.h"

#ifndef HWEXTIRQ_IMPL

//#warning "HWEXTIRQ is not implemented!"

class THwExtIrq_noimpl : public THwExtIrq_pre
{
public: // mandatory
	bool Init(int aportnum, int apinnum, unsigned aflags)   { return false; }

	void IrqBegin()   {  }  // ATSAM MCU-s require this (but they don't require IrqAck in return)
	bool IrqPending() { return false; }
	void IrqAck()     {  }
	void Enable()     {  }
	void Disable()    {  }
};

#define HWEXTIRQ_IMPL   THwExtIrq_noimpl

#endif // ndef HWEXTIRQ_IMPL

//-----------------------------------------------------------------------------

class THwExtIrq : public HWEXTIRQ_IMPL
{
public:
};

#endif // HWEXTIRQ_H_

#else
  #undef HWEXTIRQ_PRE_ONLY
#endif
