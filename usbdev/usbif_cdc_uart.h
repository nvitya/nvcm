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
 *  file:     usbif_cdc_uart.h
 *  brief:    USB to Serial USB Interface definition
 *  version:  1.00
 *  date:     2020-08-01
 *  authors:  nvitya
 *  note:
 *    it can be used multiple times
*/

#ifndef SRC_USBIF_CDC_UART_H_
#define SRC_USBIF_CDC_UART_H_

#include "usbdevice.h"
#include "usbif_cdc.h"
#include "hwuart.h"
#include "hwdma.h"


class TUifCdcUartData;

class TUifCdcUartControl : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:
	THwUart *         uart = nullptr;
	TUifCdcUartData *	dataif = nullptr;

	THwDmaChannel *   dma_rx = nullptr;
	THwDmaChannel *   dma_tx = nullptr;

	TCdcLineCoding    linecoding;

	TUsbEndpoint      ep_manage;
	uint8_t           databuf[64];

	// Serial (UART) --> USB
	uint8_t           serial_rxbuf[128];  // use as circular buffer
	uint8_t           serial_rxidx = 0;


	// USB --> Serial (UART)
	uint8_t           serial_txbuf[2][64];
	uint8_t           serial_txbufidx = 0;
	uint8_t           serial_txlen = 0;

public: // interface specific
	bool              InitCdcUart(TUifCdcUartData * adataif, THwUart * auart, THwDmaChannel * adma_tx, THwDmaChannel * adma_rx);
	void              Run(); // must be called periodically

	bool              SerialAddBytes(uint8_t * adata, unsigned adatalen);
	void              SerialSendBytes();

protected:
	bool              uart_running = false;
	THwDmaTransfer    dmaxfer_tx;
	THwDmaTransfer    dmaxfer_rx;

	void              StopUart();
	void              StartUart();

public: // mandatory virtual functions
	virtual bool      InitInterface();
	virtual void      OnConfigured();
	virtual bool      HandleTransferEvent(TUsbEndpoint * aep, bool htod);
	virtual bool      HandleSetupRequest(TUsbSetupRequest * psrq);
	virtual bool      HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen);

};

class TUifCdcUartData : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:
	TUifCdcUartControl *  control = nullptr;

	bool                  ready_to_send = true;

	TUsbEndpoint          ep_input;
	TUsbEndpoint          ep_output;

	uint8_t               usb_txbuf[2][128];
	uint8_t               usb_txbufidx = 0;
	uint8_t               usb_txlen = 0;  // non null value signalizes pending USB RX data to be sent to the serial

	uint8_t           		usb_rxbuf[64+4];  // the data from the usb (+4 for debugging)
	uint8_t               usb_rxlen = 0;

public:
	bool                  AddTxByte(uint8_t abyte);
	bool                  SendTxBytes();
	void                  Reset();
	void                  TrySendUsbDataToSerial();

public: // mandatory virtual functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
};



#endif /* SRC_USBIF_CDC_UART_H_ */
