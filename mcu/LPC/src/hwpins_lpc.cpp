// hwpins_lpc.cpp

#include "platform.h"
#include "hwpins.h"

#if defined(MCUSF_43XX)
	#define MAX_PORT_NUMBER        15
	#define MAX_GPIO_PORT_NUMBER    8
#else
  #error "Unknown subfamily"
#endif

HW_GPIO_REGS * THwPinCtrl_lpc::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_GPIO_PORT_NUMBER))
	{
		return nullptr;
	}

	return (HW_GPIO_REGS *)LPC_GPIO_PORT_BASE;
}

/* PinSetup for LPC:
 *
 * aportnum = 0..15: normal port pins
 * aportnum = 256: CLK0..3 pins
*/
bool THwPinCtrl_lpc::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	// LPC43xx version

	if (0x100 == aportnum)
	{
		// Pin setup for CLK0..3 pins

		if ((apinnum < 0) || (apinnum > 3))
		{
			return false;
		}
	}
	else
	{
		// normal ports

		if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
		{
			return false;
		}

		if ((apinnum < 0) || (apinnum > 31))
		{
			return false;
		}

		GpioPortEnable();
	}

	unsigned n = 0;

	if (flags & PINCFG_PULLDOWN)
	{
		n |= (1 << 3);
	}
	if (flags & PINCFG_PULLUP)
	{
		n |= (1 << 4);
	}
	if (flags & PINCFG_SPEED_FAST)
	{
		n |= (1 << 5);
		n |= (1 << 7); // disable input glitch filter
	}

	n |= (1 << 6); // always enable the input buffer

	if (flags & PINCFG_DRIVE_STRONG)
	{
		n |= (3 << 8);
	}

	if (flags & PINCFG_AF_MASK)
	{
		n |= ((flags >> PINCFG_AF_SHIFT) & 0x07);
	}

	if (0x100 == aportnum)
	{
		LPC_SCU->SFSCLK[apinnum] = n;
	}
	else
	{
		LPC_SCU->SFSP[aportnum][apinnum] = n;
	}

	return true;
}

bool THwPinCtrl_lpc::GpioSetup(int aportnum, int apinnum, unsigned flags)
{
	if ((aportnum < 0) || (aportnum >= MAX_GPIO_PORT_NUMBER))
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 31))
	{
		return false;
	}

	unsigned bm = (1 << apinnum);

	// set GPIO direction

	if (flags & PINCFG_OUTPUT)
	{
		LPC_GPIO_PORT->DIR[aportnum] |= bm;
	}
	else
	{
		LPC_GPIO_PORT->DIR[aportnum] &= ~bm;
	}

  // initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	LPC_GPIO_PORT->SET[aportnum] = bm;
  }
  else
  {
  	LPC_GPIO_PORT->CLR[aportnum] = bm;
  }

  return true;
}

bool THwPinCtrl_lpc::GpioPortEnable()
{
	LPC_CCU1->CLKCCU[CLK_MX_GPIO].CFG |= 1;
  return true;
}

void THwPinCtrl_lpc::GpioSet(int aportnum, int apinnum, int value)
{
	unsigned bm = (1 << apinnum);

  if (1 == value)
  {
  	LPC_GPIO_PORT->SET[aportnum] = bm;
  }
  else if (value & 2) // toggle
  {
  	LPC_GPIO_PORT->NOT[aportnum] = bm;
  }
  else
  {
  	LPC_GPIO_PORT->CLR[aportnum] = bm;
  }
}

// GPIO Port

void TGpioPort_lpc::Assign(int aportnum)
{
	portnum = aportnum;
	portptr = (volatile unsigned *)(LPC_GPIO_PORT->PIN[aportnum]);
}

void TGpioPort_lpc::Set(unsigned value)
{
	*portptr = value;
}

// GPIO Pin

void TGpioPin_lpc::Assign(int aportnum, int apinnum, bool ainvert)
{
	pinnum = 0xFF;

	if ((aportnum < 0) || (aportnum >= MAX_GPIO_PORT_NUMBER))
	{
		return;
	}

	if ((apinnum < 0) || (apinnum > 31))
	{
		return;
	}

  portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

	setbitvalue = (1 << pinnum);
	clrbitvalue = (1 << pinnum);

  togglebitptr = (unsigned *)&(LPC_GPIO_PORT->NOT[aportnum]);

  getbitptr = (unsigned *)&(LPC_GPIO_PORT->PIN[aportnum]);
  getbitshift = apinnum;

  if (ainvert)
  {
    setbitptr = (unsigned *)&(LPC_GPIO_PORT->CLR[aportnum]);
    clrbitptr = (unsigned *)&(LPC_GPIO_PORT->SET[aportnum]);
  }
  else
  {
    setbitptr = (unsigned *)&(LPC_GPIO_PORT->SET[aportnum]);
    clrbitptr = (unsigned *)&(LPC_GPIO_PORT->CLR[aportnum]);
  }
}

void TGpioPin_lpc::SwitchDirection(int adirection)
{
  if (adirection)
  {
		LPC_GPIO_PORT->DIR[portnum] |= setbitvalue;
	}
	else
	{
		LPC_GPIO_PORT->DIR[portnum] &= ~setbitvalue;
	}
}
