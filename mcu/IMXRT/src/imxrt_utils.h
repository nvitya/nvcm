/*
 * imxrt_utils.h
 *
 *  Created on: Dec 13, 2017
 *      Author: nagy
 */

#ifndef IMXRT_UTILS_H_
#define IMXRT_UTILS_H_

#include "platform.h"

static inline void imxrt_set_clock_gate(unsigned index, unsigned shift, unsigned value)
{
	if (index <= 6)
	{
		volatile uint32_t *reg;
		reg = ((volatile uint32_t *)&CCM->CCGR0) + index;
		*reg = ((*reg) & ~(3U << shift)) | ((value & 3) << shift);
	}
}


#endif /* IMXRT_UTILS_H_ */
