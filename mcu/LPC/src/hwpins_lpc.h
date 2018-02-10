// hwpins_lpc.h

#ifndef _HWPINS_LPC_H
#define _HWPINS_LPC_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

class THwPinCtrl_lpc : public THwPinCtrl_pre
{
public:
	// platform specific
	bool PinSetup(int aportnum, int apinnum, unsigned flags);

	HW_GPIO_REGS * GetGpioRegs(int aportnum);

	bool GpioPortEnable();

	void GpioSet(int aportnum, int apinnum, int value);

	bool GpioSetup(int aportnum, int apinnum, unsigned flags);
};

class TGpioPort_lpc : public TGpioPort_pre
{
public:
	void Assign(int aportnum);
	void Set(unsigned value);

public:
	volatile unsigned * portptr = nullptr;
};

class TGpioPin_lpc : public TGpioPin_common
{
public:
	unsigned *       togglebitptr = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	inline void Toggle()  { *togglebitptr = setbitvalue; }

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_lpc
#define HWGPIOPORT_IMPL  TGpioPort_lpc
#define HWGPIOPIN_IMPL   TGpioPin_lpc

#endif /* HWPINS_H_ */
