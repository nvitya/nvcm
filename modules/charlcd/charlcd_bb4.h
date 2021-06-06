// charlcd_bb4.h

// driver for the character display connected over GPIO sinals, 4-bit mode

#ifndef CHARLCD_BB4_H_
#define CHARLCD_BB4_H_

#include "charlcd.h"
#include "hwpins.h"

class TCharLcd_bb4 : public TCharLcd
{
public:
	// these pins must be assigned
	TGpioPin        pin_en;
	TGpioPin        pin_rs;
	TGpioPin        pin_rw;  // can be tied to 0 or left uninitialized
	TGpioPin        pin_data[4];  // actually connected to D7..D4



	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData(uint8_t adata);

protected:

	void SetDataBits(uint8_t adata);
	void Write4Bits(uint8_t acmd);
};

#endif /* CHARLCD_BB4_H_ */
