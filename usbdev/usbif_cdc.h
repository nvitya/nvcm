/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
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
 *  file:     usbif_cdc.h
 *  brief:    USB CDC Class generic definitions
 *  version:  1.00
 *  date:     2020-08-02
 *  authors:  nvitya
*/

#ifndef USBIF_CDC_H_
#define USBIF_CDC_H_

struct TCdcLineCoding
{
	uint32_t   baudrate;
	uint8_t    charformat;  // 0 = 1 stop bits, 1 = 1.5 stop bits, 2 = 2 stop bits
	uint8_t    paritytype;  // 0 = off, 1 = odd parity, 2 = even parity
	uint8_t    databits;
};

extern const uint8_t cdc_desc_header_func[5];
extern const uint8_t cdc_desc_call_management[5];
extern const uint8_t cdc_desc_call_acm_func[4];
extern const uint8_t cdc_desc_call_union_func[5];

#endif /* USBIF_CDC_H_ */
