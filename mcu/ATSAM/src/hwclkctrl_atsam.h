// hwclkctrl_atsam.h

#ifndef HWCLKCTRL_ATSAM_H_
#define HWCLKCTRL_ATSAM_H_

#define HWCLKCTRL_PRE_ONLY
#include "hwclkctrl.h"

class THwClkCtrl_atsam : public THwClkCtrl_pre
{
public:
	void StartExtOsc();
	bool ExtOscReady();

	void StartIntHSOsc();
	bool IntHSOscReady();

	void PrepareHiSpeed(unsigned acpuspeed);  // increase Flash wait states etc. for reliable high speed operation

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed);
};

#define HWCLKCTRL_IMPL THwClkCtrl_atsam

#endif // def HWCLKCTRL_ATSAM_H_
