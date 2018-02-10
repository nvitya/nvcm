// hwuart_atsam.h

#ifndef HWUART_ATSAM_H_
#define HWUART_ATSAM_H_

#define HWUART_PRE_ONLY
#include "hwuart.h"

class THwUart_atsam : public THwUart_pre
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
#ifdef HW_UART_ALT_REGS
	HW_UART_ALT_REGS * 	usartregs = nullptr;  // on Atmel the USART units can be used too
#endif
};

#define HWUART_IMPL THwUart_atsam

#endif // def HWUART_ATSAM_H_
