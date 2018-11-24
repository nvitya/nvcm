/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     hwpins_imxrt.cpp
 *  brief:    IMXRT Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-11-23
 *  authors:  nvitya
*/


#include "platform.h"
#include "hwpins.h"
#include "imxrt_pads.h"
#include "imxrt_utils.h"

const unsigned char imxrt_gpio_table[] =
{
// 0: GPIO_EMC_00..31
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
// 32: GPIO_EMC_32..41
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
// 42: GPIO_AD_B0_00..15
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
// 58: GPIO_AD_B1_00..15
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
  1, 31, // 73

#if defined(MCUSF_1020)

// 74:  (GPIO_SD_B0_00-06)
  3, 13,
  3, 14,
  3, 15,
  3, 16,
  3, 17,
  3, 18,
  3, 19,

// 81:  (GPIO_SD_B1_00-11)
  3, 20,
  3, 21,
  3, 22,
  3, 23,
  3, 24,
  3, 25,
  3, 26,
  3, 27,
  3, 28,
  3, 29,
  3, 30,
  3, 31,

#elif defined(MCUSF_1050)
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

#else

#error "The GPIO talble is not defined for this IMXRT subfamily"

#endif

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

  padc |= (1 << 16); // enable hysteresis

  *padctrlreg = padc;
}


bool THwPinCtrl_imxrt::PadSetup(int padnum, unsigned flags)
{
	unsigned * iomuxaddr;
	unsigned * padctrladdr;

	if ((padnum >= 0x100) && (padnum <= 0x102))
	{
		// the special pads are not where the others are
		unsigned pinnum = padnum - 0x100;
		iomuxaddr   = (unsigned *)(IMXRT_SPECPAD_MUX_REG_START + 4 * pinnum);
		padctrladdr = (unsigned *)(IMXRT_SPECPAD_CTRL_REG_START + 4 * pinnum);
	}
	else if ((padnum >= 0) && (padnum < PAD_COUNT))
	{
		iomuxaddr   = (unsigned *)(IMXRT_GENPAD_MUX_REG_START + 4 * padnum);
		padctrladdr = (unsigned *)(IMXRT_GENPAD_CTRL_REG_START + 4 * padnum);
	}
   else
   {
      return false;
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
			if (flags & PINCFG_GPIO_INIT_1)
			{
				regs->DR |= (1 << pinnum);
			}
			else
			{
				regs->DR &= ~(1 << pinnum);
			}

			regs->GDIR |= (1 << pinnum);
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

// see iomuxc_select_input_t definition in the MIMXRT10xx.h for the possible function identifiers
bool THwPinCtrl_imxrt::InputSelect(int afuncid, unsigned ainput)
{
	volatile uint32_t * daisyreg = (volatile uint32_t *)(IMXRT_GENPAD_IOSEL_REG_START + 4 * afuncid);
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

  if (1 == value)
  {
  	regs->DR_SET = (1 << apinnum);
  }
  else if (value & 2) // toggle
  {
  	regs->DR_TOGGLE = (1 << apinnum);
  }
  else
  {
  	regs->DR_CLEAR = (1 << apinnum);
  }
}

// GPIO Port

void TGpioPort_imxrt::Assign(int aportnum)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
}

void TGpioPort_imxrt::Set(unsigned value)
{
	regs->DR = value;
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

	if ((apinnum < 0) || (apinnum > 31))
	{
		regs = nullptr;
		return;
	}

	clrbitvalue = (1 << pinnum);
	setbitvalue = (1 << pinnum);
  getbitptr = (unsigned *)&(regs->PSR);
  getbitshift = pinnum;
  togglebitptr = (unsigned *)&(regs->DR_TOGGLE);

  if (ainvert)
  {
    setbitptr = (unsigned *)&(regs->DR_CLEAR);
    clrbitptr = (unsigned *)&(regs->DR_SET);
  }
  else
  {
    setbitptr = (unsigned *)&(regs->DR_SET);
    clrbitptr = (unsigned *)&(regs->DR_CLEAR);
  }
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
