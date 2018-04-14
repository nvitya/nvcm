// oleddisp_i2c.h

#ifndef SRC_OLEDDISP_I2C_H_
#define SRC_OLEDDISP_I2C_H_

#include "oleddisp.h"
#include "hwi2c.h"

class TOledDisp_i2c : public TOledDisp
{
public:
	// interface dependent
	uint8_t   i2caddress = 0x3C;  // alternatively 0x3D

	THwI2c    i2c;

	uint32_t  commerrcnt = 0;

	uint8_t   cmdbuf[16];

	virtual bool InitInterface();

	virtual void WriteCmd(uint8_t adata);

	//virtual void WriteData8(uint8_t adata);
	//virtual void WriteData16(uint16_t adata);

	//virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	//virtual void FillColor(uint16_t acolor, unsigned acount);

	virtual void Run(); // constantly updates the display
};

#endif /* SRC_OLEDDISP_I2C_H_ */
