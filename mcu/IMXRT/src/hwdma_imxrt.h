// hwdma_imxrt.h

#ifndef HWDMA_IMXRT_H_
#define HWDMA_IMXRT_H_

#define HWDMA_PRE_ONLY
#include "hwdma.h"

typedef struct DMA_TCD_Type
{
  __IO uint32_t SADDR;
  __IO uint16_t SOFF;
  __IO uint16_t ATTR;
  __IO uint32_t NBYTES;
  __IO uint32_t SLAST;
  __IO uint32_t DADDR;
  __IO uint16_t DOFF;
  __IO uint16_t CITER;
  __IO uint32_t DLAST_SGA;
  __IO uint16_t CSR;
  __IO uint16_t BITER;
//
} DMA_TCD_Type;

#define HW_DMA_REGS      	DMA_TCD_Type

class THwDmaChannel_imxrt : public THwDmaChannel_pre
{
public:
	unsigned           chbit = 0;
	int                perid = -1;
	HW_DMA_REGS *      regs = nullptr;

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

#define HWDMACHANNEL_IMPL  THwDmaChannel_imxrt

#endif // def HWDMA_IMXRT_H_
