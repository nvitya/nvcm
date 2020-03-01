// monolcd_bb.cpp

#include "monolcd_bb.h"
#include "clockcnt.h"

#define LCD_BB_DELAY()  // no delay by default

bool TMonoLcd_bb::InitInterface()
{
	if (!pin_clk.Assigned()	|| !pin_din.Assigned() || !pin_din.Assigned())
	{
		return false;
	}

	if (pin_reset.Assigned())
	{
		delay_us(5000);
		pin_reset.Set0();
		delay_us(5000);
		pin_reset.Set1();
		delay_us(10000);
	}

	pin_clk.Set0();
	pin_ce.Set1();

	return true;
}

void TMonoLcd_bb::WriteCmd(uint8_t adata)
{
	pin_ce.Set0();
	pin_din.Set0(); // the signalises command byte
	LCD_BB_DELAY();
	pin_clk.Set1();
	LCD_BB_DELAY();
	pin_clk.Set0();

	uint8_t d = adata;
	for (int i = 0; i < 8; ++i)
	{
		pin_din.SetTo(d >> (7 - i));
		LCD_BB_DELAY();
		pin_clk.Set1();
		LCD_BB_DELAY();
		pin_clk.Set0();
	}
	pin_ce.Set1();
}

void TMonoLcd_bb::Run()
{
	switch (updatestate)
	{
	case 0: // wait for update request
		if (lastupdate == updatecnt)
		{
			return;
		}

		lastupdate = updatecnt;
		dataptr = pdispbuf;

		WriteCmd(0xB0);  // set Y to 0
		WriteCmd(0x10);  // set X to 0
		WriteCmd(0x00);

		updatestate = 5;
		bytes_remaining = ((width * ((height + 7) & 0x1F8)) >> 3);

		// no break !

	case 5: // sending data

		int batch_remaining = update_byte_batch;

		pin_ce.Set0();

		while (true)
		{
			pin_din.Set1(); // the signalises data byte
			LCD_BB_DELAY();
			pin_clk.Set1();
			LCD_BB_DELAY();
			pin_clk.Set0();

			uint8_t d = *dataptr;
			for (int i = 0; i < 8; ++i)
			{
				pin_din.SetTo(d >> (7 - i));
				LCD_BB_DELAY();
				pin_clk.Set1();
				LCD_BB_DELAY();
				pin_clk.Set0();
			}

			++dataptr;

			if (bytes_remaining > 0)
			{
				--bytes_remaining;
				if (batch_remaining > 0)
				{
					--batch_remaining;
				}
				else
				{
					break;  // leave the CPU to do some other stuff
				}
			}
			else
			{
				updatestate = 0;
				break;
			}
		} // while batch byte processing

		pin_ce.Set1();
		break;
	}
}

