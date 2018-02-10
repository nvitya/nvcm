// hwspi_atsam.h

#ifndef HWSPI_ATSAM_H_
#define HWSPI_ATSAM_H_

#define HWSPI_PRE_ONLY
#include "hwspi.h"

class THwSpi_atsam : public THwSpi_pre
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
	unsigned  					basespeed;
	HW_SPI_REGS * 			regs;
#ifdef HW_UART_ALT_REGS
	HW_UART_ALT_REGS *	usartregs;  // on Atmel the USART units can be used too
#endif

};


#define HWSPI_IMPL THwSpi_atsam

#endif // def HWSPI_ATSAM_H_
