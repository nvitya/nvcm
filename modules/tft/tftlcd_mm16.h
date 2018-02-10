// tftlcd_mm.h

#ifndef TFT_TFTLCD_MM16_H_
#define TFT_TFTLCD_MM16_H_

#include "tftlcd.h"

class TTftLcd_mm16: public TTftLcd
{
public:
	typedef TTftLcd super;

	volatile uint16_t *  cmdreg = nullptr;
	volatile uint16_t *  datareg = nullptr;

public:
	// interface dependent

	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);

};

#endif /* TFT_TFTLCD_MM_H_ */
