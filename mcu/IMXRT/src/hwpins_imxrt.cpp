// hwpins_imxrt.cpp

#include "platform.h"
#include "hwpins.h"
#include "imxrt_pads.h"
#include "imxrt_utils.h"

const unsigned char imxrt_gpio_table[] =
{
// 0
  4, 0,
  4, 1,
  4, 2,
  4, 3,
  4, 4,
  4, 5,
  4, 6,
  4, 7,
  4, 8,
  4, 9,
  4, 10,
  4, 11,
  4, 12,
  4, 13,
  4, 14,
  4, 15,
  4, 16,
  4, 17,
  4, 18,
  4, 19,
  4, 20,
  4, 21,
  4, 22,
  4, 23,
  4, 24,
  4, 25,
  4, 26,
  4, 27,
  4, 28,
  4, 29,
  4, 30,
  4, 31,
// 32
  3, 18,
  3, 19,
  3, 20,
  3, 21,
  3, 22,
  3, 23,
  3, 24,
  3, 25,
  3, 26,
  3, 27,
// 42
  1, 0,
  1, 1,
  1, 2,
  1, 3,
  1, 4,
  1, 5,
  1, 6,
  1, 7,
  1, 8,
  1, 9,
  1, 10,
  1, 11,
  1, 12,
  1, 13,
  1, 14,
  1, 15,
  1, 16,
  1, 17,
  1, 18,
  1, 19,
  1, 20,
  1, 21,
  1, 22,
  1, 23,
  1, 24,
  1, 25,
  1, 26,
  1, 27,
  1, 28,
  1, 29,
  1, 30,
  1, 31,
// 74
  2, 0,
  2, 1,
  2, 2,
  2, 3,
  2, 4,
  2, 5,
  2, 6,
  2, 7,
  2, 8,
  2, 9,
  2, 10,
  2, 11,
  2, 12,
  2, 13,
  2, 14,
  2, 15,
  2, 16,
  2, 17,
  2, 18,
  2, 19,
  2, 20,
  2, 21,
  2, 22,
  2, 23,
  2, 24,
  2, 25,
  2, 26,
  2, 27,
  2, 28,
  2, 29,
  2, 30,
  2, 31,
// 106
  3, 12,
  3, 13,
  3, 14,
  3, 15,
  3, 16,
  3, 17,
// 112
  3, 0,
  3, 1,
  3, 2,
  3, 3,
  3, 4,
  3, 5,
  3, 6,
  3, 7,
  3, 8,
  3, 9,
  3, 10,
  3, 11,

// 124
  5, 0,
  5, 1,
  5, 2,

	0xFF, 0xFF  // list terminator
};

const unsigned imxrt_gpio_base_addresses[] = GPIO_BASE_ADDRS;

int imxrt_pad_by_gpio(int aportnum, int apinnum)
{
	unsigned short searchval = (aportnum | (apinnum << 8));
	unsigned short * tp = (unsigned short *)(&imxrt_gpio_table[0]);
	unsigned short cv;
	int idx = 0;
	while (1)
	{
		cv = *tp;
		if (cv == searchval)
		{
			return idx;
		}

	  if (cv == 0xFFFF)
	  {
	  	return -1;
	  }

		++tp;
		++idx;
	}
}

HW_GPIO_REGS * THwPinCtrl_imxrt::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum > MAX_GPIO_PORT_NUMBER))
	{
		return nullptr;
	}
	else
	{
		return (HW_GPIO_REGS *)(imxrt_gpio_base_addresses[aportnum]);
	}
}

void THwPinCtrl_imxrt::UpdateGpioOutputShadow()
{
	unsigned n;
	for (n = 0; n < MAX_GPIO_PORT_NUMBER; ++n)
	{
		HW_GPIO_REGS * regs = GetGpioRegs(n);
		gpio_output_shadow[n] = regs->DR;
	}
}

bool THwPinCtrl_imxrt::GpioSetup(int aportnum, int apinnum, unsigned flags)
{
	int padnum = -1;

	if (aportnum >= 0)
	{
		// search pad by GPIO
		if ((apinnum < 0) || (apinnum > 31))
		{
			return false;
		}

		padnum = imxrt_pad_by_gpio(aportnum, apinnum);
	}
	else
	{
		padnum = apinnum;
	}

	return PadSetup(padnum, flags);
}

