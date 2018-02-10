// hwclkctrl_stm32.cpp

#include "platform.h"
#include "hwclkctrl.h"

bool THwClkCtrl_stm32::ExtOscReady()
{
  return ((RCC->CR & RCC_CR_HSERDY) != 0);
}

void THwClkCtrl_stm32::StartExtOsc()
{
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);
}

bool THwClkCtrl_stm32::IntHSOscReady()
{
  return ((RCC->CR & RCC_CR_HSIRDY) != 0);
}

void THwClkCtrl_stm32::StartIntHSOsc()
{
#ifdef RCC_CR_HSIKERON
  RCC->CR |= (RCC_CR_HSION | RCC_CR_HSIKERON);
#else
  RCC->CR |= RCC_CR_HSION;
#endif
}

#if defined(MCUSF_F0) || defined(MCUSF_L0)

void THwClkCtrl_stm32::PrepareHiSpeed(unsigned acpuspeed)
{
  /* Enable Prefetch Buffer and set Flash Latency */
#ifdef FLASH_ACR_PRFTBE
  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
#else
  FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN;
#endif
}

bool THwClkCtrl::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	// select the HSI as clock source
  RCC->CFGR &= ~3;
  RCC->CFGR |= RCC_CFGR_SW_HSI;
  while (((RCC->CFGR >> 2) & 3) != RCC_CFGR_SW_HSI) // wait until it is set
  {
  }

	RCC->CR &= ~RCC_CR_PLLON;  // disable the PLL

  /* Wait till PLL is not ready */
  while ((RCC->CR & RCC_CR_PLLRDY) != 0)
  {
  }

	unsigned freqmul = acpuspeed / abasespeed;

	if ((freqmul < 2) or (freqmul > 16))
	{
		return false;
	}

#if defined(MCUSF_L0)

	// only a few combinations supported, but usually the max speed will be used.

	unsigned plldiv = 1;  // = 2, using this fix value
	unsigned pllmul = 0;
	switch (freqmul)
	{
	  case  2:  pllmul = 1; break;
	  case  3:  pllmul = 2; break;
	  case  4:  pllmul = 3; break;
	  case  6:  pllmul = 4; break;
	  case  8:  pllmul = 5; break;
	  case 12:  pllmul = 6; break;
	  case 16:  pllmul = 7; break;
	  case 24:  pllmul = 8; break;
	  default:  pllmul = 0; break;
	}

	// do not divide the peripheral clocks:
  RCC->CFGR &= (uint32_t)(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

  /* PLL configuration = HSE * 6 = 48 MHz */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLDIV | RCC_CFGR_PLLMUL));
  RCC->CFGR |= (uint32_t)((pllmul << 18) | (plldiv << 22));
  if (aextosc)
  {
  	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE);
  }

#else

	unsigned pllmul = ((freqmul - 2) & 0xF);  // STM32F0 multiplyer code

  /* HCLK = SYSCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

  /* PCLK = HCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
  RCC->CFGR |= (uint32_t)(pllmul << 18);

  if (aextosc)
  {
  	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1);
  }
  else
  {
  	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_PREDIV);
  }

#endif

  //RCC->CFGR |= 0x80000000;

  /* Enable PLL */
  RCC->CR |= RCC_CR_PLLON;

  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }

  /* Select PLL as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
  {
  }

  return true;
}

#endif

#if defined(MCUSF_F1) || defined(MCUSF_F3)

void THwClkCtrl_stm32::PrepareHiSpeed(unsigned acpuspeed)
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // Enable Power Control clock

  /* Enable Prefetch Buffer */
  FLASH->ACR |= FLASH_ACR_PRFTBE;

  unsigned ws;
  if (acpuspeed <= 24000000)
  {
  	ws = 0;
  }
  else if (acpuspeed <= 48000000)
  {
  	ws = 1;
  }
  else
  {
  	ws = 2;
  }

  ws = 2;

  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  FLASH->ACR |= ws;

  //FLASH->ACR |= (1 << 3);
}

