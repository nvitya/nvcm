// gpio.cpp

#include "platform.h"
#include "hwpins.h"

#define PIOA_BASE  (0x400E0E00U)
#define PIOA_PIOB_DISTANCE  (0x200)

#if defined(PIOE)
  #define MAX_PORT_NUMBER 5
#elif defined(PIOD)
  #define MAX_PORT_NUMBER 4
#elif defined(PIOC)
  #define MAX_PORT_NUMBER 3
#else
  #define MAX_PORT_NUMBER 2
#endif


HW_GPIO_REGS * THwPinCtrl_atsam::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}
	else
	{
		return (HW_GPIO_REGS *)(PIOA_BASE + PIOA_PIOB_DISTANCE*aportnum);
	}
}


bool THwPinCtrl_atsam::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	// Atmel version

	HW_GPIO_REGS * regs = GetGpioRegs(aportnum);
	if (!regs)
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 31))
	{
		return false;
	}

	// 1. turn on port power
	GpioPortEnable(aportnum);

	unsigned pinbit = (1 << apinnum);

	if (flags & PINCFG_AF_MASK)
	{
		// disable PIO
		regs->PIO_PDR = pinbit;

		unsigned afnum = (((flags & PINCFG_AF_MASK) >> PINCFG_AF_SHIFT) & 3);

#ifdef REG_PIOA_ABCDSR
		// 4 alternate function selection possibility
		if (afnum & 1)
		{
			regs->PIO_ABCDSR[0] |= pinbit;
		}
		else
		{
			regs->PIO_ABCDSR[0] &= ~pinbit;
		}

		if (afnum & 2)
		{
			regs->PIO_ABCDSR[1] |= pinbit;
		}
		else
		{
			regs->PIO_ABCDSR[1] &= ~pinbit;
		}
#else
		// 2 alternate function selection possibility
		if (afnum & 1)
		{
			regs->PIO_ABSR |= pinbit;
		}
		else
		{
			regs->PIO_ABSR &= ~pinbit;
		}
#endif

	}
	else
	{
		// GPIO
		regs->PIO_PER = pinbit;
		//regs->PIO_IFDR = pinbit;  // disable input filter

		if (flags & PINCFG_OUTPUT)
		{
			regs->PIO_OER = pinbit;
		}
		else
		{
			regs->PIO_ODR = pinbit;
		}
	}

  // 3. set open-drain
  if (flags & PINCFG_OPENDRAIN)
  {
  	regs->PIO_MDER = pinbit;
  }
  else
  {
  	regs->PIO_MDDR = pinbit;
  }

  // 4. set pullup / pulldown
  if (flags & PINCFG_PULLUP)
  {
    regs->PIO_PUER = pinbit;
  }
  else
  {
    regs->PIO_PUDR = pinbit;
  }

  // 5. set speed
  // no speed options for SAM3

  // 6. initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	regs->PIO_SODR = pinbit;
  }
  else
  {
  	regs->PIO_CODR = pinbit;
  }

  return true;
}

bool THwPinCtrl_atsam::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

	PMC->PMC_PCER0 = (1 << (11 + aportnum));

  return true;
}

void THwPinCtrl_atsam::GpioSet(int aportnum, int apinnum, int value)
{
	HW_GPIO_REGS * regs = (HW_GPIO_REGS *)(PIOA_BASE + PIOA_PIOB_DISTANCE*aportnum);

	unsigned pinbit = (1 << apinnum);

  if (1 == value)
  {
  	regs->PIO_SODR = pinbit;
  }
  else if (value & 2) // toggle
  {
  	if (regs->PIO_ODSR & pinbit)
  	{
    	regs->PIO_CODR = pinbit;
  	}
  	else
  	{
    	regs->PIO_SODR = pinbit;
  	}
  }
  else
  {
  	regs->PIO_CODR = pinbit;
  }
}

// GPIO Port

void TGpioPort_atsam::Assign(int aportnum)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
}

void TGpioPort_atsam::Set(unsigned value)
{
	regs->PIO_ODSR = value;
}

// GPIO Pin

void TGpioPin_atsam::Assign(int aportnum, int apinnum, bool ainvert)
{
	portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return;
	}

	setbitvalue = (1 << pinnum);
	clrbitvalue = setbitvalue;
  getbitptr = (unsigned *)&(regs->PIO_PDSR);
  getbitshift = pinnum;

  if (ainvert)
  {
    setbitptr = (unsigned *)&(regs->PIO_CODR);
    clrbitptr = (unsigned *)&(regs->PIO_SODR);
  }
  else
  {
    setbitptr = (unsigned *)&(regs->PIO_SODR);
    clrbitptr = (unsigned *)&(regs->PIO_CODR);
  }
}

void TGpioPin_atsam::Toggle()
{
	if (regs->PIO_ODSR & setbitvalue)
	{
  	regs->PIO_CODR = setbitvalue;
	}
	else
	{
  	regs->PIO_SODR = setbitvalue;
	}
}

void TGpioPin_atsam::SwitchDirection(int adirection)
{
	if (adirection)
	{
		regs->PIO_OER = setbitvalue;
	}
	else
	{
		regs->PIO_ODR = setbitvalue;
	}
}

