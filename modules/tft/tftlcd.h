// tftlcd.h

#ifndef TFTLCD_H_
#define TFTLCD_H_

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

class TTftLcd
{
public:
	uint32_t 				devid;

	char       			rotation = 0;

	bool            mirrorx = false;
	TLcdCtrlType    ctrltype = LCD_CTRL_UNKNOWN;

	uint16_t			 	hwwidth =  0;
	uint16_t				hwheight = 0;

	uint16_t				width =  0;
	uint16_t				height = 0;

	uint16_t  			bgcolor = 0x0000;
	uint16_t  			fgcolor = 0xFFFF;;

	virtual ~TTftLcd() {} // to avoid warning

public:
	// interface dependent

	virtual bool InitInterface();

	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);

public:
	// interface independent functions

	bool Init(TLcdCtrlType atype, uint16_t awidth, uint16_t aheight);
	void InitLcdPanel();
	void SetRotation(uint8_t m);
	void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void FillScreen(uint16_t color);
	void DrawPixel(int16_t x, int16_t y, uint16_t color);

	void DrawChar(int16_t x, int16_t y, char ch);

	// Panel dependent
	void RunCommandList(const uint8_t * addr);
};
#endif /* TFTLCD_H_ */
