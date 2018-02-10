// tm1638.h

#ifndef SRC_TM1638_H_
#define SRC_TM1638_H_

#include "hwpins.h"
#include "clockcnt.h"

class Ttm1638
{
public:
	int  state;
	int  nextstate;

	unsigned      clock_sysclocks;

	unsigned char outregs[16];
	unsigned char inregs[4];

	unsigned      scancounter;

	unsigned char ctrl;  // bits:  0000abbb, a = activate display, bbb = brightness

	TGpioPin      dio_pin;
	TGpioPin      clk_pin;
	TGpioPin      stb_pin;

	bool          initialized = false;

	bool          Init();
	void          Run();

protected:
	clockcnt_t		 sstarttime;

	int            shiftstate;
	int            shiftbit;
	int            shiftbyte;
	unsigned char  shiftdata[32];
	unsigned char  readdata[4];
	int            shiftdatacnt;
	int            readdatacnt;

/*
	void set_data(unsigned char avalue);
	void set_clock(unsigned char avalue);
	void set_strobe(unsigned char avalue);
	unsigned char get_data();
	void datadir_output(bool aoutput);
*/
};

#endif /* SRC_TM1638_H_ */
