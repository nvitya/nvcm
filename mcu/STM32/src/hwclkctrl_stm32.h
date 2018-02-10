// hwclkctrl_stm32.h

#ifndef HWCLKCTRL_STM32_H_
#define HWCLKCTRL_STM32_H_

#define HWCLKCTRL_PRE_ONLY
#include "hwclkctrl.h"

class THwClkCtrl_stm32 : public THwClkCtrl_pre
{
public:
	void StartExtOsc();
	bool ExtOscReady();

	void StartIntHSOsc();
	bool IntHSOscReady();

	void PrepareHiSpeed(unsigned acpuspeed);  // increase Flash wait states etc. for reliable high speed operation

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed);
};

#define HWCLKCTRL_IMPL THwClkCtrl_stm32

#endif // def HWCLKCTRL_STM32_H_
