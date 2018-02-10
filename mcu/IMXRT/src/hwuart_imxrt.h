// hwuart_imxrt.h

#ifndef HWUART_IMXRT_H_
#define HWUART_IMXRT_H_

#define HWUART_PRE_ONLY
#include "hwuart.h"

class THwUart_imxrt : public THwUart_pre
{
public:
	bool Init(int adevnum);

	bool TrySendChar(char ach);
	bool TryRecvChar(char * ach);

	bool SendFinished();

	void DmaAssign(bool istx, THwDmaChannel * admach);

	bool DmaStartSend(THwDmaTransfer * axfer);
	bool DmaStartRecv(THwDmaTransfer * axfer);

public:
	HW_UART_REGS *      regs = nullptr;
};

#define HWUART_IMPL THwUart_imxrt

#endif // def HWUART_IMXRT_H_
