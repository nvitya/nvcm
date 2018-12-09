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
 *  file:     hwlcdctrl.h
 *  brief:    A minimal interface to the integrated LCD controller
 *  version:  1.00
 *  date:     2018-12-09
 *  authors:  nvitya
*/

#ifndef _HWLCDCTRL_H_PRE_
#define _HWLCDCTRL_H_PRE_

#include "platform.h"
#include "hwpins.h"

class THwLcdCtrl_pre
{
public:
	uint16_t         hsync   = 41;
	uint16_t         hbp     = 13;
	uint16_t         hfp     = 32;

	uint16_t         vsync   = 10;
	uint16_t         vbp     =  2;
	uint16_t         vfp     =  2;

	uint16_t         hwwidth  = 480;
	uint16_t         hwheight = 272;

	uint8_t *        framebuffer = nullptr;
};

#endif // ndef _HWLCDCTRL_H_PRE_

#ifndef HWLCDCTRL_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWLCDCTRL_H_
#define HWLCDCTRL_H_

#include "mcu_impl.h"

#ifndef HWLCDCTRL_IMPL

class THwLcdCtrl_noimpl : public THwLcdCtrl_pre
{
public: // mandatory
	bool Init(uint16_t awidth, uint16_t aheight, void * aframebuffer)  { return false; }
};

#define HWLCDCTRL_IMPL   THwLcdCtrl_noimpl

#endif // ndef HWLCDCTRL_IMPL

//-----------------------------------------------------------------------------

class THwLcdCtrl : public HWLCDCTRL_IMPL
{

};

#endif // HWLCDCTRL_H_

#else
  #undef HWLCDCTRL_PRE_ONLY
#endif

