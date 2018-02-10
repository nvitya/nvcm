// hwqspi_imxrt.cpp

#include "string.h"
#include "platform.h"
#include "imxrt_utils.h"
#include "hwqspi.h"

#include "traces.h"

#define FLEXSPI_LUT_KEY_VAL  0x5AF05AF0

// for every flash device a LUT table with instruction sequences must be defined (see "Look Up Table" in the FlexSPI controller manual)
// the LUT contains 16 sequences of up to 8 instructions
// every FlexSPI transaction is tied to at least one LUT sequence (sequences can be chained)

#define LUTINS(acmd, apads, adata)  (adata | (apads << 8) | (acmd << 10))
// apads: 0 = 1x line, 1 = 2x, 2 = 4x, 3 = 8x

// LUT commands:
//  0: end of LUT instuction sequence
//  1: transmit command code (usually the first)
//  2: transmit row address, data = bit number
//  6: transmit 4 mode bits, data = mode bits
//  7: transmit 8 mode bits, data = mode bits
//  8: transmit programming data to flash
//  9: receive data from flash
// 12: dummy cycles, data = clock count (usually 8 required)

const unsigned short flexspi_lut_qspi_generic[128] =
{
	//  0: SINGLE LINE READ, command = ADDRESS(7..0)
		LUTINS(2,0,8), LUTINS(9,0,0), 0, 0, 0, 0, 0, 0,
	//  1: SINGLE LINE WRITE, command = ADDRESS(7..0)
		LUTINS(2,0,8), LUTINS(8,0,0), 0, 0, 0, 0, 0, 0,
	//  2: SINGLE LINE WRITE WITHOUT DATA, command = ADDRESS(7..0)
		LUTINS(2,0,8), 0, 0, 0, 0, 0, 0, 0,
	//  3:
		0, 0, 0, 0, 0, 0, 0, 0,
	//  4:
		0, 0, 0, 0, 0, 0, 0, 0,
	//  5:
		0, 0, 0, 0, 0, 0, 0, 0,
	//  6:
		0, 0, 0, 0, 0, 0, 0, 0,
	//  7:
		0, 0, 0, 0, 0, 0, 0, 0,

	//  8: SINGLE LINE FAST READ
		LUTINS(1,0,0x0B), LUTINS(2,0,24), LUTINS(12,0,8), LUTINS(9,0,0), 0, 0, 0, 0,
	//  9: DUAL I/O READ
		LUTINS(1,0,0xBB), LUTINS(2,1,24), LUTINS(6,1,0x00), LUTINS(12,1,2), LUTINS(9,1,0), 0, 0, 0,
	// 10: QUAD I/O READ
		LUTINS(1,0,0xEB), LUTINS(2,2,24), LUTINS(7,2,0x00), LUTINS(12,2,4), LUTINS(9,2,0), 0, 0, 0,

	// 11:
		0, 0, 0, 0, 0, 0, 0, 0,
	// 12: SINGLE LINE PAGE PROGRAM
		LUTINS(1,0,0x02), LUTINS(2,0,24), LUTINS(8,0,0), 0, 0, 0, 0, 0,
	// 13: QUAD LINE PAGE PROGRAM
		LUTINS(1,0,0x32), LUTINS(2,0,24), LUTINS(8,2,0), 0, 0, 0, 0, 0,
	// 14:
		0, 0, 0, 0, 0, 0, 0, 0,
	// 15:
		0, 0, 0, 0, 0, 0, 0, 0
};

bool THwQspi_imxrt::InitInterface()
{
	unsigned pincfg = PINCFG_SPEED_FAST | PINCFG_AF_1 | PINCFG_PULLUP;

	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_06, pincfg); // CS
	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_07, pincfg); // CLK

	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_08, pincfg); // DATA0
	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_09, pincfg); // DATA1
	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_10, pincfg); // DATA2
	hwpinctrl.PadSetup(PAD_GPIO_SD_B1_11, pincfg); // DATA3

	// The DMA channel 7 is used by the QSPI !
	txdma.Init(0x000007);  // perid = 0, DMA mux = 0
	// use the same channel for tx and rx
	rxdma.Init(0x000007);

	return true;
}


