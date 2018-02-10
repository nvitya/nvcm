// hwclkctrl_lpc.cpp

#include "platform.h"
#include "hwclkctrl.h"

bool THwClkCtrl_lpc::ExtOscReady()
{
  return true;
}

void THwClkCtrl_lpc::StartExtOsc()
{
	if (LPC_CGU->XTAL_OSC_CTRL & 2)
	{
		// clear bypass mode
		LPC_CGU->XTAL_OSC_CTRL &= ~2;
	}

	// we assume low speed oscillator (1 - 20 MHz)
	if (LPC_CGU->XTAL_OSC_CTRL & 4)
	{
		// clear highspeed mode
		LPC_CGU->XTAL_OSC_CTRL &= ~4;
	}

	// start the oscillator
	LPC_CGU->XTAL_OSC_CTRL &= ~1;

	volatile uint32_t delay = 1000;
	/* Delay for 250uSec */
	while (delay--) {}
}

void THwClkCtrl_lpc::PrepareHiSpeed(unsigned acpuspeed)
{
	uint32_t FAValue = acpuspeed / 21510000;

	LPC_CREG->FLASHCFGA = (LPC_CREG->FLASHCFGA & (~(0xF << 12))) | (FAValue << 12);
	LPC_CREG->FLASHCFGB = (LPC_CREG->FLASHCFGB & (~(0xF << 12))) | (FAValue << 12);
}



/* Structure for initial base clock states */
struct CLK_BASE_STATES
{
	CHIP_CGU_BASE_CLK_T clk;	/* Base clock */
	CHIP_CGU_CLKIN_T clkin;	/* Base clock source, see UM for allowable souorces per base clock */
	bool powerdn;			/* Set to true if the base clock is initially powered down */
};

static const struct CLK_BASE_STATES InitClkStates[] =
{
	{CLK_BASE_SAFE,     CLKIN_IRC,     false},
	{CLK_BASE_APB1,     CLKIN_MAINPLL, false},
	{CLK_BASE_APB3,     CLKIN_MAINPLL, false},
	{CLK_BASE_USB0,     CLKIN_USBPLL,  true},
	{CLK_BASE_PERIPH,   CLKIN_MAINPLL, false},
	{CLK_BASE_SPI,      CLKIN_MAINPLL, false},
	{CLK_BASE_ADCHS,    CLKIN_MAINPLL, true},
	{CLK_BASE_SDIO,     CLKIN_MAINPLL, false},
	{CLK_BASE_SSP0,     CLKIN_MAINPLL, false},
	{CLK_BASE_SSP1,     CLKIN_MAINPLL, false},
	{CLK_BASE_UART0,    CLKIN_MAINPLL, false},
	{CLK_BASE_UART1,    CLKIN_MAINPLL, false},
	{CLK_BASE_UART2,    CLKIN_MAINPLL, false},
	{CLK_BASE_UART3,    CLKIN_MAINPLL, false},
	{CLK_BASE_OUT,      CLKINPUT_PD,   false},
	{CLK_BASE_APLL,     CLKINPUT_PD,   false},
	{CLK_BASE_CGU_OUT0, CLKINPUT_PD,   false},
	{CLK_BASE_CGU_OUT1, CLKINPUT_PD,   false},
};

static const CHIP_CCU_CLK_T TurnOffClocks[] =
{
	CLK_APB3_I2C1,		/*!< I2C1 register/perigheral clock from base clock CLK_BASE_APB3 */
	CLK_APB1_MOTOCON,	/*!< Motor controller register/perigheral clock from base clock CLK_BASE_APB1 */
	CLK_APB1_I2C0,		/*!< I2C0 register/perigheral clock from base clock CLK_BASE_APB1 */
	CLK_APB1_I2S,		/*!< I2S register/perigheral clock from base clock CLK_BASE_APB1 */
	CLK_APB1_CAN1,		/*!< CAN1 register/perigheral clock from base clock CLK_BASE_APB1 */

	CLK_MX_LCD,			/*!< LCD register clock from base clock CLK_BASE_MX */
	CLK_MX_ETHERNET,	/*!< ETHERNET register clock from base clock CLK_BASE_MX */
	CLK_MX_USB0,		/*!< USB0 register clock from base clock CLK_BASE_MX */
	CLK_MX_SDIO,		/*!< SDIO register clock from base clock CLK_BASE_MX */
	CLK_MX_SCT,			/*!< SCT register clock from base clock CLK_BASE_MX */
	CLK_MX_USB1,		/*!< USB1 register clock from base clock CLK_BASE_MX */
#if defined(CHIP_LPC43XX)
	CLK_M4_M0APP,		/*!< M0 app CPU core clock from base clock CLK_BASE_MX */
	CLK_MX_ADCHS,		/*!< ADCHS clock from base clock CLK_BASE_ADCHS */
#endif
	CLK_MX_UART0,		/*!< UART0 register clock from base clock CLK_BASE_MX */
	CLK_MX_UART1,		/*!< UART1 register clock from base clock CLK_BASE_MX */
	CLK_MX_SSP0,		/*!< SSP0 register clock from base clock CLK_BASE_MX */
	CLK_MX_TIMER0,		/*!< TIMER0 register/perigheral clock from base clock CLK_BASE_MX */
	CLK_MX_TIMER1,		/*!< TIMER1 register/perigheral clock from base clock CLK_BASE_MX */
	CLK_MX_UART2,		/*!< UART3 register clock from base clock CLK_BASE_MX */
	CLK_MX_UART3,		/*!< UART4 register clock from base clock CLK_BASE_MX */
	CLK_MX_TIMER2,		/*!< TIMER2 register/perigheral clock from base clock CLK_BASE_MX */
	CLK_MX_TIMER3,		/*!< TIMER3 register/perigheral clock from base clock CLK_BASE_MX */
	CLK_MX_SSP1,		/*!< SSP1 register clock from base clock CLK_BASE_MX */
	CLK_MX_QEI			/*!< QEI register/perigheral clock from base clock CLK_BASE_MX */
};


