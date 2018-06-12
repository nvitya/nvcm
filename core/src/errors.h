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
 *  file:     errors.h
 *  brief:    NVCM error codes
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef ERRORS_H_
#define ERRORS_H_

#define ERROR_OK  0

#define ERROR_NOTINIT  -1   // the device is not initialized
#define ERROR_TIMEOUT  -2   // operation timed out
#define ERROR_NOTIMPL  -3   // not implemented
#define ERROR_BUSY     -4   // unit busy
#define ERROR_UNKNOWN  -5   // unknown hardware
#define ERROR_READ     -6   // read error
#define ERROR_WRITE    -7   // write error


#endif /* ERRORS_H_ */
