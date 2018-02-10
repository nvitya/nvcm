// spiflash.h

#ifndef SPIFLASH_H_
#define SPIFLASH_H_

#include "platform.h"
#include "hwpins.h"
#include "hwspi.h"
#include "hwdma.h"
#include "serialflash.h"
#include "errors.h"

class TSpiFlash : public TSerialFlash
{
public:
	typedef TSerialFlash super;

	// Required HW resources
	TGpioPin       pin_cs;
	THwSpi         spi;
	THwDmaChannel  txdma;
	THwDmaChannel  rxdma;

	// overrides
	virtual bool   InitInherited();
	virtual bool   ReadIdCode();
	virtual void   Run();

public:
	THwDmaTransfer txfer;
	THwDmaTransfer rxfer;

	// smaller buffers for simple things
	unsigned char  txbuf[16];
	unsigned char  rxbuf[16];

	unsigned       curcmdlen = 0;

	void StartCmd(unsigned acmdlen);
	void ExecCmd(unsigned acmdlen);

	void ResetChip();

	bool StartReadStatus();

};

#endif /* SPIFLASH_H_ */