bool THwQspi_imxrt::Init()
{
	unsigned n;
	unsigned tmp;

	initialized = false;

	if (!InitInterface())
	{
		return false;
	}

	if (!txdma.initialized || !rxdma.initialized)
	{
		return false;
	}

	regs = FLEXSPI;

	// Configure Clock Speed

	unsigned baseclock = 480000000;
	unsigned div = baseclock / speed;

	if (div > 8)  div = 8;
	if (div < 1)  div = 1;
	--div;

	tmp = CCM->CSCMR1;
	tmp &= ~(3 << 29);
	tmp |=  (1 << 29);  // 0 = semc_clk_root, 1 = Use PLL3 (480MHz)
	tmp &= ~(7 << 23);
	tmp |=  (div << 23);
	CCM->CSCMR1 = tmp;

	// Configure FlexSPI

	imxrt_set_clock_gate(6, 10, 3); // enable the clock

	// enable the module
  regs->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

  // Reset
  regs->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
  while (regs->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
  {
  }

  // MCR0
  tmp = 0
  	| (0 << 11)   // HSEN: 1 = half speed access enable
  	| (0 <<  7)   // ATDFEN: 0 = TX FIFO on IP bus, 1 = TX FIFO on AHB bus
  	| (0 <<  6)   // ARDFEN: 0 = RX FIFO on IP bus, 1 = RX FIFO on AHB bus
  	| (0 <<  4)   // RXCLKSRC(2): 0 = no external clock feedback
  	| (0 <<  1)   // MDIS: 0 = module enabled
  	| (0 <<  0)   // SWRESET: 0 = no reset
  ;
  regs->MCR0 = ((regs->MCR0 & 0xFFFF0000) | tmp);  // keep the upper 16 bits !

  // MCR1
  regs->MCR1 = 0
  	| (0xFFFF << 16)  // SEQWAIT
		| (0xFFFF <<  0)  // AHBBUSWAIT
	;

  // MCR2
  regs->MCR2 = 0
		| (0xFF << 24)  // RESUMEWAIT
		| (0    << 19)  // SCKBDIFFOPT: 1 = SCKB is differential pair of SCKA
		| (0    << 15)  // SAMEDEVICEEN: 0 = individual mode
		| (0    << 11)  // CLRAHBBUFOPT ...
  ;

  // AHB control
  regs->AHBCR = 0
		| (1 << 6)  // READADDROPT: 1 = no aligment limitation
		| (0 << 5)  // PREFETCHEN: 1 = prefetch enabled
		| (0 << 4)  // BUFFERABLEEN: 0 = wait for finish
		| (0 << 3)  // CACHABLEEN: 0 = do not cache
		| (0 << 0)  // APAREN: 0 = individual mode
	;

  regs->INTEN = 0;

  // reset AHB rx buffers
  for (n = 0; n < 4; ++n)
  {
     regs->AHBRXBUFCR0[n] = 0;
  }

  // Configure IP Fifo-s
  regs->IPRXFCR = 0
		| (0 << 2)  // RXWMRK(4): RX fifo watermark
		| (0 << 1)  // RXDMAEN: 1 = DMA enabled
		| (1 << 0)  // CLRIPRXF: 1 = clear RX FIFO
	;

  regs->IPTXFCR = 0
		| (0 << 2)  // TXWMRK(4): TX fifo watermark
		| (0 << 1)  // TXDMAEN: 1 = DMA enabled
		| (1 << 0)  // CLRIPTXF: 1 = clear TX FIFO
	;

  // CONFIGURE FLASH DEVICE

  // Wait for bus idle before change flash configuration.
  WaitForIdle();

  regs->FLSHCR0[0] = 16484;  // fix 16MByte flash size
  // disable the other ports
  regs->FLSHCR0[1] = 0;
  regs->FLSHCR0[2] = 0;
  regs->FLSHCR0[3] = 0;

  regs->FLSHCR1[0] = 0
		| (2 << 16)  // CSINTERVAL(16)
		| (0 << 15)  // CSINTERVALUNIT: 0 = 1 serial clock
		| (0 << 11)  // CAS(4): column address size, 0 = no column address
		| (0 << 10)  // WA: 0 = not word addressable
		| (3 <<  5)  // TCSH(5): CS hold time
		| (3 <<  0)  // TCSS(5): CS setup time
	;

  // AHB Config
  regs->FLSHCR2[0] = 0
		| (0u << 31)  // CLRINSTRPTR
		| ( 0 << 28)  // AWRWAITUNIT(3): AHB wait unit, 0 = 2 AHB cycle
		| (20 << 16)  // AWRWAIT(12): wait cycles after command sequences
		| ( 0 << 13)  // AWRSEQNUM(3): 0 = single sequence WRITE command
		| (10 <<  8)  // AWRSEQID(4): AHB WRITE command sequence id
		| ( 0 <<  5)  // ARDSEQNUM(3): 0 = single sequence READ command
		| ( 8 <<  0)  // ARDSEQID(4): AHB READ command sequence id
	;

  // TODO: check FLEXSPI_ConfigureDll()
  regs->DLLCR[0] = 0
		| ( 0 << 9)  // OVRDVAL(6)
		| ( 0 << 8)  // OVRDEN
		| ( 0 << 3)  // SLVDLYTARGET(4)
		| ( 0 << 1)  // DLLRESET: 1 = reset delay
		| ( 0 << 0)  // DLLEN: 1 = DLL calibration enable
	;

  regs->FLSHCR4 = 0
		| ( 0 << 3)  // WMENB
		| ( 0 << 2)  // WMENA
		| ( 0 << 0)  // WMOPT1
	;

	// configure the LUT

  // Wait for bus idle before change flash configuration.
  WaitForIdle();

  // Unlock LUT for update.
  regs->LUTKEY = FLEXSPI_LUT_KEY_VAL;
  regs->LUTCR = 0x02;

  // copy the lut table
  unsigned * src = (unsigned *)&flexspi_lut_qspi_generic[0];
  unsigned * dst = (unsigned *)&regs->LUT[0];
  for (n = 0; n < 64; ++n)
  {
      *dst++ = *src++;
  }

  // Lock LUT
  regs->LUTKEY = FLEXSPI_LUT_KEY_VAL;
  regs->LUTCR = 0x01;

	// init DMA channels
	rxdma.Init((kDmaRequestMuxFlexSPIRx << 8) | rxdmachannel);
	txdma.Init((kDmaRequestMuxFlexSPITx << 8) | txdmachannel);

	txdma.Prepare(true,  (void *)&regs->TFDR[0], 0);
	rxdma.Prepare(false, (void *)&regs->RFDR[0], 0);

	initialized = true;

	return true;
}

int THwQspi_imxrt::StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

  WaitForIdle();

  // select sequence and coding

  unsigned cmd = (acmd & 0xFF);

	istx = false;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;

	unsigned seqid = 0;
	unsigned datasize = len;

	if      (0xEB == cmd)  // quad read
	{
		regs->IPCR0 = address;
		seqid = 10;
	}
	else if (0xBB == cmd)  // dual read
	{
		regs->IPCR0 = address;
		seqid = 9;
	}
	else if (0x0B == cmd)  // single read
	{
		regs->IPCR0 = address;
		seqid = 8;
	}
	else
	{
		if (acmd & 0x0F00)
		{
			// other multiline are not supported
			return ERROR_NOTIMPL;
		}

		seqid = 0;
		regs->IPCR0 = cmd;  // addr(7..0) = command
	}

	regs->INTR = 0xF0F; // clear interrupts

	regs->IPCR1 = 0
		| (0 << 24)         // ISEQNUM, 0 = only one sequence
		| (seqid << 16)     // ISEQID
		| (datasize <<  0)  // IDATSZ: data size in bytes
	;

  regs->IPRXFCR = 0
		| (0 << 2)  // RXWMRK(4): RX fifo watermark
		| (0 << 1)  // RXDMAEN: 1 = DMA enabled
		| (1 << 0)  // CLRIPRXF: 1 = clear RX FIFO
	;

	// decide to use DMA or not
	if (remainingbytes > 8)
	{
		dmaused = true;
  	regs->IPRXFCR |= (1 << 1);  // enable DMA

  	xfer.dstaddr = dataptr;
  	xfer.bytewidth = 8; // !!!
  	xfer.count = (remainingbytes >> 3);
  	xfer.addrinc = true;

  	dataptr += (xfer.count << 3);
  	remainingbytes &= 0x07;

  	rxdma.StartTransfer(&xfer);
	}
	else
	{
		dmaused = false;
	}

	regs->IPCMD = 1; // Start IP command

	busy = true;

	return ERROR_OK;
}

