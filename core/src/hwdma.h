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
 *  file:     hwdma.h
 *  brief:    DMA controller vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWDMA_H_PRE_
#define HWDMA_H_PRE_

#include "platform.h"

// DMA Transfer Flags

#define DMATR_NO_ADDR_INC   0x0003  // do not increment SRC and DST ADDRESS
#define DMATR_NO_SRC_INC    0x0001
#define DMATR_NO_DST_INC    0x0002
#define DMATR_MEM_TO_MEM    0x0008
#define DMATR_IRQ           0x0010
#define DMATR_CIRCULAR      0x0100  // not all HW supports it !

class THwDmaTransfer
{
public:
	void *      srcaddr = nullptr;
	void *      dstaddr = nullptr;
	uint8_t     bytewidth = 1;  // 1, 2 or 4
	uint32_t    count = 0;
	uint32_t    flags = 0;
};

class THwDmaChannel_pre
{
public:
	bool               istx = false;  // mem to periph ?
	void *             periphaddr = nullptr;
	int           		 chnum = -1;
	bool               initialized = false;
	int                priority = 0;
};

#endif // ndef HWDMA_H_PRE_

#ifndef HWDMA_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWDMA_H_
#define HWDMA_H_

#include "mcu_impl.h"

#ifndef HWDMACHANNEL_IMPL

#ifdef SKIP_UNIMPLEMENTED_WARNING
  #undef SKIP_UNIMPLEMENTED_WARNING
#else
  #warning "HWDMA is not implemented!"
#endif

class THwDmaChannel_noimpl : public THwDmaChannel_pre
{
public: // mandatory
	bool Init(int achnum)  { return false; }

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags)  { }
	void Disable() { }
	void Enable()  { }

	bool Enabled() { return false; }
	bool Active()  { return false; }

	void PrepareTransfer(THwDmaTransfer * axfer)  { }
	void StartPreparedTransfer()  { }
};

#define HWDMACHANNEL_IMPL THwDmaChannel_noimpl

  #define HWDMA_IMPLEMENTED 0
#else
  #define HWDMA_IMPLEMENTED 1
#endif // ndef HWDMA_IMPL

//-----------------------------------------------------------------------------

class THwDmaChannel : public HWDMACHANNEL_IMPL
{
public:
	inline void StartTransfer(THwDmaTransfer * axfer)
	{
		PrepareTransfer(axfer);
		StartPreparedTransfer();
	}
};

#endif /* HWDMA_H_ */

#else
  #undef HWDMA_PRE_ONLY
#endif
