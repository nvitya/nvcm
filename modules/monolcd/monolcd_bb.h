// monolcd_bb.h

#ifndef MONOLCD_BB_H_
#define MONOLCD_BB_H_

#include "monolcd.h"
#include "hwpins.h"

class TMonoLcd_bb : public TMonoLcd
{
public:
	// interface dependent
	TGpioPin        pin_ce;
	TGpioPin        pin_clk;
	TGpioPin        pin_din;
	TGpioPin 	      pin_reset;

	int             update_byte_batch = 16;  // determines the CPU time in the Run()

	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);

	//virtual void WriteData8(uint8_t adata);
	//virtual void WriteData16(uint16_t adata);

	//virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	//virtual void FillColor(uint16_t acolor, unsigned acount);

	virtual void Run(); // constantly updates the display

protected:
	uint8_t *       dataptr;
	uint32_t        bytes_remaining;

};



#endif /* MONOLCD_BB_H_ */
