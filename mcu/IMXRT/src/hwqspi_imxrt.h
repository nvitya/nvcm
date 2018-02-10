// hwqspi_imxrt.h

#ifndef HWQSPI_IMXRT_H_
#define HWQSPI_IMXRT_H_

#define HWQSPI_PRE_ONLY
#include "hwqspi.h"

class THwQspi_imxrt : public THwQspi_pre
{
public:
	unsigned char  txdmachannel = 6;
	unsigned char  rxdmachannel = 7;

	HW_QSPI_REGS * regs = nullptr;

	bool Init();

	virtual bool InitInterface(); // override

	int  StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len);
	int  StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len);
	void Run();

	void WaitForIdle();
};

#define HWQSPI_IMPL THwQspi_imxrt

#endif // def HWQSPI_IMXRT_H_
