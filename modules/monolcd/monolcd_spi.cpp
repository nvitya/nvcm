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
	switch (updatestate)
	{
	case 0: // wait for update request
		if (lastupdate == updatecnt)
		{
			return;
		}

		lastupdate = updatecnt;
		current_page = 0;
		dataptr = pdispbuf;
		updatestate = 2;

		// no break !

	case 2: // send commands
		pin_cd.Set0(); // set to command
		if (MLCD_CTRL_PCD8544 == ctrltype)
		{
			cmdbuf[0] = 0x40 | current_page; // set y address
			cmdbuf[1] = 0x80 | 0; // set column start address / MSB
			cmdcnt = 2;
		}
		else
		{
			cmdbuf[0] = 0x00 + (rotation == 2 ? 4 : 0); // set column start address / LSB
			cmdbuf[1] = 0x10; // set column start address / MSB
			cmdbuf[2] = 0xB0 + current_page; // set page address
			cmdcnt = 3;
		}

		cmdidx = 0;

		pin_cs.Set0();
		updatestate = 3;
		// no break !

	case 3: // sending command bytes
		while (cmdidx < cmdcnt)
		{
			if (!spi.TrySendData(cmdbuf[cmdidx]))
			{
				return;
			}
			++cmdidx;
		}

		if (!spi.SendFinished())
		{
			return;
		}

		pin_cs.Set1();
		updatestate = 4;
		// no break !

	case 4: // start send row data

		row_remaining = hwwidth;
		pin_cd.Set1();
		pin_cs.Set0();
		updatestate = 5;

		// no break;

	case 5: // sending row data

		while (row_remaining > 0)
		{
			if (!spi.TrySendData(*dataptr))
			{
				return;
			}
			++dataptr;
			--row_remaining;
		}

		if (!spi.SendFinished())
		{
			return;
		}

		pin_cs.Set1();

		// go to the next row
		++current_page;
		if (current_page < (hwheight >> 3))
		{
			updatestate = 2; Run(); return;
		}
		else
		{
			// the end.
			updatestate = 0;
		}
		break;
	}
}
