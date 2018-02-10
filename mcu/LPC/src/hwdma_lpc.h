// hwdma_lpc.h

#ifndef HWDMA_LPC_H_
#define HWDMA_LPC_H_

#define HWDMA_PRE_ONLY
#include "hwdma.h"

class THwDmaChannel_lpc : public THwDmaChannel_pre
{
public:
	unsigned           chbit = 0;
	int                perid = -1;
	HW_DMA_REGS *      regs = nullptr;
#ifdef HW_DMA_ALT_REGS
  HW_DMA_ALT_REGS *  altregs = nullptr;  // Some Atmel systems have two different DMA system
#endif

	bool Init(int achnum);
	bool InitPeriphDma(bool aistx, void * aregs, void * aaltregs);  // special function for Atmel PDMA

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);
	void Disable();
	void Enable();

	bool Enabled();
	bool Active();

	bool StartTransfer(THwDmaTransfer * axfer);
	bool StartMemToMem(THwDmaTransfer * axfer);
};

#define HWDMACHANNEL_IMPL  THwDmaChannel_lpc

#endif // def HWDMA_LPC_H_
