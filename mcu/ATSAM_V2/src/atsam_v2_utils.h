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
 *  file:     atsam_v2_utils.h
 *  brief:    ATSAM V2 Utilities
 *  version:  1.00
 *  date:     2019-01-18
 *  authors:  nvitya
*/

#ifndef ATSAM_V2_UTILS_H_
#define ATSAM_V2_UTILS_H_

#include "platform.h"

void atsam2_gclk_setup(uint8_t genid, uint8_t reference, uint32_t division);
void atsam2_enable_mclk(bool isahb, uint8_t regid, uint8_t bitid);
void atsam2_set_periph_gclk(uint32_t perid, uint8_t gclk);
bool atsam2_sercom_enable(int devnum, uint8_t clksrc);

void atsam2_extint_init();
void atsam2_extint_setup(uint8_t extintnum, uint8_t aconfig);
bool atsam2_extint_enable(bool aenable);

extern const Sercom *  sercom_inst_list[];

#endif /* ATSAM_V2_UTILS_H_ */
