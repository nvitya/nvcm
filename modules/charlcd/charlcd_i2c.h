// charlcd_i2ci.h

// driver for the character display connected over an I2C extender.
// using 4 bit mode

#ifndef CHARLCD_I2C_H_
#define CHARLCD_I2C_H_

#include "charlcd.h"
#include "hwi2c.h"

class TCharLcd_i2c : public TCharLcd
{
public:
	bool            backlight_on = true;

public:
	// interface dependent
	THwI2c          i2c;

	uint8_t         i2caddr = 0x3F;  // the default address of the I2C extender

	uint8_t         pinmask_rs = 0x01;
	uint8_t         pinmask_rw = 0x02;
	uint8_t         pinmask_en = 0x04;
	uint8_t         pinmask_bl = 0x08;
	// the data bits are b7..4;

	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData(uint8_t adata);

protected:

	uint8_t         base_bits = 0;
	uint8_t         cmdbuf[4];

	void Write4Bits(uint8_t acmd);
	void PulseEn(uint8_t abasedata);
	void ExpanderWrite(uint8_t adata);

};

#endif /* CHARLCD_BB4_H_ */
