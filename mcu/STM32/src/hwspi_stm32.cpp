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
 *  file:     hwspi_stm32.cpp
 *  brief:    STM32 SPI (master only)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
//#include "hwclkctrl.h"
#include "hwspi.h"

bool THwSpi_stm32::Init(int adevnum)
{
	devnum = adevnum;

	unsigned clockdiv = 1;

	initialized = false;

	regs = nullptr;
	if (false)
	{

	}
#if defined(SPI1_BASE)
	else if (1 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI1_BASE;
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
		clockdiv = 2;
	}
#endif
#if defined(SPI2_BASE)
	else if (2 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI2_BASE;
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
		clockdiv = 4;
	}
#endif
#if defined(SPI3_BASE) && defined(RCC_APB1ENR_SPI3EN)
	else if (3 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI3_BASE;
		RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
		clockdiv = 4;
	}
#endif
#if defined(SPI4_BASE)
	else if (4 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI4_BASE;
		RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
		clockdiv = 2;
	}
#endif
#if defined(SPI5_BASE)
	else if (5 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI5_BASE;
		RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
		clockdiv = 2;
	}
#endif
#if defined(SPI6_BASE)
	else if (6 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI6_BASE;
		RCC->APB2ENR |= RCC_APB2ENR_SPI6EN;
		clockdiv = 2;
	}
#endif

	if (!regs)
	{
		return false;
	}

	basespeed = SystemCoreClock / clockdiv;

	unsigned dcode = 0;
	while ((basespeed / (1 << (dcode + 1)) > speed) && (dcode < 7))
	{
		++dcode;
	}

	regs->CR1 &= ~(1 << 6);  // SPI Disable

	unsigned n;

	n = (dcode << 3)
		| (1 << 2)   // Master Mode
		| (0 << 9)   // software NSS management
		| (1 << 8)   // SSI bit
	;

	if (lsb_first)  n |= (1 << 7);
	if (idleclk_high) n |= (1 << 1);
	if (datasample_late) n |= (1 << 0);

	regs->CR1 = n;

	n =	(((databits-1) & 0x7) << 8)
		| (0 <<  4)  // Motorola Frame mode
		| (1 << 12)  // 8 bit fifo settings
	;

	if (inter_frame_pulse)  n |= (1 << 3);  // generate NSS pulse
	n |= (1 << 2);  // 1 = SS output enable

	regs->CR2 = n;

	regs->CR1 |= (1 << 6);  // SPI Enable

	initialized = true;

	return true;
}

bool THwSpi_stm32::TrySendData(unsigned short adata)
{
	if (regs->SR & SPI_SR_TXE)
	{
		*(__IO uint8_t *)(&(regs->DR)) = (adata & 0xff);
		return true;
	}
	else
	{
		return false;
	}
}

bool THwSpi_stm32::TryRecvData(unsigned short * dstptr)
{
	if (regs->SR & SPI_SR_RXNE)
	{
		*dstptr = *(__IO uint8_t *)(&regs->DR);
		return true;
	}
	else
	{
		return false;
	}
}

void THwSpi_stm32::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
	}
	else
	{
		rxdma = admach;
	}

	admach->Prepare(istx, (void *)&regs->DR, 0);
}

bool THwSpi_stm32::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	//regs->CR2 &= ~(1 << 1); // disable the TX DMA
	regs->CR2 |= (1 << 1); // enable the TX DMA

	txdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_stm32::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	//regs->CR2 &= ~(1 << 0); // disable the RX DMA
	regs->CR2 |= (1 << 0); // enable the RX DMA

	rxdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_stm32::SendFinished()
{
	if (regs->SR & SPI_SR_BSY)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool THwSpi_stm32::DmaSendCompleted()
{
	if (txdma && txdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwSpi_stm32::DmaRecvCompleted()
{
	if (rxdma && rxdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return true;
}
