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
 *  file:     hwpins.h
 *  brief:    Pin/Pad and GPIO configuration vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWPINS_H_PRE_
#define HWPINS_H_PRE_

#include "platform.h"

#define PORTNUM_A            0
#define PORTNUM_B            1
#define PORTNUM_C            2
#define PORTNUM_D            3
#define PORTNUM_E            4
#define PORTNUM_F            5
#define PORTNUM_G            6
#define PORTNUM_H            7
#define PORTNUM_I            8
#define PORTNUM_J            9
#define PORTNUM_K           10

#define PINCFG_INPUT         0x0000
#define PINCFG_OUTPUT        0x0001

#define PINCFG_OPENDRAIN     0x0002
#define PINCFG_PULLUP        0x0004
#define PINCFG_PULLDOWN      0x0008

#define PINCFG_ANALOGUE      0x0010

#define PINCFG_DRIVE_WEAK    0x0000
#define PINCFG_DRIVE_STRONG  0x0020

#define PINCFG_SPEED_MASK    0x0F00
#define PINCFG_SPEED_MEDIUM  0x0000
#define PINCFG_SPEED_SLOW    0x0100
#define PINCFG_SPEED_MED2    0x0200
#define PINCFG_SPEED_MEDIUM2 0x0200  // alias
#define PINCFG_SPEED_FAST    0x0300

#define PINCFG_GPIO_INIT_1   0x8000
#define PINCFG_GPIO_INIT_0   0x0000

#define PINCFG_AF_MASK     0x1F0000
#define PINCFG_AF_SHIFT    16
#define PINCFG_AF_0        0x100000
#define PINCFG_AF_1        0x110000
#define PINCFG_AF_2        0x120000
#define PINCFG_AF_3        0x130000
#define PINCFG_AF_4        0x140000
#define PINCFG_AF_5        0x150000
#define PINCFG_AF_6        0x160000
#define PINCFG_AF_7        0x170000
#define PINCFG_AF_8        0x180000
#define PINCFG_AF_9        0x190000
#define PINCFG_AF_10       0x1A0000
#define PINCFG_AF_11       0x1B0000
#define PINCFG_AF_12       0x1C0000
#define PINCFG_AF_13       0x1D0000
#define PINCFG_AF_14       0x1E0000
#define PINCFG_AF_15       0x1F0000

#define PINCFG_AF_A        0x100000  // Atmel uses ABC.. instead of 123..
#define PINCFG_AF_B        0x110000
#define PINCFG_AF_C        0x120000
#define PINCFG_AF_D        0x130000
#define PINCFG_AF_E        0x140000
#define PINCFG_AF_F        0x150000
#define PINCFG_AF_G        0x160000
#define PINCFG_AF_H        0x170000
#define PINCFG_AF_I        0x180000
#define PINCFG_AF_J        0x190000
#define PINCFG_AF_K        0x1A0000
#define PINCFG_AF_L        0x1B0000
#define PINCFG_AF_M        0x1C0000
#define PINCFG_AF_N        0x1D0000
#define PINCFG_AF_O        0x1E0000
#define PINCFG_AF_P        0x1F0000

class THwPinCtrl_pre
{
};

class TGpioPort_pre
{
public:
	int  portnum = -1;
};

class TGpioPin_pre
{
public:
	unsigned char    pinnum = 0xFF;
	unsigned char    portnum = 0xFF;
	bool             inverted = false;
};

// This suits almost all MCU families
class TGpioPin_common : public TGpioPin_pre
{
public:
	unsigned *       setbitptr = nullptr;
	unsigned *       clrbitptr = nullptr;
	unsigned *       getbitptr = nullptr;
	unsigned         setbitvalue = 0;
	unsigned         clrbitvalue = 0;
	unsigned         getbitshift = 0;

	inline void Set1()                 { *setbitptr = setbitvalue; }
	inline void Set0()                 { *clrbitptr = clrbitvalue; }
	inline void SetTo(unsigned value)  { if (value & 1) Set1(); else Set0(); }

	inline unsigned char Value()       { return ((*getbitptr >> getbitshift) & 1); }
};

#endif // ndef HWPINS_H_PRE_

#ifndef HWPINS_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWPINS_H_
#define HWPINS_H_

#include "mcu_impl.h"

#ifndef HWPINCTRL_IMPL

#warning "HWPINS is not implemented!"

class THwPinCtrl_noimpl : public THwPinCtrl_pre
{
public: // mandatory
	bool PinSetup(int aportnum, int apinnum, unsigned flags)  { return false; }
	void GpioSet(int aportnum, int apinnum, int value)  { }
	// for independent GPIO systems:
	bool GpioSetup(int aportnum, int apinnum, unsigned flags)  { return false; }

public: // optional
	// pin matrix setup for LPC8xx
	bool PinFuncConnect(int afuncid, int aportnum, int apinnum)  { return false; }
	// Input DAISY setup for IMXRT, see iomuxc_select_input_t for the function identifiers
	bool InputSelect(int afuncid, unsigned ainput)  { return false; }
};

class TGpioPort_noimpl : public TGpioPort_pre
{
public: // mandatory
	void Assign(int aportnum)  { }
	void Set(unsigned value)   { }
};

class TGpioPin_noimpl : public TGpioPin_pre
{
public: // mandatory
	void Assign(int aportnum, int apinnum, bool ainvert)  { }

	void Set0()  { }
	void Set1()  { }
	void SetTo(unsigned value)  { }
	void Toggle()  { }

	unsigned char Value() { return 0; }
	void SwitchDirection(int adirection)  { }
};

#define HWPINCTRL_IMPL   THwPinCtrl_noimpl
#define HWGPIOPORT_IMPL  TGpioPort_noimpl
#define HWGPIOPIN_IMPL   TGpioPin_noimpl

#endif // def HWCLKCTRL_IMPL

//-----------------------------------------------------------------------------

class THwPinCtrl : public HWPINCTRL_IMPL
{
};

class TGpioPort : public HWGPIOPORT_IMPL
{
public:
	TGpioPort(int aportnum)  { Assign(aportnum); }
};

class TGpioPin : public HWGPIOPIN_IMPL
{
public:
	TGpioPin(); // for empty constructor
	TGpioPin(int aportnum, int apinnum, bool ainvert);

	bool Assigned() { return pinnum != 0xFF; }

	bool Setup(unsigned flags);
};

// the global variable to handle the pins
extern THwPinCtrl hwpinctrl;

#endif // ndef HWPINS_H_

#else
  #undef HWPINS_PRE_ONLY
#endif
