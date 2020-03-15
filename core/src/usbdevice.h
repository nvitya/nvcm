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
 *  file:     usbdevice.h
 *  brief:    USB Device Generics
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
 *  notes:
 *    Speical thanks to Niklas Gürtler for this tutorial: https://www.mikrocontroller.net/articles/USB-Tutorial_mit_STM32
 *    Special thanks to Craig Peacock for this tutorial: https://www.beyondlogic.org/usbnutshell/usb1.shtml
**/

#ifndef USBDEVICE_H_
#define USBDEVICE_H_

#include "hwusbctrl.h"

#define USBDEV_CTRL_BUF_SIZE   128  // every decriptor must fit into this, some are bigger than 64 byte !
#define USBDEV_MAX_STRINGS      16
#define USBDEV_MAX_INTERFACES    6
#define USBDEV_MAX_DESCREC       8
#define USBDEV_MAX_ENDPOINTS     8
#define USBINTF_MAX_DESCREC     16
#define USBINTF_MAX_ENDPOINTS    4

#define USBDESCF_CONFIG    1
#define USBDESCF_SUBTYPE   2

#define USB_DESC_TYPE_DEVICE                           1
#define USB_DESC_TYPE_CONFIGURATION                    2
#define USB_DESC_TYPE_STRING                           3
#define USB_DESC_TYPE_INTERFACE                        4
#define USB_DESC_TYPE_ENDPOINT                         5
#define USB_DESC_TYPE_DEVICE_QUALIFIER                 6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        7
#define USB_DESC_TYPE_BOS                              0x0F

#define USB_LEN_DEV_QUALIFIER_DESC                     0x0A
#define USB_LEN_DEV_DESC                               0x12
#define USB_LEN_CFG_DESC                               0x09
#define USB_LEN_IF_DESC                                0x09
#define USB_LEN_EP_DESC                                0x07
#define USB_LEN_OTG_DESC                               0x03
#define USB_LEN_LANGID_STR_DESC                        0x04
#define USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09

#define USBD_STRIDX_LANGID        0x00
#define USBD_STRIDX_MANUFACTURER  0x01
#define USBD_STRIDX_PRODUCT       0x02
#define USBD_STRIDX_SERIAL        0x03
#define USBD_STRIDX_CONFIG        0x04
#define USBD_STRIDX_INTERFACE     0x05

#define USBCTRL_STAGE_SETUP    0
#define USBCTRL_STAGE_DATAOUT  1
#define USBCTRL_STAGE_DATAIN   2
#define USBCTRL_STAGE_STATUS   3

typedef struct TUsbSetupRequest
{
	uint8_t   rqtype;   // bit0..4: recipient, 0 = device, 1 = interface, 2 = endpoint, 3 = other
	                    // bit5..6: type, 0 = standard, 1 = class, 2 = vendor, 3 = reserved
	                    // bit7: data direction, 0 = host to device (htod), 1 = device to host (dtoh)
	uint8_t   request;
	uint16_t  value;
	uint16_t  index;
	uint16_t  length;
//
} __attribute__((__packed__)) TUsbSetupRequest;

typedef struct TUsbDeviceDesc
{
	uint8_t		length;
	uint8_t 	descriptor_type;
	uint16_t 	usb_version;
	uint8_t  	device_class;
	uint8_t  	device_sub_class;
	uint8_t  	device_protocol;
	uint8_t  	max_packet_size;
	uint16_t 	vendor_id;
	uint16_t 	product_id;
	uint16_t 	device_version;
	uint8_t  	stri_manufacturer;
	uint8_t  	stri_product;
	uint8_t  	stri_serial_number;
	uint8_t  	num_configurations;
//
} __attribute__((__packed__)) TUsbDeviceDesc;

typedef struct TUsbDeviceQualifierDesc
{
	uint8_t		length;
	uint8_t 	descriptor_type;
	uint16_t 	usb_version;
	uint8_t  	device_class;
	uint8_t  	device_sub_class;
	uint8_t  	device_protocol;
	uint8_t  	max_packet_size_ep0;
	uint8_t  	other_speed_configs;
	uint8_t  	reserved;
//
} __attribute__((__packed__)) TUsbDeviceQualifierDesc;