bool THwClkCtrl_stm32::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	unsigned tmp;

  RCC->CFGR &= ~3;
  RCC->CFGR |= RCC_CFGR_SW_HSI;
  while (((RCC->CFGR >> 2) & 3) != RCC_CFGR_SW_HSI) // wait until it is set
  {
  }

	RCC->CR &= ~RCC_CR_PLLON;  // disable the PLL

  /* Wait till PLL is not ready */
  while ((RCC->CR & RCC_CR_PLLRDY) != 0)
  {
  }

	unsigned freqmul = acpuspeed / abasespeed;
	if ((freqmul < 2) or (freqmul > 16))
	{
		return false;
	}

	tmp = RCC->CFGR;

	tmp &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

  /* HCLK = SYSCLK */
  tmp |= RCC_CFGR_HPRE_DIV1;

  /* PCLK2 = HCLK */
  tmp |= RCC_CFGR_PPRE2_DIV1;

  /* PCLK1 = HCLK / 2 */
  tmp |= RCC_CFGR_PPRE1_DIV2;

  RCC->CFGR = tmp;

#if !defined(RCC_CFGR_PLLMUL) && defined(RCC_CFGR_PLLMULL)
  #define RCC_CFGR_PLLMUL RCC_CFGR_PLLMULL
#endif

  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |  RCC_CFGR_PLLMUL));

#if defined(MCUSF_F1)

  if (aextosc)
  {
  	unsigned pllmul = ((freqmul - 2) & 0xF);  // STM32F1 multiplyer code
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC | (pllmul << 18));
  }
  else
  {
  	// src bit cleared = HSI / 2 is the clock source for PLL
  	// With internal RC clock the maximal speed is 64 MHz !!!
  	unsigned pllmul = ((freqmul*2 - 2) & 0xF);
    RCC->CFGR |= (uint32_t)(pllmul << 18);
  }

#elif defined(MCUSF_F3)

  if (aextosc)
  {
  	// no prediv
    unsigned pllmul = ((freqmul - 2) & 0xF);  // STM32F3 multiplyer code
    RCC->CFGR |= (uint32_t)(pllmul << 18);
  	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1);
  }
  else
  {
  	// With internal RC clock the maximal speed is 64 MHz !!!
  	// we have a /2 division
  	unsigned pllmul = freqmul*2;
  	if (pllmul >= 16)
  	{
  		pllmul = 15;
  	}
  	else
  	{
  		pllmul -= 2;
  	}
    RCC->CFGR |= (uint32_t)(pllmul << 18);
  	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_DIV2);
  }

#endif

  /* Enable PLL */
  RCC->CR |= RCC_CR_PLLON;

  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }

  /* Select PLL as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
  {
  }

  return true;
}

#endif


#if defined(MCUSF_F4)

void THwClkCtrl_stm32::PrepareHiSpeed(unsigned acpuspeed)
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // Enable Power Control clock

  // set voltage scaling for maximal speed
#if defined(PWR_CR_ODEN)

  // enabling overdrive
  PWR->CR |= PWR_CR_ODEN;
  while (!(PWR->CSR & PWR_CSR_ODRDY)) { }  // wait for status flag

  PWR->CR |= PWR_CR_ODSWEN;
  while (!(PWR->CSR & PWR_CSR_ODSWRDY)) { }  // wait for status flag

  PWR->CR |= PWR_CR_VOS; // set voltage scaling

#else
  PWR->CR |= PWR_CR_VOS; // set voltage scaling
#endif

  // set Flash latency
  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

  // enable caches and prefetch
  FLASH->ACR |= (FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN);
}

bool THwClkCtrl::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	// select the HSI as clock source
  RCC->CFGR &= ~3;
  RCC->CFGR |= RCC_CFGR_SW_HSI;
  while (((RCC->CFGR >> 2) & 3) != RCC_CFGR_SW_HSI) // wait until it is set
  {
  }

	RCC->CR &= ~RCC_CR_PLLON;  // disable the PLL

  /* Wait till PLL is not ready */
  while ((RCC->CR & RCC_CR_PLLRDY) != 0)
  {
  }

  unsigned vcospeed = acpuspeed * 2;
	unsigned pllp = 2; // divide by 2 to get the final CPU speed
	unsigned pllm = abasespeed / 2000000;   // generate 2 MHz VCO input
	unsigned plln = vcospeed / 2000000;     // the vco multiplier
	unsigned pllq = vcospeed / 48000000;  // usb speed

	RCC->PLLCFGR =
			(1 << 22)  // select HSE as the clock source
		| (pllm <<  0)
		| (plln <<  6)
		| (((pllp >> 1) - 1) << 16)
		| (pllq << 24)
	;

	RCC->CR |= RCC_CR_PLLON;  // enable the PLL

  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }

  // Set AHBCLK divider:
  RCC->CFGR &= ~RCC_CFGR_HPRE;
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

  // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  // clocks dividers

  RCC->CFGR &= ~3;
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while (((RCC->CFGR >> 2) & 3) != RCC_CFGR_SW_PLL) // wait until it is set
  {
  }

  // Set APB1CLK Divider:
  RCC->CFGR &= ~RCC_CFGR_PPRE1;
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

  // Set APB2CLK Divider:
  RCC->CFGR &= ~RCC_CFGR_PPRE2;
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

	return true;
}

