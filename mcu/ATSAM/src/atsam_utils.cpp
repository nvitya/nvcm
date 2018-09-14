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
 *  file:     atsam_utils.cpp
 *  brief:    ATSAM Utilities
 *  version:  1.00
 *  date:     2018-06-07
 *  authors:  nvitya
*/

#include "platform.h"

uint32_t atsam_peripheral_clock()
{
#if defined(PMC_MCKR_MDIV_Pos)
	uint32_t result = SystemCoreClock;
	uint32_t div = (PMC->PMC_MCKR >> PMC_MCKR_MDIV_Pos) & 3;
	if (div == 1)  return (result >> 1);
	if (div == 2)  return (result / 3);
	if (div == 3)  return (result >> 2);
	return result;
#else
	return SystemCoreClock;
#endif
}

void atsam_enable_peripheral(uint32_t perid)
{
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}
}
