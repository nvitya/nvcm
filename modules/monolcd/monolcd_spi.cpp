// monolcd_spi.cpp

#include "monolcd_spi.h"
#include "clockcnt.h"

bool TMonoLcd_spi::InitInterface()
{
	if (!spi.initialized
			|| !pin_cs.Assigned()
			|| !pin_cd.Assigned())
	{
		return false;
	}

	if (pin_reset.Assigned())
	{
		pin_reset.Set0();
		delay_us(100000);
		pin_reset.Set1();
	}

  // Reset the controller state...
	pin_cd.Set0();
	pin_cs.Set0();
	delay_us(100000);
	pin_cs.Set1();

	return true;
}

void TMonoLcd_spi::WriteCmd(uint8_t adata)
{
	pin_cd.Set0();
	pin_cs.Set0();
	spi.SendData(adata);
	spi.WaitSendFinish();
	pin_cs.Set1();
}

void TMonoLcd_spi::Run()
{
	uint32_t page;
	uint32_t col;
	uint8_t * dp = pdispbuf;

	switch (updatestate)
	{
	case 0: // wait for update request
		if (lastupdate != updatecnt)
		{
			updatestate = 1;  Run();  return;
		}
		break;

	case 1: // start update
		updatestate = 5;
		break;

	case 5: // start sending data
		lastupdate = updatecnt;


		for (page = 0; page < (hwheight >> 3); ++page)
		{
			WriteCmd(0x04); // set column start address / LSB
			WriteCmd(0x10); // set column start address / MSB
			WriteCmd(0xB0 + page); // set page address
			pin_cd.Set1();
			pin_cs.Set0();
			for (col = 0; col < hwwidth; ++col)
			{
				spi.SendData(*dp++);
			}
			spi.WaitSendFinish();
			pin_cs.Set0();
		}

		updatestate = 6;
		break;

	case 6: // wait until the buffer write finished
		updatestate = 0; // return the idle state
		break;
	}
}
