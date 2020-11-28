// monolcd_spi.h

#ifndef SRC_MONOLCD_SPI_H_
#define SRC_MONOLCD_SPI_H_

#include "monolcd.h"
#include "hwspi.h"

class TMonoLcd_spi : public TMonoLcd
{
public:
	// interface dependent
	THwSpi          spi;
	TGpioPin        pin_cs;
	TGpioPin        pin_cd;
	TGpioPin 	      pin_reset;  // unassigned

	THwDmaChannel   txdma;
	THwDmaTransfer  dmaxfer;

	uint8_t         cmdbuf[16];
	uint8_t         cmdcnt;
	uint8_t         cmdidx;

	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData(uint8_t adata);

	//virtual void WriteData8(uint8_t adata);
	//virtual void WriteData16(uint16_t adata);

	//virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	//virtual void FillColor(uint16_t acolor, unsigned acount);

	virtual void Run(); // constantly updates the display

protected:
	uint8_t         current_page;
	uint8_t *       dataptr;
	uint32_t        row_remaining;


};

#endif /* SRC_MONOLCD_SPI_H_ */