int THwQspi_imxrt::StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

  WaitForIdle();

  // select sequence and coding

  unsigned cmd = (acmd & 0xFF);

	istx = true;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;

	unsigned seqid = 0;
	unsigned datasize = len;

	if      (0x32 == cmd)  // QUAD Page Program
	{
		regs->IPCR0 = address;
		seqid = 13;
	}
	else if (0x02 == cmd)  // Single page program
	{
		regs->IPCR0 = address;
		seqid = 12;
	}
	else
	{
		if (acmd & 0x0F00)
		{
			// other multiline are not supported
			return ERROR_NOTIMPL;
		}

		regs->IPCR0 = cmd;  // addr(7..0) = command
		if (datalen == 0)
		{
			seqid = 2; // no data
		}
		else
		{
			seqid = 1; // with data
		}
	}

	regs->INTR = 0xF0F; // clear interrupts

	regs->IPCR1 = 0
		| (0 << 24)         // ISEQNUM, 0 = only one sequence
		| (seqid << 16)     // ISEQID
		| (datasize <<  0)  // IDATSZ: data size in bytes
	;

	dmaused = false;

  regs->IPTXFCR = 0
		| (0 << 2)  // TXWMRK(4): RX fifo watermark
		| (0 << 1)  // TXDMAEN: 1 = DMA enabled
		| (1 << 0)  // CLRIPTXF: 1 = clear RX FIFO
	;

	// decide to use DMA or not
	if (remainingbytes > 8)
	{
		dmaused = true;
  	regs->IPTXFCR |= (1 << 1);  // enable DMA

  	xfer.srcaddr = dataptr;
  	xfer.bytewidth = 8; // !!!
  	xfer.count = (remainingbytes >> 3);
  	xfer.addrinc = true;

  	dataptr += (xfer.count << 3);
  	remainingbytes &= 0x07;
	}
	else
	{
		dmaused = false;
	}

	regs->IPCMD = 1; // Start IP command

	busy = true;

	if (dmaused)
	{
  	txdma.StartTransfer(&xfer);
	}
	else
	{
		Run(); // to fill the transmit FIFO
	}

	return ERROR_OK;
}

