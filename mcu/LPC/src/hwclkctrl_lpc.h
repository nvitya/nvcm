// hwclkctrl_lpc.h

#ifndef HWCLKCTRL_LPC_H_
#define HWCLKCTRL_LPC_H_

#define HWCLKCTRL_PRE_ONLY
#include "hwclkctrl.h"

class THwClkCtrl_lpc : public THwClkCtrl_pre
{
public:
	void StartExtOsc();
	bool ExtOscReady();

	void StartIntHSOsc();
	bool IntHSOscReady();

	void PrepareHiSpeed(unsigned acpuspeed);  // increase Flash wait states etc. for reliable high speed operation

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed);
};

#define HWCLKCTRL_IMPL THwClkCtrl_lpc

#endif // def HWCLKCTRL_LPC_H_
