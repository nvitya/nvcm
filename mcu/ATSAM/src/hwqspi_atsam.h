// hwqspi_atsam.h

#ifndef HWQSPI_ATSAM_H_
#define HWQSPI_ATSAM_H_

#define HWQSPI_PRE_ONLY
#include "hwqspi.h"

class THwQspi_atsam : public THwQspi_pre
{
public:
	unsigned char  txdmachannel = 5;
	unsigned char  rxdmachannel = 6;

	HW_QSPI_REGS * regs = nullptr;

	unsigned char *  qspidatamem;

	bool Init();

	virtual bool InitInterface(); // override

	int  StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len);
	int  StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len);
	void Run();

	unsigned       runstate = 0;
};

#define HWQSPI_IMPL THwQspi_atsam

#endif // def HWQSPI_ATSAM_H_