void THwQspi_imxrt::Run()
{
	if (!busy)
	{
		return;
	}

	if (istx)
	{
		if (dmaused && txdma.Active())
		{
			return;
		}

		while ((remainingbytes > 0) && ((regs->IPTXFSTS & 0xFF) < 14))
		{
			// fill TX FIFO

			unsigned * srcp = (unsigned *)dataptr;

			if (remainingbytes > 8)
			{
				regs->TFDR[0] = *srcp++;
				regs->TFDR[1] = *srcp++;
				regs->INTR = FLEXSPI_INTR_IPTXWE_MASK; // push fifo
				remainingbytes -= 8;
			}
			else
			{
				// not a full line
				unsigned bline[2];
				memcpy(&bline[0], srcp, remainingbytes);
				srcp = (unsigned *)(unsigned(srcp) + remainingbytes);
				regs->TFDR[0] = bline[0];
				regs->TFDR[1] = bline[1];
				regs->INTR = FLEXSPI_INTR_IPTXWE_MASK; // push fifo
				remainingbytes = 0;
			}

			dataptr = (uint8_t *)srcp;
		}

		if ((regs->INTR & 1) == 0)
		{
			return;
		}
	}
	else
	{
		unsigned * dstp = (unsigned *)dataptr;

		while (!dmaused && (remainingbytes > 8) && ((regs->IPRXFSTS & 0xFF) > 0))
		{
			*dstp++ = regs->RFDR[0];
			*dstp++ = regs->RFDR[1];
			regs->INTR = FLEXSPI_INTR_IPRXWA_MASK; // pop fifo
			remainingbytes -= 8;

			dataptr = (uint8_t *)dstp;
		}

		if ((regs->INTR & 1) == 0)
		{
			return;
		}

		if (dmaused && rxdma.Active())
		{
			return;
		}

		if (remainingbytes > 0)
		{
			if ((regs->IPRXFSTS & 0xFF) == 0)
			{
				return;
			}

			// get the last line
			unsigned bline[2];
			bline[0] = regs->RFDR[0];
			bline[1] = regs->RFDR[1];
			memcpy(dataptr, &bline[0], remainingbytes);
			regs->INTR = FLEXSPI_INTR_IPRXWA_MASK; // pop fifo
			remainingbytes = 0;
		}
	}

	busy = false;
}

void THwQspi_imxrt::WaitForIdle()
{
  while ( ! ((regs->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (regs->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)) )
  {
  }
}
