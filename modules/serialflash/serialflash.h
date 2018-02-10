// serialflash.h

#ifndef SERIALFLASH_H_
#define SERIALFLASH_H_

#include "platform.h"
#include "hwpins.h"
#include "hwspi.h"
#include "hwdma.h"
#include "errors.h"

#define SERIALFLASH_STATE_READMEM   1
#define SERIALFLASH_STATE_WRITEMEM  2
#define SERIALFLASH_STATE_ERASE     3
#define SERIALFLASH_STATE_ERASEALL  4

class TSerialFlash
{
public:

public: // settings
	unsigned       has4kerase = false;

public:
	unsigned       idcode = 0;
	unsigned       bytesize = 0; // auto-detected from JEDEC ID

	bool 					 initialized = false;
	bool           completed = true;
	int            errorcode = 0;

	virtual        ~TSerialFlash() {} // to avoid warnings (never destructed)

	bool   				 Init();
	virtual bool   InitInherited();
	virtual bool   InitInterface();

	bool 					 StartReadMem(unsigned aaddr, void * adstptr, unsigned alen);
	bool 					 StartEraseMem(unsigned aaddr, unsigned alen);
	bool 					 StartEraseAll();
	bool 					 StartWriteMem(unsigned aaddr, void * asrcptr, unsigned alen); // must be erased before

	virtual bool   ReadIdCode();

	virtual void   Run(); // must be overridden !
	void 					 WaitForComplete();

protected:
	// state machine
	int            state = 0;
	int            phase = 0;

	unsigned       chunksize = 0;
	unsigned       maxchunksize = HW_DMA_MAX_COUNT;

	uint8_t *      dataptr = nullptr;
	unsigned       datalen = 0;
	unsigned       address = 0;
	unsigned       remaining = 0;
	unsigned       erasemask = 0;

};


#endif /* SERIALFLASH_H_ */
