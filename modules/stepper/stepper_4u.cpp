// stepper_4u.cpp

#include "stepper_4u.h"
#include "clockcnt.h"

bool TStepper_4u::Init()
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		if (!pin[i].Assigned())  return false;
	}

	for (i = 0; i < 4; ++i)
	{
		pin[i].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	}

	ctrl_pattern[0] = 0x03;
	ctrl_pattern[1] = 0x06;
	ctrl_pattern[2] = 0x0C;
	ctrl_pattern[3] = 0x09;
	ctrl_len = 4;

	step_time_clocks = (SystemCoreClock / 1000000) * step_time_us;

	return true;
}

void TStepper_4u::Run()
{
	int i;

	switch (state)
	{
	case 0:
		if (actual_pos < position)
		{
			// forward
			if (ctrl_idx >= ctrl_len - 1)
			{
				ctrl_idx = 0;
			}
			else
			{
				++ctrl_idx;
			}
			SetCtrlPattern(ctrl_pattern[ctrl_idx]);
			starttime = CLOCKCNT;
			state = 10; // hold;
		}
		else if (actual_pos > position)
		{
			if (ctrl_idx == 0)
			{
				ctrl_idx = ctrl_len - 1;
			}
			else
			{
				--ctrl_idx;
			}
			SetCtrlPattern(ctrl_pattern[ctrl_idx]);
			starttime = CLOCKCNT;
			state = 20; // hold
		}
		else
		{
			SetCtrlPattern(0);
		}
		break;

	case 10:
		if (CLOCKCNT - starttime >= step_time_clocks)
		{
			++actual_pos;
			state = 0;
		}
		break;

	case 20:
		if (CLOCKCNT - starttime >= step_time_clocks)
		{
			--actual_pos;
			state = 0;
		}
		break;

	}
}

void TStepper_4u::SetCtrlPattern(uint8_t apat)
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		pin[i].SetTo((apat >> i) & 1);
	}
}
