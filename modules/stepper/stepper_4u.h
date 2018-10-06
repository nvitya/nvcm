/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
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
 *  file:     stepper_4u.h
 *  brief:    Simple 4 phase stepper motor controller module
 *  version:  1.00
 *  date:     2018-09-29
 *  authors:  nvitya
 *  notes:
 *    tested with 28BYJ-48
*/

#ifndef STEPPER_4U_H_
#define STEPPER_4U_H_

#include "hwpins.h"

class TStepper_4u
{
public:
	TGpioPin  pin[4]; // must be assigned before calling init

	int       position = 0;     // target position
	int       actual_pos = 0;

  uint32_t  step_time_us = 5000;  // the 28BYJ worked with this, still quite slow

	bool Init();

	void Run(); // state machine, follows the position

public: // internal state
  int       state = 0;
  uint32_t  starttime = 0;

  uint8_t   ctrl_idx = 0;
  uint8_t   ctrl_pattern[16];
  uint8_t   ctrl_len = 0;

  uint32_t  step_time_clocks;

	void SetCtrlPattern(uint8_t apat);
};

#endif /* STEPPER_4U_H_ */
