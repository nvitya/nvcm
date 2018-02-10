// hwpins_imxrt.h

#ifndef _HWPINS_IMXRT_H
#define _HWPINS_IMXRT_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

#include "imxrt_pads.h"
#include "imxrt_iomuxc.h"

#define MAX_GPIO_PORT_NUMBER  5

class THwPinCtrl_imxrt : public THwPinCtrl_pre
{
public:
	unsigned gpio_output_shadow[MAX_GPIO_PORT_NUMBER];

	void     UpdateGpioOutputShadow();

public:
	// IMXRT specific, best used with the definitions in imxrt_pads.h
	bool PadSetup(int padnum, unsigned flags);

	// IMXRT special for the defines in the imxrt_iomuxc.h, do not use for GPIO
	void PadSetup(uint32_t muxreg, uint8_t muxmode, uint32_t inputreg, uint8_t daisy, uint32_t confreg, unsigned flags);

	void SetPadControl(unsigned * padctrlreg, unsigned flags);

	bool InputSelect(int afuncid, unsigned ainput); // IMXRT Specific for INPUT DAISY registers

	HW_GPIO_REGS * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	bool GpioSetup(int aportnum, int apinnum, unsigned flags);
};

class TGpioPort_imxrt : public TGpioPort_pre
{
public:
	HW_GPIO_REGS *   regs = nullptr;
	unsigned *       drshadow;

	void Assign(int aportnum);
	void Set(unsigned value);

public:
	volatile unsigned * portptr = nullptr;
};

class TGpioPin_imxrt : public TGpioPin_pre
{
public:

	HW_GPIO_REGS *   regs = nullptr;

	unsigned *       drreal;
	unsigned *       drshadow;

	unsigned *       getbitptr = nullptr;
	unsigned         setbitvalue = 0;
	unsigned         clrbitvalue = 0;
	unsigned         getbitshift = 0;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	void Set1();
	void Set0();
	inline void SetTo(unsigned value)  { if (value & 1)  Set1(); else Set0(); }

	inline unsigned char Value()  { return ((*getbitptr >> getbitshift) & 1); }

	inline void Toggle();

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_imxrt
#define HWGPIOPORT_IMPL  TGpioPort_imxrt
#define HWGPIOPIN_IMPL   TGpioPin_imxrt

#endif /* HWPINS_IMXRT_H_ */