typedef struct TUsbConfigDesc
{
	uint8_t		length; // = 9
	uint8_t 	descriptor_type; // = 1
	uint16_t 	total_length;     // total length of all descriptors including interface descriptors
	uint8_t  	num_interfaces;
	uint8_t  	configuration_value;
	uint8_t  	stri_configuration;
	uint8_t  	attributes;
	uint8_t  	max_power;
//
} __attribute__((__packed__)) TUsbConfigDesc;

typedef struct TUsbInterfaceDesc
{
	uint8_t		length; // = 9
	uint8_t 	descriptor_type; // = 4
	uint8_t  	interface_number;
	uint8_t  	alternate_setting;
	uint8_t  	num_endpoints;
	uint8_t  	interface_class;
	uint8_t  	interface_subclass;
	uint8_t  	interface_protocol;
	uint8_t  	stri_interface;
//
} __attribute__((__packed__)) TUsbInterfaceDesc;

typedef struct TUsbEndpointDesc
{
	uint8_t		length; // = 7
	uint8_t 	descriptor_type; // = 5
	uint8_t  	endpoint_address;  // including direction
	uint8_t  	attributes;
	uint16_t 	max_packet_size;
	uint8_t  	interval;  // polling interval
//
} __attribute__((__packed__)) TUsbEndpointDesc;



typedef struct TUsbDevDescRec
{
	uint16_t				id;
	uint8_t         flags;
	uint8_t     		datalen;
	uint8_t  *      dataptr;
//
} TUsbDevDescRec;

class TUsbInterface;
class TUsbDevice;

class TUsbEndpoint : public THwUsbEndpoint
{
public:
	TUsbInterface *      interface = nullptr;  // will be set on interface add

	uint8_t              interval = 10; // polling interval

	bool Init(uint8_t aattr, bool adir_htod, uint16_t amaxlen);
	void SetIndex(uint8_t aindex);

	//virtual void OnSendFinished();
	//virtual void OnDataReceived(int adatalength);

	virtual bool         HandleTransferEvent(bool htod);
	virtual bool         HandleSetupRequest(TUsbSetupRequest * psrq);

};

class TUsbInterface
{
public:
	TUsbDevice *         device = nullptr;  // will be set on device add

	uint8_t              index = 0xFF;
	uint8_t              altsetting = 0;    // provided for setconfig / getconfig

	bool                 configured = false;

	TUsbInterfaceDesc    intfdesc =
	{
		.length = 9,
		.descriptor_type = USB_DESC_TYPE_INTERFACE,
		.interface_number = 0,  // will be set automatically
		.alternate_setting = 0,
		.num_endpoints = 0,  // will be set automatically
		.interface_class = 0xFF, // invalid
		.interface_subclass = 0xFF,
		.interface_protocol = 0xFF,
		.stri_interface = 0  // will be set automatically
	};

	TUsbDevDescRec       desclist[USBINTF_MAX_DESCREC];
	int                  desccount = 0;

	const char *         interface_name = nullptr;

	TUsbEndpoint *       eplist[USBINTF_MAX_ENDPOINTS];
	int                  epcount = 0;

public:
  virtual ~TUsbInterface() {} // to avoid warnings

	virtual bool         InitInterface(); // should be overridden

	bool                 AddDesc(uint16_t atype, void * adataptr, uint8_t alen, uint8_t aflags);
	bool                 AddConfigDesc(void * adataptr, bool asubtype);

	TUsbDevDescRec *     FindDesc(uint16_t aid);

	void                 AddEndpoint(TUsbEndpoint * aep);
	int                  AppendConfigDesc(uint8_t * dptr, uint16_t amaxlen);

	virtual bool         HandleTransferEvent(TUsbEndpoint * aep, bool htod);
	virtual bool         HandleSetupRequest(TUsbSetupRequest * psrq);

	virtual void         OnConfigured();

	void                 SetConfigured();
};

