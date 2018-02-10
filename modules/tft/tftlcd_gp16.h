// tftlcd_gp16.h

#ifndef TFT_TFTLCD_GP16_H_
#define TFT_TFTLCD_GP16_H_

#include "tftlcd.h"
#include "hwpins.h"

class TTftLcd_gp16: public TTftLcd
{
public:
	typedef TTftLcd super;

	unsigned   data_hold_clocks = 32;

	// required hw resources
	TGpioPin   pin_cs;
	TGpioPin   pin_wr;
	TGpioPin   pin_cd;
	TGpioPin 	 pin_reset;  // unassigned

	TGpioPin   pin_d[16];

public:
	// interface dependent

	virtual bool InitInterface();
	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);

	// local
	void SetData8(uint16_t adata);
	void SetData16(uint16_t adata);

};

#endif /* TFT_TFTLCD_GP16_H_ */