void Chip_Clock_SetBaseClock(CHIP_CGU_BASE_CLK_T BaseClock, CHIP_CGU_CLKIN_T Input, bool autoblocken, bool powerdn)
{
	uint32_t reg = LPC_CGU->BASE_CLK[BaseClock];

	if (BaseClock < CLK_BASE_NONE)
	{
		if (Input != CLKINPUT_PD)
		{
			reg &= ~((0x1F << 24) | 1 | (1 << 11));
			if (autoblocken)  reg |= (1 << 11);
			if (powerdn)  		reg |= (1 << 0);
			reg |= (Input << 24);
			LPC_CGU->BASE_CLK[BaseClock] = reg;
		}
	}
	else
	{
		LPC_CGU->BASE_CLK[BaseClock] = reg | 1;	/* Power down this base clock */
	}
}

void Chip_Clock_Disable(CHIP_CCU_CLK_T clk)
{
	if (clk >= CLK_CCU2_START)
	{
		LPC_CCU2->CLKCCU[clk - CLK_CCU2_START].CFG &= ~1;
	}
	else {
		LPC_CCU1->CLKCCU[clk].CFG &= ~1;
	}
}

#define PLL_MIN_CCO_FREQ 156000000  /**< Min CCO frequency of main PLL */
#define PLL_MAX_CCO_FREQ 320000000  /**< Max CCO frequency of main PLL */

bool THwClkCtrl_lpc::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	int i;
	volatile uint32_t delay = 500;
	//PLL_PARAM_T ppll;

	for (i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); i++)
	{
		Chip_Clock_SetBaseClock(InitClkStates[i].clk, CLKIN_IRC, true, InitClkStates[i].powerdn);
	}

	//Chip_Clock_SetBaseClock(CLK_BASE_MX, clkin, true, false);

	uint32_t baseclk = LPC_CGU->BASE_CLK[CLK_BASE_MX];  // CPU core base clock
	baseclk &= ~(0x1F << 24);
	baseclk |= (CLKIN_IRC << 24);  // set clock source
	baseclk &= ~(1 << 0); // clear power down
	baseclk |= (1 << 11); // enable autoblock
	LPC_CGU->BASE_CLK[CLK_BASE_MX] = baseclk;

	for (delay = 500; delay > 0; --delay) { }  // Wait for approx 50 uSec

	// power down main PLL
	LPC_CGU->PLL1_CTRL |= 1;

	uint32_t pllcfg = 0;

	// Calculate the PLL Parameters
	unsigned nsel, msel, psel;
	if (acpuspeed >= PLL_MIN_CCO_FREQ)
	{
		// the usual case
		pllcfg &= ~(1 << 6); // clear FBSEL

		// setting direct mode immediately makes the start instable
		// so therefore we increase the speed in more steps
		pllcfg &= ~(1 << 7); // clear direct mode for now (post divided by 2)

		psel = 0;
		nsel = 0;
		msel = (acpuspeed / abasespeed) - 1;
	}
	else
	{
		// not implemented
		return false;
	}

	pllcfg |= (psel << 8);
	pllcfg |= (nsel << 12);
	pllcfg |= (msel << 16);
	pllcfg |= (CLKIN_CRYSTAL << 24);  // set clock source

	LPC_CGU->PLL1_CTRL = pllcfg;  // Setup and start the PLL

	while ((LPC_CGU->PLL1_STAT & 1) == 0) {}  // Wait for the PLL to lock

	for (delay = 50000; delay > 0; --delay) { }  // Wait for approx 50 uSec

	// Set core clock base as PLL1
	//Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_MAINPLL, true, false);
	baseclk &= ~(0x1F << 24);
	baseclk |= (CLKIN_MAINPLL << 24);  // set clock source
	LPC_CGU->BASE_CLK[CLK_BASE_MX] = baseclk;

	for (delay = 1000; delay > 0; --delay) { __NOP(); }  // Wait for approx 50 uSec

	pllcfg |= (1 << 7); // set direct mode
	LPC_CGU->PLL1_CTRL = pllcfg;

	// Setup system base clocks and initial states. This won't enable and
	//   disable individual clocks, but sets up the base clock sources for
	//   each individual peripheral clock.
	for (i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); i++)
	{
		Chip_Clock_SetBaseClock(InitClkStates[i].clk, InitClkStates[i].clkin, true, InitClkStates[i].powerdn);
	}

	// The LPC43xx chip is quite HOT !
	// many clocks are turned on by default we turn some of them off
	for (i = 0; i < (sizeof(TurnOffClocks) / sizeof(TurnOffClocks[0])); i++)
	{
		Chip_Clock_Disable(TurnOffClocks[i]);
	}

	return true;
}
