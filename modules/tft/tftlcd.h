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
 *  file:     tftlcd.h
 *  brief:    TFT LCD Display driver support base class + definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef TFTLCD_H_
#define TFTLCD_H_

#include "gfxbase.h"
#include "platform.h"

typedef enum
{
	LCD_CTRL_UNKNOWN = 0,
	LCD_CTRL_ILI9341,
	LCD_CTRL_ILI9486,
	LCD_CTRL_ST7735,
	LCD_CTRL_HX8357B
//
} TLcdCtrlType;

class TTftLcd : public TGfxBase
{
public:
	uint32_t 				devid;

	bool            mirrorx = false;
	TLcdCtrlType    ctrltype = LCD_CTRL_UNKNOWN;

	virtual ~TTftLcd() {} // to avoid warning

public:
	// interface dependent

	virtual bool InitInterface();

	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

public:
	// interface independent functions

	bool Init(TLcdCtrlType atype, uint16_t awidth, uint16_t aheight);
	void InitLcdPanel();
	void SetRotation(uint8_t m);
	void DrawPixel(int16_t x, int16_t y, uint16_t color);

	void DrawChar(int16_t x, int16_t y, char ch);

	// Panel dependent
	void RunCommandList(const uint8_t * addr);
};
#endif /* TFTLCD_H_ */
