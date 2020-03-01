// monolcd.h

#ifndef SRC_MONOLCD_H_
#define SRC_MONOLCD_H_

#include "platform.h"
#include "gfxbase.h"

typedef enum
{
	MLCD_CTRL_UNKNOWN = 0,
	MLCD_CTRL_UC1701,
	MLCD_CTRL_PCD8544, // = Nokia 5110
	MLCD_CTRL_HX1230
//
} TMonoLcdCtrlType;

#define MLCD_CTRL_NOKIA5110   MLCD_CTRL_PCD8544

class TMonoLcd : public TGfxBase
{
public:
	bool              initialized = false;

	TMonoLcdCtrlType  ctrltype = MLCD_CTRL_UNKNOWN;
	bool              mirrorx = false;

	uint32_t          updatecnt = 0; // for screen update control
	uint32_t          lastupdate = 0x12345678;

	int               updatestate = 0;

	uint8_t    			  rotation = 0;
	uint8_t           contrast = 0x8F;
	bool              externalvcc = false;

	uint16_t          aw_x = 0;
	uint16_t          aw_y = 0;

	uint8_t *         pdispbuf;

	virtual ~TMonoLcd() {} // to avoid warning

public:
	// interface dependent

	virtual bool InitInterface();

	//virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);

public:
	// interface independent functions

	bool Init(TMonoLcdCtrlType atype, uint16_t awidth, uint16_t aheight, uint8_t * adispbuf);

	void InitPanel();
	void SetDisplayOn(bool aon);

	void SetRotation(uint8_t m);

	virtual void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	virtual void DrawPixel(int16_t x, int16_t y, uint16_t color);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void SetAddrWindowStart(uint16_t x0, uint16_t y0);
	virtual void FillColor(uint16_t acolor, unsigned acount);

	bool UpdateFinished();
	virtual void Run();
};

#endif /* SRC_MONOLCD_H_ */
