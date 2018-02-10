// hwspi_imxrt.h

#ifndef HWSPI_IMXRT_H_
#define HWSPI_IMXRT_H_

#define HWSPI_PRE_ONLY
#include "hwspi.h"

class THwSpi_imxrt : public THwSpi_pre
{
public:
	bool Init(int adevnum);

	bool TrySendData(unsigned short adata);
	bool TryRecvData(unsigned short * dstptr);
	bool SendFinished();

	void DmaAssign(bool istx, THwDmaChannel * admach);

	bool DmaStartSend(THwDmaTransfer * axfer);
	bool DmaStartRecv(THwDmaTransfer * axfer);
	bool DmaSendCompleted();
	bool DmaRecvCompleted();

public:
	HW_SPI_REGS * 			regs = nullptr;
	unsigned            tcrbase = 0;
};


#define HWSPI_IMPL THwSpi_imxrt

#endif // def HWSPI_IMXRT_H_