#endif

#if defined(MCUSF_F7)

void THwClkCtrl_stm32::PrepareHiSpeed(unsigned acpuspeed)
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // Enable Power Control clock

  // The voltage scaling allows optimizing the power consumption when the
  // device is clocked below the maximum system frequency, to update the
  // voltage scaling value regarding system frequency refer to product
  // datasheet.

  // Enable the Over-drive to extend the clock frequency to 216 MHz
  PWR->CR1 |= PWR_CR1_ODEN;
  while((PWR->CSR1 & PWR_CSR1_ODRDY) == 0) { }

  // Enable the Over-drive switch
  PWR->CR1 |= (uint32_t)PWR_CR1_ODSWEN;
  while((PWR->CSR1 & PWR_CSR1_ODRDY) == 0) { }

  PWR->CR1 |= PWR_CR1_VOS;  // voltage scaling for maximum speed (Scale 1)

  // set Flash latency
  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  FLASH->ACR |= FLASH_ACR_LATENCY_6WS;

	FLASH->ACR |= FLASH_ACR_ARTEN | FLASH_ACR_PRFTEN;  // turn on the ART accelerator
}

bool THwClkCtrl_stm32::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	RCC->CR &= ~RCC_CR_PLLON;  // disable the PLL

  /* Wait till PLL is not ready */
  while ((RCC->CR & RCC_CR_PLLRDY) != 0)
  {
  }

  unsigned vcospeed = acpuspeed * 2;
	unsigned pllp = 2; // divide by 2 to get the final CPU speed
	unsigned pllm = abasespeed / 2000000;   // generate 2 MHz VCO input
	unsigned plln = vcospeed / 2000000;     // the vco multiplier
	unsigned pllq = vcospeed / 48000000;  // usb speed

	RCC->PLLCFGR =
			(1 << 22)  // select HSE as the clock source
		| (pllm <<  0)
		| (plln <<  6)
		| (((pllp >> 1) - 1) << 16)
		| (pllq << 24)
	;

	RCC->CR |= RCC_CR_PLLON;  // enable the PLL

  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }

  // Set AHBCLK divider:
  RCC->CFGR &= ~RCC_CFGR_HPRE;
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

  // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  // clocks dividers

  RCC->CFGR &= ~3;
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while (((RCC->CFGR >> 2) & 3) != RCC_CFGR_SW_PLL) // wait until it is set
  {
  }

  // Set APB1CLK Divider:
  RCC->CFGR &= ~RCC_CFGR_PPRE1;
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

  // Set APB2CLK Divider:
  RCC->CFGR &= ~RCC_CFGR_PPRE2;
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

	return true;
}

#endif
