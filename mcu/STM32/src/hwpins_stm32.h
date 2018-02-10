// hwpins_stm32.h

#ifndef _HWPINS_STM32_H
#define _HWPINS_STM32_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

class THwPinCtrl_stm32 : public THwPinCtrl_pre
{
public:
	// platform specific
	bool PinSetup(int aportnum, int apinnum, unsigned flags);

	GPIO_TypeDef * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	inline bool GpioSetup(int aportnum, int apinnum, unsigned flags)  { return PinSetup(aportnum, apinnum, flags); }
};

class TGpioPort_stm32 : public TGpioPort_pre
{
public:
	void Assign(int aportnum);
	void Set(unsigned value);

public:
	GPIO_TypeDef *       regs = nullptr;
	volatile unsigned *  portptr = nullptr;
};

class TGpioPin_stm32 : public TGpioPin_common
{
public:
	GPIO_TypeDef *   regs = nullptr;
	unsigned *       togglebitptr = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	void Toggle();

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_stm32
#define HWGPIOPORT_IMPL  TGpioPort_stm32
#define HWGPIOPIN_IMPL   TGpioPin_stm32

#endif /* HWPINS_STM32_H_ */
