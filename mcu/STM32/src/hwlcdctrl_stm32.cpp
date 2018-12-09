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
 *  file:     hwlcdctrl_stm32.cpp
 *  brief:    STM32 Integrated LCD controller implementation
 *  version:  1.00
 *  date:     2018-12-09
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwlcdctrl.h"

#if defined(LTDC_SRCR_IMR)

bool THwLcdCtrl_stm32::Init(uint16_t awidth, uint16_t aheight, void * aframebuffer)
{
	uint32_t tmp;

	hwwidth  = awidth;
	hwheight = aheight;
	framebuffer = (uint8_t *)aframebuffer;

	// generate 8 MHz pixel clock for at least 60 Frames / s

	// the VCO input clock is 2 MHz by the NVCM standard setup
	// let the PLLSAI run at 192 MHz.
	// the possible dividers for PLLSAI-R are 2 - 7 !
	// there is a post divider for the LCD clock: PLLSAIDIVR = /2, /4, /8, /16

	uint32_t freq = 192000000;
	// todo: search the best value
	uint32_t divr = 5;
	uint32_t postdiv = 4;

	tmp = RCC->PLLSAICFGR;
	tmp &= ~(0x1FF <<  6);
	tmp |=  (96    <<  6);  // PLLSAI = 2 * 96 = 192 MHz
	tmp &= ~(7     << 28);
	tmp |=  (divr  << 28);
	RCC->PLLSAICFGR = tmp;

	tmp = RCC->DCKCFGR1;
	tmp &= ~(3       << 16);
	tmp |=  (postdiv << 16);
	RCC->DCKCFGR1 = tmp;

	RCC->CR |= RCC_CR_PLLSAION; // turn on...

	// wait until the PLLSAI is ready

  while((RCC->CR & RCC_CR_PLLSAIRDY) == 0)
  {
  }

  // Enable the LCD controller
  RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;
  if (RCC->APB2ENR) { } // do some sync

	// configure the LCD controller

	LTDC_TypeDef * regs = LTDC;

	// Synchronization Size Configuration Register
	regs->SSCR = 0
		| ((hsync - 1) << 16) // Horizontal Synchronization Width - 1 (in units of pixel clock period)
		| ((vsync - 1) <<  0) // Vertical Synchronization Height - 1 (in units of horizontal scan line)
  ;

	// Back Porch Configuration Register
	regs->BPCR = 0
		| ((hsync + hbp - 1) << 16)
		| ((vsync + vbp - 1) <<  0)
  ;

	//  Active Width Configuration Register
	regs->AWCR = 0
		| ((hsync + hbp + hwwidth  - 1) << 16)
		| ((vsync + vbp + hwheight - 1) <<  0)
  ;

	// Total Width Configuration Register
	regs->TWCR = 0
		| ((hsync + hbp + hwwidth  + hfp - 1) << 16)
		| ((vsync + vbp + hwheight + vfp - 1) <<  0)
  ;

	// Global Control Register
	regs->GCR = 0
		| (0    << 31) // HSPOL: Horizontal Synchronization Polarity, 0 = active low
		| (0    << 30) // VSPOL: Vertical Synchronization Polarity, 0 = active low
		| (0    << 29) // DEPOL: Not Data Enable Polarity, 0 = active low
		| (0    << 28) // PCPOL: Pixel Clock Polarity, 0 = input pixel clock, 1 = inverted pixel clock
		| (0    << 16) // DEN: Dither Enable, 0 = disabled
		| (2    << 12) // DRW(3): Dither Red Width
		| (2    <<  8) // DGW(3): Dither Green Width
		| (2    <<  4) // DBW(3): Dither Blue Width
		| (0    <<  0) // LTDCEN: LCD-TFT controller enable bit (do not enable now)
	;

	// Background Color Configuration Register
	regs->BCCR = 0
		| (0x80 << 16)  // RED(8)
		| (0x00 <<  8)  // GREEN(8)
		| (0x00 <<  0)  // BLUE(8)
	;

	// Interrupt Enable Register
	regs->IER = 0;

	LTDC_Layer1->CR = 0; // disable the layer1
	LTDC_Layer2->CR = 0; // disable the layer2

	regs->GCR |= 1; // enable the controller

	// LAYER CONFIG

#if 1
	// configure only one layer

	lregs = LTDC_Layer1;

	lregs->CR = 0; // disable the layer for now

	// Layerx Window Horizontal Position Configuration Register
	lregs->WHPCR = 0
		| ((hsync + hbp) << 0)
		| ((hsync + hbp + hwwidth - 1) << 16)
	;

	// Layerx Window Vertical Position Configuration Register
	lregs->WVPCR = 0
		| ((vsync + vbp) << 0)
		| ((vsync + vbp + hwheight - 1) << 16)
	;

	// Layerx Color Keying Configuration Register
	lregs->CKCR = 0;

	// Layerx Pixel Format Configuration Register
	lregs->PFCR = 0
		| (2 << 0) // PF(3): 2 = RGB565
	;

	// Layerx Constant Alpha Configuration Register
	lregs->CACR = 255;

	// Layerx Default Color Configuration Register
	lregs->DCCR = 0
		| (0    << 24)
		| (0    << 16)
		| (0    <<  8)
		| (0x80 <<  0)  // should not be visible
	;

	// Layerx Blending Factors Configuration Register
	lregs->BFCR = 0
		| (4  << 8)  // BF1(3): 4 = constant alpha
		| (5  << 0)  // BF2(3): 5 = (1 - constant alpha)
	;

	// Layerx Color Frame Buffer Address Register
	lregs->CFBAR = uint32_t(framebuffer);

	// Layerx Color Frame Buffer Length Register
	lregs->CFBLR = 0
		| ((hwwidth * 2) << 16)
		| ((hwwidth * 2 + 3) << 0)
	;

	//  Layerx ColorFrame Buffer Line Number Register
	lregs->CFBLNR = 0
	  | (hwheight << 0)
	;

	lregs->CR |= 1; // enable the layer

#endif

	// reload layer config
  regs->SRCR = LTDC_SRCR_IMR; // reload config


	return true;
}

#endif