void THwPinCtrl_imxrt::SetPadControl(unsigned * padctrlreg, unsigned flags)
{
	// Set PAD control

	unsigned padc = 0
		| (0 << 16)  // histeresis disable
	;

	if (flags & PINCFG_OPENDRAIN)
	{
		padc |= (1 << 11);
	}


  if (flags & PINCFG_PULLUP)
  {
  	padc |= (0
  		|	(2 << 14)  // 100k Pullup
  		|	(1 << 13)  // select pull
  		|	(1 << 12)  // enable pull/keeper
		);
  }
  else if (flags & PINCFG_PULLDOWN)
  {
  	padc |= (0
  		|	(0 << 14)  // 100k Pulldown
  		|	(1 << 13)  // select pull
  		|	(1 << 12)  // enable pull/keeper
		);
  }

  if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_MEDIUM)
  {
  	padc |= (1 << 6);
  }
  else if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_FAST)
  {
  	padc |= (3 << 6);
  	padc |= (1 << 0); // turn on fast slew rate too!
  }

  if (flags & PINCFG_DRIVE_STRONG)
  {
  	padc |= (7 << 3);
  }
  else
  {
  	padc |= (6 << 3); // medium drive strength (this is the default)
  }

  *padctrlreg = padc;
}


bool THwPinCtrl_imxrt::PadSetup(int padnum, unsigned flags)
{
	if ((padnum < 0) || (padnum >= PAD_COUNT))
	{
		return false;
	}

	unsigned * iomuxaddr;
	unsigned * padctrladdr;

	if (padnum >= PAD_WAKEUP)
	{
		// the last pads are not where the others are
		unsigned pinnum = padnum - PAD_WAKEUP;
		iomuxaddr   = (unsigned *)(0x400A8000 + 4 * pinnum);
		padctrladdr = (unsigned *)(0x400A8018 + 4 * pinnum);
	}
	else
	{
		iomuxaddr   = (unsigned *)(0x401F8014 + 4 * padnum);
		padctrladdr = (unsigned *)(0x401F8204 + 4 * padnum);
	}

	SetPadControl(padctrladdr, flags);

  // set IOMUX

	unsigned iomux = 0
	  | (0 << 4);  // 1 == force input path

	if (flags & PINCFG_AF_MASK)
	{
		iomux |= ((flags >> PINCFG_AF_SHIFT) & 7);
	}
	else
	{
		iomux |= 5; // GPIO is always ALT_5
	}

	*iomuxaddr = iomux;

  // Set GPIO
	if ((flags & PINCFG_AF_MASK) == 0)
	{
		// GPIO

	  unsigned char * cp = (unsigned char *)&(imxrt_gpio_table[padnum * 2]);
	  unsigned char portnum = *cp;
	  ++cp;
	  unsigned char pinnum = *cp;

		GpioPortEnable(portnum);

		HW_GPIO_REGS * regs = GetGpioRegs(portnum);
		if (!regs)
		{
			return false;
		}

		if (flags & PINCFG_OUTPUT)
		{
			hwpinctrl.UpdateGpioOutputShadow();

			regs->GDIR |= (1 << pinnum);

			if (flags & PINCFG_GPIO_INIT_1)
			{
				regs->DR |= (1 << pinnum);
			}
			else
			{
				regs->DR &= ~(1 << pinnum);
			}
		}
		else
		{
			regs->GDIR &= ~(1 << pinnum);
		}
	}

  return true;
}

// alternative PadSetup based on the defines in the imxrt_iomuxc.h.
// Warning!: no GPIO setup
void THwPinCtrl_imxrt::PadSetup(uint32_t muxreg, uint8_t muxmode, uint32_t inputreg, uint8_t daisy, uint32_t confreg, unsigned flags)
{
	unsigned tmp;
	volatile uint32_t * reg;

	reg = (volatile uint32_t *)muxreg;
	tmp = *reg;
	tmp &= ~7;
	tmp |= (muxmode & 7);
	*reg = tmp;

	reg = (volatile uint32_t *)inputreg;
  if (reg)
  {
    *reg = daisy;
  }

  if (confreg)
  {
  	SetPadControl((unsigned *)confreg, flags);
  }
}

