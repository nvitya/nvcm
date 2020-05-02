/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya.
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
 *  file:     swo.cpp
 *  brief:    SWO (Serial Wire Output) trace implementation
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#if __CORTEX_M >= 3 // available only from Cortex-M3 and above

#include <stdio.h>
#include <stdarg.h>

#include "swo.h"

// some Atmel devices use this name
#ifdef PORT
  #undef PORT
#endif

bool swo_try_send_char(char ch)
{
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
      ((ITM->TER & 1UL               ) != 0UL)   )     /* ITM Port #0 enabled */
  {
    if (ITM->PORT[0U].u32)
    {
    	ITM->PORT[0U].u8 = ch;
    	return true;
    }

    return false;
  }

  return true;  // drop the characters when not enabled
}

void swo_putc(char ch)
{
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
      ((ITM->TER & 1UL               ) != 0UL)   )     /* ITM Port #0 enabled */
  {
    while (ITM->PORT[0U].u32 == 0UL)
    {
      __NOP();
    }

    ITM->PORT[0U].u8 = ch;
  }
}

void swo_printf(const char * fmt, ...)
{
	char tracefmtbuf[SWO_MAX_MESSAGE]; // allocated on the stack !

  va_list arglist;
  va_start(arglist, fmt);
  char * pch;

  pch = &tracefmtbuf[0];

  *pch = 0;

  vsnprintf(pch, SWO_MAX_MESSAGE, fmt, arglist);

  while (*pch != 0)
  {
  	swo_putc(*pch);
    ++pch;
  }

  va_end(arglist);
}

#endif

//--- End Of file --------------------------------------------------------------
