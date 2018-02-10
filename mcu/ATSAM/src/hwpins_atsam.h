// hwpins_atsam.h

#ifndef _HWPINS_ATSAM_H
#define _HWPINS_ATSAM_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

class THwPinCtrl_atsam : public THwPinCtrl_pre
{
public:
	// platform specific
	bool PinSetup(int aportnum, int apinnum, unsigned flags);

	HW_GPIO_REGS * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	inline bool GpioSetup(int aportnum, int apinnum, unsigned flags)  { return PinSetup(aportnum, apinnum, flags); }
};

class TGpioPort_atsam : public TGpioPort_pre
{
public:
	void Assign(int aportnum);
	void Set(unsigned value);

public:
	HW_GPIO_REGS *       regs = nullptr;
	volatile unsigned *  portptr = nullptr;
};

class TGpioPin_atsam : public TGpioPin_common
{
public:
	HW_GPIO_REGS *   regs = nullptr;
	unsigned *       togglebitptr = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	void Toggle();

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_atsam
#define HWGPIOPORT_IMPL  TGpioPort_atsam
#define HWGPIOPIN_IMPL   TGpioPin_atsam

#endif /* HWPINS_ATSAM_H_ */