// see iomuxc_select_input_t definition in the MIMXRT1052.h for the possible function identifiers
bool THwPinCtrl_imxrt::InputSelect(int afuncid, unsigned ainput)
{
	volatile uint32_t * daisyreg = (volatile uint32_t *)(0x401F83F4 + 4 * afuncid);
	*daisyreg = ainput;
	return true;
}

bool THwPinCtrl_imxrt::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_GPIO_PORT_NUMBER))
	{
		return false;
	}

	// TODO: enable, however it is enabled by default

  return true;
}

void THwPinCtrl_imxrt::GpioSet(int aportnum, int apinnum, int value)
{
	HW_GPIO_REGS * regs = GetGpioRegs(aportnum);
	unsigned * drshadow = &gpio_output_shadow[aportnum];

	unsigned pm = __get_PRIMASK();  // save interrupt disable status
	__disable_irq();

	unsigned tmp = *drshadow; // shadow gpio DR register, to avoid slow read
  if (1 == value)
  {
  	tmp |= (1 << apinnum);
  }
  else if (value & 2) // toggle
  {
  	tmp ^= (1 << apinnum);
  }
  else
  {
  	tmp &= ~(1 << apinnum);
  }
	regs->DR = tmp;  // set the real output
	*drshadow = tmp;  // update shadow

 	__set_PRIMASK(pm); // restore interrupt disable status
}

// GPIO Port

void TGpioPort_imxrt::Assign(int aportnum)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
	drshadow = &hwpinctrl.gpio_output_shadow[aportnum];
}

void TGpioPort_imxrt::Set(unsigned value)
{
	regs->DR = value;
	*drshadow = value;
}

// GPIO Pin

void TGpioPin_imxrt::Assign(int aportnum, int apinnum, bool ainvert)
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
	clrbitvalue = ~(1 << pinnum);
  getbitptr = (unsigned *)&(regs->PSR);
  getbitshift = pinnum;

  drreal   = (unsigned *)&(regs->DR);
	drshadow = (unsigned *)&(hwpinctrl.gpio_output_shadow[aportnum]);
}

void TGpioPin_imxrt::Set0()
{
	unsigned pm = __get_PRIMASK();  // save interrupt disable status
	__disable_irq();
	unsigned tmp = *drshadow; // shadow gpio DR register, to avoid slow read
	if (inverted)
	{
		tmp |= setbitvalue;
	}
	else
	{
		tmp &= clrbitvalue;
	}

	*drreal = tmp;  // set the real output
	*drshadow = tmp;  // update shadow
 	__set_PRIMASK(pm); // restore interrupt disable status
}

void TGpioPin_imxrt::Set1()
{
	unsigned pm = __get_PRIMASK(); // save interrupt disable status
	__disable_irq();
	unsigned tmp = *drshadow; // read shadow gpio DR register, to avoid slow read
	if (inverted)
	{
		tmp &= clrbitvalue;
	}
	else
	{
		tmp |= setbitvalue;
	}
	*drreal = tmp;  // set the real output
	*drshadow = tmp;  // update shadow
 	__set_PRIMASK(pm); // restore interrupt disable status
}

void TGpioPin_imxrt::Toggle()
{
	unsigned pm = __get_PRIMASK(); // save interrupt disable status
	__disable_irq();
	unsigned tmp = *drshadow; // shadow gpio DR register, to avoid slow read
	tmp ^= setbitvalue;
	*drreal = tmp;  // set the real output
	*drshadow = tmp;  // update shadow
 	__set_PRIMASK(pm); // restore interrupt disable status
}

void TGpioPin_imxrt::SwitchDirection(int adirection)
{
	if (adirection)
	{
		regs->GDIR |= setbitvalue;
	}
	else
	{
		regs->GDIR &= ~setbitvalue;
	}
}
