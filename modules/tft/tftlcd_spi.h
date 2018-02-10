// tftlcd_spi.h

#ifndef TFTLCD_SPI_H_
#define TFTLCD_SPI_H_

#include "tftlcd.h"
#include "hwpins.h"
#include "hwspi.h"
#include "hwdma.h"

#define TFTLCD_SPI_DMA_BUFSIZE  256

class TTftLcd_spi: public TTftLcd
{
public:
	typedef TTftLcd super;

	// required hw resources:
	THwSpi     spi;
	TGpioPin   pin_cs;
	TGpioPin   pin_cd;
	TGpioPin 	 pin_reset;  // unassigned

	THwDmaChannel   txdma;
	THwDmaTransfer  dmaxfer;

	uint8_t cmdbuf[4];
	uint8_t databuf[TFTLCD_SPI_DMA_BUFSIZE];

public:
	// interface dependent

	virtual bool InitInterface();
	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);


};

#endif /* TFTLCD_SPI_H_ */
