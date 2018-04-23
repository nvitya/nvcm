/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     tftlcd.cpp
 *  brief:    TFT LCD Display driver support base class
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "tftlcd.h"

#include "clockcnt.h"

bool TTftLcd::Init(TLcdCtrlType atype, uint16_t awidth, uint16_t aheight)
{
	InitGfx();

	ctrltype = atype;
	hwwidth = awidth;
	hwheight = aheight;

	InitInterface();

	InitLcdPanel();

	return true;
}

bool TTftLcd::InitInterface()
{
	// should be overridden
	return false;
}

void TTftLcd::ResetPanel()
{
	// should be overridden
}

void TTftLcd::WriteCmd(uint8_t adata)
{
	// should be overridden
}

void TTftLcd::WriteData8(uint8_t adata)
{
	// should be overridden
}

void TTftLcd::WriteData16(uint16_t adata)
{
	// should be overridden
}

#define TFTLCD_DELAY 0xFE

// I found these initialization command lists in several different (Arduino) libraries (credits for Adafruit)
// In some level the panels also work without these initializations

static const uint8_t lcd_init_cmdlist_7735R[] =
{
    0xB1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    0xB2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    0xB3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    0xB4 , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    0xC0 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    0xC1 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    0xC2 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    0xC3 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,
    0xC4 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    0xC5 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
// Rcmd3:
		0xE0, 16      , //  Positive gamma control
			0x02, 0x1c, 0x07, 0x12,
			0x37, 0x32, 0x29, 0x2d,
			0x29, 0x25, 0x2B, 0x39,
			0x00, 0x01, 0x03, 0x10,
		0xE1, 16      , //  Negative gamma control
			0x03, 0x1d, 0x07, 0x06,
			0x2E, 0x2C, 0x29, 0x2D,
			0x2E, 0x2E, 0x37, 0x3F,
			0x00, 0x00, 0x02, 0x10,
    0xFF // close the list
};

static const uint8_t lcd_init_cmdlist_ILI9486[] =
{
		0xF2, 9,		// ?????
		  0x1C, 0xA3, 0x32, 0x02, 0xb2, 0x12, 0xFF, 0x12, 0x00,
		0xF1, 2,		// ?????
		  0x36, 0xA4,
		0xF8, 2,		// ?????
		  0x21, 0x04,
		0xF9, 2,		// ?????
		  0x00, 0x08,
		0xC0, 2,		// Power Control 1
		  0x0d, 0x0d,
		0xC1, 2,		// Power Control 2
		  0x43, 0x00,
		0xC2, 1,		// Power Control 3
		  0x00,
		0xC5, 2,		// VCOM Control
		  0x00, 0x48,
		0xB6, 3,		// Display Function Control
		  0x00,
		  0x22,			// 0x42 = Rotate display 180 deg.
		  0x3B,
		0xE0, 15,		// PGAMCTRL (Positive Gamma Control)
		  0x0f, 0x24, 0x1c, 0x0a, 0x0f, 0x08, 0x43, 0x88, 0x32, 0x0f, 0x10, 0x06, 0x0f, 0x07, 0x00,
		0xE1, 15,		// NGAMCTRL (Negative Gamma Control)
		  0x0F, 0x38, 0x30, 0x09, 0x0f, 0x0f, 0x4e, 0x77, 0x3c, 0x07, 0x10, 0x05, 0x23, 0x1b, 0x00,
    0xFF // close the list
};

static const uint8_t lcd_init_cmdlist_ILI9341[] =
{
		0xEF, 3,
	    0x03, 0x80, 0x02,
	  0xCF, 3,
	    0x00, 0XC1, 0X30,
	  0xED, 4,
	    0x64, 0x03, 0X12,
	    0X81,
	  0xE8, 3,
	    0x85, 0x00, 0x78,
	  0xCB, 5,
	    0x39, 0x2C, 0x00, 0x34, 0x02,
	  0xF7, 1,
	    0x20,
	  0xEA, 2,
	    0x00, 0x00,
	  0xC0, 1,   //Power control
	    0x23,    //VRH[5:0]
	  0xC1, 1,   //Power control
	    0x10,    //SAP[2:0];BT[3:0]
	  0xC5, 2,   //VCM control
	    0x3e, 0x28,
	  0xC7, 1,   //VCM control2
	    0x86,    //--
	  0x36, 1,   // Memory Access Control
	    0x48,
	  0x37, 2,   // Vertical scroll
	    0, 0,       // Zero
	  0x3A, 1,
	    0x55,
	  0xB1, 2,
	    0x00, 0x18,
	  0xB6, 3,   // Display Function Control
	    0x08, 0x82, 0x27,
	  0xF2, 1,   // 3Gamma Function Disable
	    0x00,
	  0x26, 1,   //Gamma curve selected
	    0x01,
	  0xE0, 15,  //Set Gamma
	    0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
	  0xE1, 15,  //Set Gamma
	    0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,

		0xFF  // end of the list
};

