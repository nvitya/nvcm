// hwqspi_lpc.h

#ifndef HWQSPI_LPC_H_
#define HWQSPI_LPC_H_

#define HWQSPI_PRE_ONLY
#include "hwqspi.h"

class THwQspi_lpc : public THwQspi_pre
{
public:
	unsigned char  dmachannel = 7;
	HW_QSPI_REGS * regs = nullptr;

	bool Init();

	virtual bool InitInterface(); // override

	int  StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len);
	int  StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len);
	void Run();

	unsigned       ctrlbase = 0;
};

#define HWQSPI_IMPL THwQspi_lpc

#endif // def HWQSPI_LPC_H_
