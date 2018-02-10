/*
 * lpc_utils.h
 *
 *  Created on: Dec 13, 2017
 *      Author: nagy
 */

#ifndef LPC_UTILS_H_
#define LPC_UTILS_H_

#include "platform.h"

void lpc_enable_clock(unsigned index, unsigned value);
void lpc_disable_clock(unsigned index, unsigned value);

void lpc_set_clock_divider(CHIP_CGU_IDIV_T Divider, CHIP_CGU_CLKIN_T Input, uint32_t Divisor);
void lpc_set_base_clock(CHIP_CGU_BASE_CLK_T BaseClock, CHIP_CGU_CLKIN_T Input, bool autoblocken, bool powerdn);

#endif
