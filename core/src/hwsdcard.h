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
 *  file:     hwsdcard.h
 *  brief:    Internal SDCARD Interface vendor-independent implementations
 *  version:  1.00
 *  date:     2018-06-07
 *  authors:  nvitya
*/

#ifndef HWSDCARD_H_PRE_
#define HWSDCARD_H_PRE_

#include "platform.h"
#include "hwdma.h"

#define SDCMD_RES_NO         0x0000
#define SDCMD_RES_48BIT      0x0001
#define SDCMD_RES_136BIT     0x0002
#define SDCMD_RES_R1B        0x0003
#define SDCMD_RES_MASK       0x0003

#define SDCMD_OPENDRAIN      0x0010

#define SD_SPECIAL_CMD_INIT       1

class THwSdcard_pre
{
public:
	int      devnum = -1;
	uint8_t  bus_width = 1;
	bool     high_speed = false;

	uint32_t clockspeed = 400000; // 20 MHz by default

	bool     initialized = false;

	uint32_t cmdtimeout = 0;
	bool     cmdrunning = false;
	bool     cmderror = false;
	uint32_t lastcmdtime = 0;

	uint32_t after_error_delay_clocks = 1;

};

#endif // ndef HWSDCARD_H_PRE_

#ifndef HWSDCARD_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWSDCARD_H_
#define HWSDCARD_H_

#include "mcu_impl.h"

#ifndef HWSDCARD_IMPL

class THwSdcard_noimpl : public THwSdcard_pre
{
public: // mandatory
	bool HwInit()        { return false; }

	void SetSpeed(uint32_t speed) { }
	void SetBusWidth(uint8_t abuswidth) { }

	void SendSpecialCmd(uint32_t aspecialcmd);
	void SendCmd(uint8_t acmd, uint32_t cmdarg, uint32_t cmdflags) { }
	bool CmdFinished() { return true; }

	uint32_t GetCmdResult32() { return 0; }
	void GetCmdResult128(void * adataptr) { }
};

#define HWSDCARD_IMPL   THwSdcard_noimpl

#endif // ndef HWSDCARD_IMPL

//-----------------------------------------------------------------------------

class THwSdcard : public HWSDCARD_IMPL
{
public:
	int         state = 0;

	bool        card_present = false;

	uint32_t    reg_ocr = 0;
	uint8_t     reg_cid[8];

  bool Init();

  void Run(); // operate the state machine
};

#endif /* HWSDCARD_H_ */

#else
  #undef HWSDCARD_PRE_ONLY
#endif
