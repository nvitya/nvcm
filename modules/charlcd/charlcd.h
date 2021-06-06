// charlcd.h

#ifndef CHARLCD_H_
#define CHARLCD_H_

#include "platform.h"
#include "textscreen.h"

typedef enum
{
	CHLCD_CTRL_UNKNOWN = 0,
	CHLCD_CTRL_HD44780,
//
} TCharLcdCtrlType;

class TCharLcd : public TTextScreen
{
public:
	bool              initialized = false;

	TCharLcdCtrlType  ctrltype = CHLCD_CTRL_UNKNOWN;

	uint32_t          updatecnt = 0; // for screen update control
	uint32_t          lastupdate = 0x12345678;
	int               updatestate = 0;

	bool              interface_4bit = true;

	virtual ~TCharLcd() {} // to avoid warning

public:
	// interface dependent

	virtual bool InitInterface();

	//virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData(uint8_t adata);

public: // Textreen mandatory implementations

	virtual void    DrawChar(unsigned acol, unsigned arow, char ach);
	virtual void    SetCursor();

public:
	// interface independent functions

	bool Init(TCharLcdCtrlType atype, unsigned acols, unsigned arows, uint8_t * ascreenbuf, uint8_t * achangemap);

	void InitPanel();
};

#endif /* CHARLCD_H_ */
