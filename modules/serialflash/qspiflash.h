// qspiflash.h

#ifndef QSPIFLASH_H_
#define QSPIFLASH_H_

#include "platform.h"
#include "hwpins.h"
#include "hwqspi.h"
#include "hwdma.h"
#include "serialflash.h"
#include "errors.h"

class TQspiFlash : public TSerialFlash
{
public:
	typedef TSerialFlash super;

	// Required HW resources
	THwQspi        qspi;

	// overrides
	virtual bool   InitInherited();
	virtual bool   ReadIdCode();
	virtual void   Run();

protected:
	// smaller buffers for simple things
	unsigned char  txbuf[8];
	unsigned char  rxbuf[8];

	unsigned       curcmdlen = 0;

	unsigned       statusreg;

	void StartReadStatus();
	void StartWriteEnable();
};

#endif /* QSPIFLASH_H_ */
