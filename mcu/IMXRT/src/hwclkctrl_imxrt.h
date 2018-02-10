// hwclkctrl_imxrt.h

#ifndef HWCLKCTRL_IMXRT_H_
#define HWCLKCTRL_IMXRT_H_

#define HWCLKCTRL_PRE_ONLY
#include "hwclkctrl.h"

class THwClkCtrl_imxrt : public THwClkCtrl_pre
{
public:
	void StartExtOsc();
	bool ExtOscReady();

	void StartIntHSOsc();
	bool IntHSOscReady();

	void PrepareHiSpeed(unsigned acpuspeed);  // increase Flash wait states etc. for reliable high speed operation

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed);
};

#define HWCLKCTRL_IMPL THwClkCtrl_imxrt

#endif // def HWCLKCTRL_IMXRT_H_
