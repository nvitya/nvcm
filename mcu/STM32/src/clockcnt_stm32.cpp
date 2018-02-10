// clockcnt_stm32.cpp

#include "platform.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
#if defined(TIM14)
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
  #define CCTIMER  TIM14

#else
	RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;
  #define CCTIMER  TIM21
#endif

	CCTIMER->CR1 = 0;
	CCTIMER->PSC = 0; // count every clock
	CCTIMER->CR1 = 1;
	CCTIMER->EGR = 1; // reinit, start the timer
}

#endif

