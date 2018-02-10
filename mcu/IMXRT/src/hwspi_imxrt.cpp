// hwspi_imxrt.cpp

#include "platform.h"
#include "imxrt_utils.h"
#include "hwspi.h"

bool THwSpi_imxrt::Init(int adevnum)
{
	unsigned tmp;
	devnum = adevnum;

	// Set LPSPI base clock: PLL3.PFD0 = 720 MHz, divided by 6 = 120 MHz
	tmp = CCM->CBCMR;
	tmp &= ~(CCM_CBCMR_LPSPI_CLK_SEL_MASK | CCM_CBCMR_LPSPI_PODF_MASK);
	tmp |=  CCM_CBCMR_LPSPI_CLK_SEL(1);  // 1 = PLL3.PFD0 = 720 MHz
	tmp |=  CCM_CBCMR_LPSPI_PODF(5);     // 5 = divide by 6
	CCM->CBCMR = tmp;
	unsigned baseclock = 120000000;

	regs = nullptr;
	if (1 == devnum)
	{
		regs = (HW_SPI_REGS *)LPSPI1;
		imxrt_set_clock_gate(1, 0, 3);
	}
	else if (2 == devnum)
	{
		regs = (HW_SPI_REGS *)LPSPI2;
		imxrt_set_clock_gate(1, 2, 3);
	}
	else if (3 == devnum)
	{
		regs = (HW_SPI_REGS *)LPSPI3;
		imxrt_set_clock_gate(1, 4, 3);
	}
	else if (4 == devnum)
	{
		regs = (HW_SPI_REGS *)LPSPI4;
		imxrt_set_clock_gate(1, 6, 3);
	}

	if (!regs)
	{
		return false;
	}

	regs->CR = 0
	  | (1 << 9)  // Reset Receive FIFO
	  | (1 << 8)  // Reset Transmit FIFO
	  | (1 << 1)  // Reset
	  | (1 << 0)  // Enable Module
	;

	for (tmp = 0; tmp < 100; ++tmp) {  __NOP(); }

	// this configuration causes small CS toggling between frames
	// so manual chip select is suggested!

	regs->CR = 1; // remove resets and clears

	regs->DER = 0;  // disable DMA

	regs->CFGR0 = 0;

	regs->CFGR1 = 0
		| (1 << 27)  // PCSCFG: 1 = enable 4-bit transfers
		| (1 << 26)	 // OUTCFG: 1 = tri-state output on CS high
		| (0 << 24)	 // PINCFG: 0 = normal duplex SIN, SOUT
		| (0 << 16)	 // MATCFG(7): match configuration, 0 = off
		| (0 <<  8)	 // PCSPOL: 0 = CS active low
		| (0 <<  3)	 // NOSTALL: 0 = stall active, 1 = buffer under/overruns reported
		| (0 <<  2)	 // AUTOPCS: 0 = no inter-frame CS toggling
		| (0 <<  1)	 // SAMPLE: 0 = sampling at SCK edge, 1 = sampling at delayed SCK edge
		| (1 <<  0)	 // MASTER: 1 = master mode
	;

	if (inter_frame_pulse)  regs->CFGR1 |= (1 << 2);

	// the minimal clock divisor is 2 !
	unsigned sckdiv = baseclock / speed;
	if (sckdiv * speed != baseclock)
	{
		++sckdiv;
	}

	if (sckdiv < 2)  sckdiv = 2;
	if (sckdiv > 257)  sckdiv = 257;

	// clock configuration register

	// configure a delay which provides continous transfer at low frequencies:
	unsigned csdelay = ((sckdiv + 1) >> 2);

	regs->CCR = 0
		| (csdelay << 24)	 // SCKPCS(8): 0 = 1 cycle delay after last clock to CS deactivation
		| (csdelay << 16)	 // PCSSCK(8): 0 = 1 cycle delay after to first clock after CS activation
		| (0 <<  8)	 			 // DBT(8): delay between transfers
		| ((sckdiv - 2) <<  0)	 // SCKDIV(8):
	;

	// The watermarks must be 0, otherwise the last byte won't be reported to the DMA !
	regs->FCR = 0
		| (0 << 16)	 // RXWATER(4): 1 = 1 word
		| (0 <<  0)	 // TXWATER(4): 1 = 1 word
	;

	// do not set the Transmit Command Register, because it goes only into the FIFO too!

	tcrbase = 0
		| (0 << 31)  // CPOL: 0 = SCK low in idle
		| (0 << 30)	 // CPHA: 0 = leading edge capture
		| (0 << 27)	 // PRESCALE(3): 0 = no prescale applied to the clock configuration
		| (0 << 24)	 // PCS(2): 0 = use LPSPI_PCS(0)
		| (0 << 23)	 // LSBF: 0 = MSB first (standard)
		| (0 << 22)	 // BYSW: Byte Swap, 0 = off
		| (0 << 21)	 // CONT: Continuous Transfer, 1 = keep the PCS active until the next command
		| (0 << 20)	 // CONTC: Continuing Command, 0 = start of a new transfer
		| (0 << 19)	 // RXMSK: 1 = ignore RX data
		| (0 << 18)	 // TXMSK: 1 = ignore TX data (tri-state tx lines)
		| (0 << 16)	 // WIDTH(2): Transfer Width, 0 = 1 bit, 1 = 2 bit (dual), 2 = 4 bit (quad)
		| ((databits - 1) <<  0)	 // FRAMESZ(12): Frame Size
	;

	if (idleclk_high)     tcrbase |= (1 << 31);
	if (datasample_late)  tcrbase |= (1 << 30);
	if (lsb_first)        tcrbase |= (1 << 23);

	regs->TCR = tcrbase; // set the transmit parameters

	return true;
}

bool THwSpi_imxrt::TrySendData(unsigned short adata)
{
	if ((regs->FSR & 0x1F) >= 30)
	{
		return false;
	}

	// ok to transmit
	//regs->TCR = tcrbase; // set the transmit parameters
	regs->TDR = adata;

	return true;
}

bool THwSpi_imxrt::TryRecvData(unsigned short * dstptr)
{
	if (regs->RSR & (1 << 1)) // FIFO Empty?
	{
		return false;
	}

	*dstptr = regs->RDR;
	return true;
}

void THwSpi_imxrt::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
		regs->DER |= (1 << 0); // enable the TX DMA
		admach->Prepare(true, (void *)&regs->TDR, 0);
	}
	else
	{
		rxdma = admach;
		regs->DER |= (1 << 1); // enable the RX DMA
		admach->Prepare(false, (void *)&regs->RDR, 0);
	}
}

bool THwSpi_imxrt::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	txdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_imxrt::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	rxdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_imxrt::SendFinished()
{
	if (regs->SR & (1 << 24)) // Check busy flag
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool THwSpi_imxrt::DmaSendCompleted()
{
	if (txdma && txdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwSpi_imxrt::DmaRecvCompleted()
{
	if (rxdma && rxdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return true;
}
