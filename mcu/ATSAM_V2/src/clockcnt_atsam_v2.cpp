// clockcnt_stm32.cpp

#include "platform.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
#if defined(MCLK_APBCMASK_TC1)

	// in 32bit mode TC0 and TC1 are used together

	MCLK->APBCMASK.reg |= (MCLK_APBCMASK_TC0 | MCLK_APBCMASK_TC1);   // ENABLE TC0 + TC1 clock
	GCLK->PCHCTRL[TC0_GCLK_ID].reg = (1 << 6) | (0 << 0);  // Enable the peripheral and select GEN0

#elif defined(PM_APBCMASK_TC1)

	PM->APBCMASK.reg |= (PM_APBCMASK_TC0 | PM_APBCMASK_TC1);
	GCLK->CLKCTRL.reg = 0x4012;    // Select main clock as CLOCK source

#endif

	TC0->COUNT32.CTRLA.reg = 0
		| (1 <<  6)  // run in standby
		| (0 <<  8)  // prescaler
		| (2 <<  2)  // 32 bit mode
		| (1 <<  1)  // enable
	;

	while (!TC0->COUNT32.CTRLA.bit.ENABLE) {}
}

uint32_t atsam_v2_clockcnt_read()
{
	TC0->COUNT32.CTRLBSET.reg = (4 << 5);  // issue sync command
	while (TC0->COUNT32.SYNCBUSY.bit.COUNT) { }
	return TC0->COUNT32.COUNT.reg;
}

#endif

