/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     hwuart.cpp
 *  brief:    Internal UART/USART vendor-independent implementations
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>
#include "mp_printf.h"

#include "hwuart.h"

void THwUart::printf_va(const char * fmt, va_list arglist)
{
  // allocate format buffer on the stack:
  char fmtbuf[FMT_BUFFER_SIZE];

  char * pch = &fmtbuf[0];
  *pch = 0;

  mp_vsnprintf(pch, FMT_BUFFER_SIZE, fmt, arglist);

  while (*pch != 0)
  {
  	SendChar(*pch);
    ++pch;
  }
}

void THwUart::printf(const char* fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);

  printf_va(fmt, arglist);

  va_end(arglist);
}

bool THwUart::DmaSendCompleted()
{
	if (txdma && txdma->Enabled())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwUart::DmaRecvCompleted()
{
	if (rxdma && rxdma->Enabled())
	{
		// Send DMA is still active
		return false;
	}

	return true;
}