static const uint8_t lcd_init_cmdlist_HX8357D_old[] =
{
	0xB9, 3,
    0xFF, 0x83, 0x57,
  TFTLCD_DELAY,
  	250,
  0xB3, 4,
    0x00, 0x00, 0x06, 0x06,
  0xB6, 1,
    0x25,  // -1.52V
  0xB0, 1,
    0x68,  // Normal mode 70Hz, Idle mode 55 Hz
  0xCC, 1,
    0x05,  // BGR, Gate direction swapped
//  0xB1, 6, // PWR1
//    0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA,
//  0xC0, 6, // STBA
//    0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08,
//  0xB4, 7,
//    0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78,
#if 1
  0x3A, 1,
    0x55,
  0x36, 1,
    0xC0,
  0x35, 1,
    0x00,
  0x44, 2,
    0x00, 0x02,

#endif

  0xFF // end of the list
};

static const uint8_t lcd_init_cmdlist_HX8357B[] =
{
//  0xD0, 3,               // my panel did not work with this
//    0x07, 0x42, 0x18,
  0xD1, 3,
    0x00, 0x07, 0x10,
  0xD2, 2,
    0x01, 0x02,
  0xC0, 5,
    0x10, 0x3B, 0x00, 0x02, 0x11,
  0xC5, 1,
    0x08,
  0xC8, 12,
    0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00,
  0x36, 1,
    0x0a,
  0x29, 0,
    //
  0xFF
};


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in flash memory.
void TTftLcd::RunCommandList(const uint8_t * addr)
{
  uint8_t  argnum;
  uint16_t ms;

  uint8_t cmd = *addr++;
  while (cmd != 0xFF) // the list is closed with 0xFF
  {
  	if (cmd == TFTLCD_DELAY)
  	{
  		ms = *addr++; // read the delay
  		if (ms == 255) ms = 500;
  		delay_ms(ms);
  	}
  	else
  	{
			WriteCmd(cmd);
			argnum = *addr++;      // argument count + delay
			while (argnum--)
			{
				WriteData8(*addr++);
			}
  	}

  	// get the next cmd
  	cmd = *addr++;
  }
}


void TTftLcd::InitLcdPanel()
{
	ResetPanel();

	// universal minimal init for all LCD panels

  WriteCmd(0x01);   // soft reset
	delay_ms(100);

	WriteCmd(0x11);		// Sleep OUT
	delay_ms(100);

	if (ctrltype == LCD_CTRL_ILI9341)
	{
		RunCommandList(&lcd_init_cmdlist_ILI9341[0]);
	}
	else if (ctrltype == LCD_CTRL_ST7735)
	{
		RunCommandList(&lcd_init_cmdlist_7735R[0]);
	}
	else if (ctrltype == LCD_CTRL_ILI9486)
	{
		RunCommandList(&lcd_init_cmdlist_ILI9486[0]);
	}
	else if (ctrltype == LCD_CTRL_HX8357B)
	{
		RunCommandList(&lcd_init_cmdlist_HX8357B[0]);
	}

	WriteCmd(0x3A);		// Set Interface Pixel Format
	WriteData8(0x55);	// 16 Bit

  WriteCmd(0x20);   // Inversion off

  WriteCmd(0x13);    // Normal display on
  delay_ms(10);

  WriteCmd(0x29);    // Display on
  delay_ms(100);

	// set the default rotation
	SetRotation(0);

	// init the address window
	SetAddrWindow(0, 0, width, height);
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void TTftLcd::SetRotation(uint8_t m)
{
	WriteCmd(0x36);  // MADCTL

	uint16_t c = MADCTL_BGR;

  rotation = m % 4; // can't be higher than 3
  switch (rotation)
  {
  case 0:
    width  = hwwidth;
    height = hwheight;
    break;

  case 1:
  	c |= MADCTL_MX | MADCTL_MV;
    width = hwheight;
    height = hwwidth;
    break;

  case 2:
  	c |= MADCTL_MX | MADCTL_MY;
    width  = hwwidth;
    height = hwheight;
    break;

  case 3:
  	c |= MADCTL_MY | MADCTL_MV;
    width = hwheight;
    height = hwwidth;
    break;
  }

  if (mirrorx)  c ^= MADCTL_MX;

  WriteData8(c);
}

void TTftLcd::DrawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) return;

  SetAddrWindow(x,y,1,1);
  WriteData16(color);
}

extern const unsigned char font_bitmap_data_5x8[1280];  // in fontdata.cpp

void TTftLcd::DrawChar(int16_t x, int16_t y, char ch)
{
	// write a character
	const unsigned char * cp = &font_bitmap_data_5x8[5 * uint8_t(ch)];

	SetAddrWindow(x, y, 5, 8);

	for (int y = 0; y < 8; ++y)
	{
		unsigned char bm = (1 << y);
		for (int x = 0; x < 5; ++x)
		{
			if (cp[x] & bm)
			{
				FillColor(color, 1);
			}
			else
			{
				FillColor(bgcolor, 1);
			}
		}
	}
}
