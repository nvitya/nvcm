// hwclkctrl_imxrt.cpp

#include "platform.h"
#include "hwclkctrl.h"

bool THwClkCtrl_imxrt::ExtOscReady()
{
  return true; // the Boot rom initializes the external crystal
}

void THwClkCtrl_imxrt::StartExtOsc()
{
	// the Boot rom initializes the external crystal
}

void THwClkCtrl_imxrt::PrepareHiSpeed(unsigned acpuspeed)
{
	unsigned tmp;

	// no flash = no wait states...

	// Adjust the core voltage at the DCDC->REG3
	//   0 = 0.8V, every step means 0.025V
	tmp = DCDC->REG3;
	tmp &= ~DCDC_REG3_TRG_MASK;
	if (acpuspeed > 528000000)
	{
		// for 600 MHz we need higher Voltage, which is kind of overclocking !
		tmp |= DCDC_REG3_TRG(18);  // 18 = 1.25V
	}
	else
	{
		// normal voltage up to 528 MHz
		tmp |= DCDC_REG3_TRG(14);  // 14 = 1.15V
	}
	DCDC->REG3 = tmp;
}

bool THwClkCtrl_imxrt::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	if (!aextosc)
	{
		// unimplemented
		return false;
	}

	unsigned tmp;

  //CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 0x1); /* Set PERIPH_CLK2 MUX to OSC */
	tmp = CCM->CBCMR;
	tmp &= ~(3 << CCM_CBCMR_PERIPH_CLK2_SEL_SHIFT);
	tmp |=  (1 << CCM_CBCMR_PERIPH_CLK2_SEL_SHIFT); // 1 = OSC selected for PERIPH_CLK2
	CCM->CBCMR = tmp;

  //CLOCK_SetMux(kCLOCK_PeriphMux, 0x1);     /* Set PERIPH_CLK MUX to PERIPH_CLK2 */
	tmp = CCM->CBCDR;
	tmp &=  ~(1 << CCM_CBCDR_PERIPH_CLK_SEL_SHIFT);
	tmp |=   (1 << CCM_CBCDR_PERIPH_CLK_SEL_SHIFT); // 1 = derive clock from periph_clk2_clk_divided
	CCM->CBCDR = tmp;
  // Wait until CCM internal handshake finish.
  while (CCM->CDHIPR & (1U << CCM_CDHIPR_PERIPH_CLK_SEL_BUSY_SHIFT))
  {
  }

  //CLOCK_InitArmPll(&armPllConfig); /* Configure ARM PLL to 1200M */
	unsigned loopdivider;
	loopdivider = 4 * (acpuspeed / abasespeed);
  CCM_ANALOG->PLL_ARM = CCM_ANALOG_PLL_ARM_ENABLE_MASK | CCM_ANALOG_PLL_ARM_DIV_SELECT(loopdivider);
  while ((CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_LOCK_MASK) == 0)
  {
  }

  //CLOCK_InitSysPll(&sysPllConfig); /* Configure SYS PLL to 528M */
  CCM_ANALOG->PLL_SYS = CCM_ANALOG_PLL_SYS_ENABLE_MASK | CCM_ANALOG_PLL_SYS_DIV_SELECT(1);
  while ((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_LOCK_MASK) == 0)
  {
  }

  //CLOCK_InitUsb1Pll(&usb1PllConfig); /* Configure USB1 PLL to 480M */
  CCM_ANALOG->PLL_USB1 = CCM_ANALOG_PLL_USB1_ENABLE_MASK      |
                         CCM_ANALOG_PLL_USB1_POWER_MASK       |
                         CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK |
                         CCM_ANALOG_PLL_USB1_DIV_SELECT(0);
  while ((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_LOCK_MASK) == 0)
  {
  }

  //CLOCK_SetDiv(kCLOCK_ArmDiv, 0x1); /* Set ARM PODF to 1, divide by 2 */
  tmp = CCM->CACRR;
  tmp &= ~(7 << CCM_CACRR_ARM_PODF_SHIFT);
  tmp |=  (1 << CCM_CACRR_ARM_PODF_SHIFT);  // ARM PODF: 1 = divide it by 2
  CCM->CACRR = tmp;
  while (CCM->CDHIPR & (1U << CCM_CDHIPR_ARM_PODF_BUSY_SHIFT))
  {
  }

  //CLOCK_SetDiv(kCLOCK_AhbDiv, 0x0); /* Set AHB PODF to 0, divide by 1 */
  tmp = CCM->CBCDR;
  tmp &= ~(7 << CCM_CBCDR_AHB_PODF_SHIFT);
  tmp |=  (0 << CCM_CBCDR_AHB_PODF_SHIFT);  // AHB PODF: 0 = divide it by 1
  CCM->CBCDR = tmp;
  while (CCM->CDHIPR & (1U << CCM_CDHIPR_AHB_PODF_BUSY_SHIFT))
  {
  }

  //CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3); /* Set IPG PODF to 3, divide by 4 */
  tmp = CCM->CBCDR;
  tmp &= ~(3 << CCM_CBCDR_IPG_PODF_SHIFT);
  tmp |=  (3 << CCM_CBCDR_IPG_PODF_SHIFT);  // IPG PODF: 3 = divide it by 4
  CCM->CBCDR = tmp;

  //CLOCK_SetMux(kCLOCK_PrePeriphMux, 0x3); /* Set PRE_PERIPH_CLK to PLL1, 1200M */
	tmp = CCM->CBCMR;
	tmp &= ~(3 << CCM_CBCMR_PRE_PERIPH_CLK_SEL_SHIFT);
	tmp |=  (3 << CCM_CBCMR_PRE_PERIPH_CLK_SEL_SHIFT); // 3 = Set PRE_PERIPH_CLK to PLL1, 1200M
	CCM->CBCMR = tmp;

  //CLOCK_SetMux(kCLOCK_PeriphMux, 0x0);    /* Set PERIPH_CLK MUX to PRE_PERIPH_CLK */
	tmp = CCM->CBCDR;
	tmp &=  ~(1 << CCM_CBCDR_PERIPH_CLK_SEL_SHIFT);
	tmp |=   (0 << CCM_CBCDR_PERIPH_CLK_SEL_SHIFT); // 0 = derive clock from pre_periph_clk_sel
	CCM->CBCDR = tmp;
  // Wait until CCM internal handshake finish.
  while (CCM->CDHIPR & (1U << CCM_CDHIPR_PERIPH_CLK_SEL_BUSY_SHIFT))
  {
  }

  // kCLOCK_UartMux        =
  //    CCM_TUPLE(CSCDR1, CCM_CSCDR1_UART_CLK_SEL_SHIFT, CCM_CSCDR1_UART_CLK_SEL_MASK, CCM_NO_BUSY_WAIT);

/*
  CCM->CCGR0 = 0x00C0000FU;
  CCM->CCGR1 = 0x30000000U;
  CCM->CCGR2 = 0xFF3F303FU;
  CCM->CCGR3 = 0xF0000330U;
  CCM->CCGR4 = 0x0000FF3CU;
  CCM->CCGR5 = 0xF003330FU;
  CCM->CCGR6 = 0x00FC0F00U;
*/

  return true;
}

