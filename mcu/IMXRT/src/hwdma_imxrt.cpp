// hwdma_imxrt.cpp

#include "hwpins.h"
#include "hwdma.h"
#include "imxrt_utils.h"

#include "traces.h"

//TDmaDesc  dmachtable[32] __attribute__((aligned(512)));  // must be aligned to 512 byte boundary !

// for request sources see dma_request_source_t in MIMXRT105x.h

bool THwDmaChannel_imxrt::Init(int achnum)
// achnum:
//   bits0..7: chnum = 0..31
//   bits8..15: DMA request sourceid see dma_request_source_t in MIMXRT105x.h
//   bit16: 1 = use DMA request source
{
	unsigned tmp;

	imxrt_set_clock_gate(5, 6, 3);  // // Enable eDMA Clock

	DMA0->CR |= (1 << 7); // EMLM: 1 = enable minor loop mapping, activates MLOFF in the NBYTES registers

	chnum = achnum & 0x1F;
	regs = (HW_DMA_REGS *)&(DMA0->TCD[chnum]);

	// Set DMA request source
	if (achnum & 0x10000)
	{
		perid = ((achnum >> 8) & 0x7F);
		DMAMUX->CHCFG[chnum] = (perid | (1u << 31)); // set source id + enable
	}
	else
	{
		perid = -1; // no peripheral id routing (mem2mem)
	}

	// General settings
	tmp = DMA0->CR;
	tmp &= ~(DMA_CR_ERCA_MASK | DMA_CR_ERGA_MASK); // fixed arbitration
	tmp |= DMA_CR_HOE_MASK;   // Halt on error
	tmp &= ~DMA_CR_CLM_MASK;  // disable Continous Link Mode
	tmp &= ~DMA_CR_EDBG_MASK; // keep running during debug
	DMA0->CR = tmp;

	// do not change the channel priority:
	//  by default the channels prioritized by their numbers

	//unsigned char * ppreg = (unsigned char *)&(DMA0->DCHPRI3); // the first address
	//ppreg += 4 * (chnum >> 4);
	//ppreg += (3 - (chnum & 3));
	//*ppreg = priority;

	Prepare(true, nullptr, 0); // set some defaults

	initialized = true;

	return true;
}

void THwDmaChannel_imxrt::Disable()
{
	DMA0->CERQ = chnum;
}

void THwDmaChannel_imxrt::Enable()
{
	//DMA0->CDNE = chnum;
	DMA0->CERR = chnum;
	DMA0->CINT = chnum;
	// enable peripheral request:
	DMA0->SERQ = chnum;
}

bool THwDmaChannel_imxrt::Enabled()
{
	//return ((regs->CSR & DMA_CSR_DONE_MASK) != 0);
	return ((DMA0->ERQ & chnum) != 0);
}

bool THwDmaChannel_imxrt::Active()
{
	return ((regs->CSR & DMA_CSR_DONE_MASK) == 0);
}

void THwDmaChannel_imxrt::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;
}

bool THwDmaChannel_imxrt::StartTransfer(THwDmaTransfer * axfer)
{
	//Disable();
	//unsigned t0, t1;
	//t0 = DWT_CYCCNT;

	unsigned tmp;

	unsigned sdsize;
	if (axfer->bytewidth == 4)
	{
		sdsize = 2;
	}
	else if (axfer->bytewidth == 1)
	{
		sdsize = 0;
	}
	else if (axfer->bytewidth == 2)
	{
		sdsize = 1;
	}
	else if (axfer->bytewidth == 8)
	{
		sdsize = 3;
	}
	else
	{
		sdsize = 0;
	}


	if (istx)
	{
		regs->SADDR = unsigned(axfer->srcaddr);
		regs->DADDR = unsigned(periphaddr);
		regs->SOFF = axfer->bytewidth;
		regs->DOFF = 0;
	}
	else
	{
		regs->SADDR = unsigned(periphaddr);
		regs->DADDR = unsigned(axfer->dstaddr);
		regs->SOFF = 0;
		regs->DOFF = axfer->bytewidth;
	}

#if 0
	regs->NBYTES = axfer->count * axfer->bytewidth;
	regs->CITER = 1; //axfer->count;
	regs->BITER = 1; //axfer->count;
#endif

#if 1
	regs->NBYTES = axfer->bytewidth;
	regs->CITER = axfer->count;
	regs->BITER = axfer->count;
#endif

	regs->SLAST = 0;
	regs->DLAST_SGA = 0;  // alternatively next TCD, when CSR.ESG == 1

	regs->ATTR = sdsize | (sdsize << 8);

	tmp = 0
	  | (0 << 14)  // BWC(2): Bandwidth Control, 0 = no engine stalls, 2 = 4 cycles after each RW, 3 = 8 cycles
		| (0 <<  8)  // MAJORCHLINK(5): Major Loop Link channel number
		| (0 <<  7)  // DONE: Channel Done
		| (0 <<  6)  // ACTIVE: Channel Active
		| (0 <<  5)  // MAJORELINK: Enable channel-channel linking on major loop complete
		| (0 <<  4)  // ESG: Enable Scatter/Gather (linked list)
		| (1 <<  3)  // DREQ: 1 = clear the hw request enable on complete
		| (0 <<  2)  // INTHALF: 1 = enable half major counter interrupt
		| (0 <<  1)  // INTMAJOR: 1 = enable interrupt when the major loop completed
		| (0 <<  0)  // START: software start request
	;

	regs->CSR = tmp;

	Enable();

	//t1 = DWT_CYCCNT;
	//TRACE("DMA Starttransfer = %u\r\n", t1 - t0);

	return true;
}