class TUsbDevice : public THwUsbCtrl
{
public:  // quick variables (the first 32 variables are accessed faster on ARM, because they can be addressed with 16 bit instructions)
	uint8_t               ctrlstage = 0;
	uint8_t               epcount = 0;
	uint8_t               interface_count = 0;
	uint8_t               string_count = 0;

	TUsbSetupRequest      setuprq;  // 8 bytes

	int                   cdlen = 0;
	int                   ctrl_datastage_remaining = 0;

protected:
	bool                  set_devaddr_on_ack = false;

public:
	uint8_t  				      devaddr = 0xFF;    // assigned address
	uint8_t               actualconfig = 0;  // actualy selected configuration, 0 = unconfigured

	TUsbEndpoint          ep_ctrl;  // The Endpoint 0 provided here

	TUsbEndpoint *        eplist[USBDEV_MAX_ENDPOINTS];
	TUsbInterface *       interfaces[USBDEV_MAX_INTERFACES];
	char *          			stringtable[USBDEV_MAX_STRINGS] = {0};

	uint8_t               ctrlbuf[USBDEV_CTRL_BUF_SIZE];

public: // Descriptors

	TUsbDeviceDesc        devdesc =
	{
		.length = 18,
		.descriptor_type = USB_DESC_TYPE_DEVICE,
		.usb_version = 0x0200,
		.device_class = 0,
		.device_sub_class = 0,
		.device_protocol = 0,
		.max_packet_size = 64,
		.vendor_id = 0, // must be overridden !
		.product_id = 0,
		.device_version = 0x0100,  // 0x0100 = full speed
		.stri_manufacturer = 0,
		.stri_product = 0,
		.stri_serial_number = 0,
		.num_configurations = 1
	};

	// only one configuration is supported

	TUsbConfigDesc        confdesc =
	{
		.length = 9,
		.descriptor_type = USB_DESC_TYPE_CONFIGURATION,
		.total_length = 0,     // calculated automatically (total length of all descriptors)
		.num_interfaces = 0,   // calculated automatically
		.configuration_value = 1,
		.stri_configuration = 0,
		.attributes = 0xE0, // bus powered, supports wakeup
		.max_power = 0x32 // 100 mA
	};

	TUsbDeviceQualifierDesc  qualifierdesc =
	{
		.length = 10,
		.descriptor_type = USB_DESC_TYPE_DEVICE_QUALIFIER,
		.usb_version = 0x0200,
		.device_class = 0,
		.device_sub_class = 0,
		.device_protocol = 0,
		.max_packet_size_ep0 = 64,
		.other_speed_configs = 0,  // but will be sent the same for all
		.reserved = 0
	};

	const char *          manufacturer_name = "Unknown Manufacturer";
	const char *          device_name = "Unknown Device";
	const char *          device_serial_number = "12345678";

public:
	bool                  Init();

	virtual bool          HandleEpTransferEvent(uint8_t epid, bool htod); // called from the vendor specific code

	void                  HandleControlEndpoint(bool htod);

	void                  ProcessSetupRequest();
	void                  SendControlAck();

	void                  SendControlStatus(bool asuccess);

	void                  StartSendControlData(); // starts the dtoh data stage using ctrlbuf[cdlen]
	void                  StartSendControlData(void * asrc, unsigned alen); // starts the dtoh data stage

	void                  StartReceiveControlData(unsigned alen); // starts the htod data stage
	void                  HandleControlData();

  void                  AddInterface(TUsbInterface * aintf);
  uint8_t               AddString(const char * astr); // returned string index + 1 as id
  void                  AddEndpoint(TUsbEndpoint * aep);
  bool                  PrepareInterface(uint8_t ifidx, TUsbInterface * pif);

	void                  SetConfiguration(uint8_t aconfig);

public: // virtual methods

  virtual bool          InitDevice();
	virtual void          HandleReset();

	virtual bool          HandleSpecialSetupRequest() { return false; }  // returns true when handled
	virtual bool          HandleSpecialControlData()  { return false; }  // returns true when handled

protected:

	void                  MakeDeviceConfig(); // prepares the device config into the ctrlbuf

};

#endif /* USBDEVICE_H_ */
