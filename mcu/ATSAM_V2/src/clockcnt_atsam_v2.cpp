// clockcnt_stm32.cpp

#include "platform.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
	PM->APBCMASK.reg |= (1 << 6);  // ENABLE TC1 clock
	GCLK->CLKCTRL.reg = 0x4012;    // Select main clock as CLOCK source

	TC1->COUNT32.CTRLA.reg = 0
		| (1 << 11)  // run in standby
		| (0 <<  8)  // prescaler
		| (2 <<  2)  // 32 bit mode
		| (1 <<  1)  // enable
	;


/*
	TC1->COUNT32.CTRLA.bit.ENABLE = 0; // disable the timer
	//TC1->COUNT32.CTRLA.bit.SWRST = 1; // reset
	//while (TC1->COUNT32.CTRLA.bit.SWRST) { } // wait until reset
	TC1->COUNT32.CTRLA.bit.MODE = 2;
	TC1->COUNT32.CTRLA.bit.PRESCALER = 0;
  TC1->COUNT32.CTRLA.bit.ENABLE = 1; // enable it
*/

	while (!TC1->COUNT32.CTRLA.bit.ENABLE) {}
}

#endif
