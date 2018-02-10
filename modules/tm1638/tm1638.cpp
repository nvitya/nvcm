// tm1638.cpp

#include "platform.h"
#include "tm1638.h"

bool Ttm1638::Init()
{
	state = 0;
	nextstate = 0;
	shiftstate = 0;

	scancounter = 0;

	clock_sysclocks = SystemCoreClock / 1000000;  // 1 MHz clock
	ctrl = 0x0F;

	outregs[0] = 0x01;
	outregs[1] = 0x01;
	outregs[2] = 0x02;
	outregs[3] = 0x02;
	outregs[4] = 0x03;
	outregs[5] = 0x03;
	outregs[6] = 0x04;
	outregs[7] = 0x04;
	outregs[8] = 0x05;
	outregs[9] = 0x05;
	outregs[10] = 0x06;
	outregs[11] = 0x06;
	outregs[12] = 0x07;
	outregs[13] = 0x07;
	outregs[14] = 0x08;
	outregs[15] = 0x08;

	initialized = false;

	if (!dio_pin.Assigned() || !clk_pin.Assigned() || !stb_pin.Assigned())
	{
		return false;
	}

	dio_pin.Setup(PINCFG_OUTPUT);
	stb_pin.Setup(PINCFG_OUTPUT);
	clk_pin.Setup(PINCFG_OUTPUT);

  dio_pin.SwitchDirection(1); // set output
  dio_pin.Clear();
  stb_pin.Set();
  clk_pin.Set();

	initialized = true;

  return true;
}

void Ttm1638::Run()
{
	int i;

	if (shiftstate > 0)
	{
		switch (shiftstate)
		{
			case 0:	// do nothing
				break;

			case 1:	// start shifting
				stb_pin.Clear();
				dio_pin.SwitchDirection(1);;
				shiftbit = 0;
				shiftbyte = 0;
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 2: // wait a bit at start
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					++shiftstate;
				}
				break;

			case 3: // start data bit, setup clock
				dio_pin.SetTo(shiftdata[shiftbyte] >> shiftbit);
				clk_pin.Clear();
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 4: // wait for clock rising edge time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					clk_pin.Set();
					++shiftstate;
				}
				break;

			case 5: // wait for clock cycle end time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftbit;
					shiftstate = 3; // default: next bit

					if (shiftbit > 7)
					{
						// byte finished.
						++shiftbyte;
						if (shiftbyte >= shiftdatacnt)
						{
							shiftstate = 16; // write finished
						}
						else
						{
							shiftbit = 0;
						}
					}
				}
				break;

			case 16: // write finished
				if (readdatacnt <= 0)
				{
					shiftstate = 32; // end
				}
				else
				{
					// switch direction
					dio_pin.SwitchDirection(0);
					sstarttime = CLOCKCNT;
					shiftbit = 0;
					shiftbyte = 0;
					for (i = 0; i < 4; ++i)  readdata[i] = 0;
					shiftstate = 20;
				}
				break;

			case 20: // wait wait time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftstate;
				}
				break;

			case 21: // start data bit, setup clock
				clk_pin.Clear();
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 22: // wait for clock rising edge time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					clk_pin.Set();
					// sample the data
					readdata[shiftbyte] |= (dio_pin.Value() << shiftbit);

					++shiftstate;
				}
				break;

			case 23: // wait for clock cycle end time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftbit;
					shiftstate = 21; // default: next bit

					if (shiftbit > 7)
					{
						// byte finished.
						++shiftbyte;
						if (shiftbyte >= readdatacnt)
						{
							// transaction finished.
							shiftstate = 32; // byte finished
						}
						else
						{
							shiftbit = 0;
						}
					}
				}
				break;

			case 32: // finish transaction
				stb_pin.Set();
				sstarttime = CLOCKCNT;
				state = nextstate;
				shiftstate = 0;  // stop;
				break;

			case 33: // hold strobe
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					state = nextstate;
					shiftstate = 0;  // stop;
				}
				break;
		}
	}
	else
	{
		// handle main state

		nextstate = state + 1;  // step forward by default

		switch (state)
		{
			case 0:	// update output registers
				shiftdata[0] = 0x40;  // write data with autoincrement address
				shiftdatacnt = 1;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 1:	// update output registers
				shiftdata[0] = 0xC0;  // set address + data

				for (i = 0; i < 16; ++i)
				{
					shiftdata[i+1] = outregs[i];
				}

				// start shifting
				shiftdatacnt = 17;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 2:  // set control register
				shiftdata[0] = 0x80 | (ctrl & 0x0F);
				shiftdatacnt = 1;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 3:  // read data
				shiftdata[0] = 0x42;
				shiftdatacnt = 1;
				readdatacnt = 4;
				shiftstate = 1;
				break;

			case 4:  // process read data
				for (i = 0; i < 4; ++i)
				{
					inregs[i] = readdata[i];
				}
				++scancounter; // increment the scan counter to help processing the buttons
				state = 0;
				break;
		}
	}
}
