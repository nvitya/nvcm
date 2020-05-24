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
 *  file:     hwusbctrl_atsam_hs.h
 *  brief:    ATSAM USB HS Controller
 *  version:  1.00
 *  date:     2020-04-13
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_ATSAM_HS_H_
#define HWUSBCTRL_ATSAM_HS_H_

#include "platform.h"

#if defined(USBHS) || defined(UOTGHS)

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define HWUSB_MAX_ENDPOINTS      10

// However the ATSAME70 and ATSAM3X have the same USB HS unit they have different names (USBHS and UOTGHS)
// as by atmel all the register names are prefixed with the unit name the code would not be re-useable
// So own definition of the USB registers was made here, basically just the USBHS_ or UOTGHS_ prefices were removed

typedef struct
{
  __IO uint32_t NXTDSC;  /**< \brief (UsbhsDevdma Offset: 0x0) Device DMA Channel Next Descriptor Address Register */
  __IO uint32_t ADDRESS; /**< \brief (UsbhsDevdma Offset: 0x4) Device DMA Channel Address Register */
  __IO uint32_t CONTROL; /**< \brief (UsbhsDevdma Offset: 0x8) Device DMA Channel Control Register */
  __IO uint32_t STATUS;  /**< \brief (UsbhsDevdma Offset: 0xC) Device DMA Channel Status Register */
} HwUsbDevDma;

/** \brief UsbhsHstdma hardware registers */
typedef struct
{
  __IO uint32_t NXTDSC;  /**< \brief (UsbhsHstdma Offset: 0x0) Host DMA Channel Next Descriptor Address Register */
  __IO uint32_t ADDRESS; /**< \brief (UsbhsHstdma Offset: 0x4) Host DMA Channel Address Register */
  __IO uint32_t CONTROL; /**< \brief (UsbhsHstdma Offset: 0x8) Host DMA Channel Control Register */
  __IO uint32_t STATUS;  /**< \brief (UsbhsHstdma Offset: 0xC) Host DMA Channel Status Register */
} HwUsbHstDma;

/** \brief Usbhs hardware registers */
#define HWUSB_DEVDMA_NUMBER 7
#define HWUSB_HSTDMA_NUMBER 7
typedef struct {
  __IO uint32_t    DEVCTRL;                    /**< \brief (Usbhs Offset: 0x0000) Device General Control Register */
  __I  uint32_t    DEVISR;                     /**< \brief (Usbhs Offset: 0x0004) Device Global Interrupt Status Register */
  __O  uint32_t    DEVICR;                     /**< \brief (Usbhs Offset: 0x0008) Device Global Interrupt Clear Register */
  __O  uint32_t    DEVIFR;                     /**< \brief (Usbhs Offset: 0x000C) Device Global Interrupt Set Register */
  __I  uint32_t    DEVIMR;                     /**< \brief (Usbhs Offset: 0x0010) Device Global Interrupt Mask Register */
  __O  uint32_t    DEVIDR;                     /**< \brief (Usbhs Offset: 0x0014) Device Global Interrupt Disable Register */
  __O  uint32_t    DEVIER;                     /**< \brief (Usbhs Offset: 0x0018) Device Global Interrupt Enable Register */
  __IO uint32_t    DEVEPT;                     /**< \brief (Usbhs Offset: 0x001C) Device Endpoint Register */
  __I  uint32_t    DEVFNUM;                    /**< \brief (Usbhs Offset: 0x0020) Device Frame Number Register */
  __I  uint32_t    Reserved1[55];
  __IO uint32_t    DEVEPTCFG[10];              /**< \brief (Usbhs Offset: 0x100) Device Endpoint Configuration Register (n = 0) */
  __I  uint32_t    Reserved2[2];
  __I  uint32_t    DEVEPTISR[10];              /**< \brief (Usbhs Offset: 0x130) Device Endpoint Status Register (n = 0) */
  __I  uint32_t    Reserved3[2];
  __O  uint32_t    DEVEPTICR[10];              /**< \brief (Usbhs Offset: 0x160) Device Endpoint Clear Register (n = 0) */
  __I  uint32_t    Reserved4[2];
  __O  uint32_t    DEVEPTIFR[10];              /**< \brief (Usbhs Offset: 0x190) Device Endpoint Set Register (n = 0) */
  __I  uint32_t    Reserved5[2];
  __I  uint32_t    DEVEPTIMR[10];              /**< \brief (Usbhs Offset: 0x1C0) Device Endpoint Mask Register (n = 0) */
  __I  uint32_t    Reserved6[2];
  __O  uint32_t    DEVEPTIER[10];              /**< \brief (Usbhs Offset: 0x1F0) Device Endpoint Enable Register (n = 0) */
  __I  uint32_t    Reserved7[2];
  __O  uint32_t    DEVEPTIDR[10];              /**< \brief (Usbhs Offset: 0x220) Device Endpoint Disable Register (n = 0) */
  __I  uint32_t    Reserved8[50];
       HwUsbDevDma DEVDMA[HWUSB_DEVDMA_NUMBER]; /**< \brief (Usbhs Offset: 0x310) n = 1 .. 7 */
  __I  uint32_t    Reserved9[32];
  __IO uint32_t    HSTCTRL;                    /**< \brief (Usbhs Offset: 0x0400) Host General Control Register */
  __I  uint32_t    HSTISR;                     /**< \brief (Usbhs Offset: 0x0404) Host Global Interrupt Status Register */
  __O  uint32_t    HSTICR;                     /**< \brief (Usbhs Offset: 0x0408) Host Global Interrupt Clear Register */
  __O  uint32_t    HSTIFR;                     /**< \brief (Usbhs Offset: 0x040C) Host Global Interrupt Set Register */
  __I  uint32_t    HSTIMR;                     /**< \brief (Usbhs Offset: 0x0410) Host Global Interrupt Mask Register */
  __O  uint32_t    HSTIDR;                     /**< \brief (Usbhs Offset: 0x0414) Host Global Interrupt Disable Register */
  __O  uint32_t    HSTIER;                     /**< \brief (Usbhs Offset: 0x0418) Host Global Interrupt Enable Register */
  __IO uint32_t    HSTPIP;                     /**< \brief (Usbhs Offset: 0x0041C) Host Pipe Register */
  __IO uint32_t    HSTFNUM;                    /**< \brief (Usbhs Offset: 0x0420) Host Frame Number Register */
  __IO uint32_t    HSTADDR1;                   /**< \brief (Usbhs Offset: 0x0424) Host Address 1 Register */
  __IO uint32_t    HSTADDR2;                   /**< \brief (Usbhs Offset: 0x0428) Host Address 2 Register */
  __IO uint32_t    HSTADDR3;                   /**< \brief (Usbhs Offset: 0x042C) Host Address 3 Register */
  __I  uint32_t    Reserved10[52];
  __IO uint32_t    HSTPIPCFG[10];              /**< \brief (Usbhs Offset: 0x500) Host Pipe Configuration Register (n = 0) */
  __I  uint32_t    Reserved11[2];
  __I  uint32_t    HSTPIPISR[10];              /**< \brief (Usbhs Offset: 0x530) Host Pipe Status Register (n = 0) */
  __I  uint32_t    Reserved12[2];
  __O  uint32_t    HSTPIPICR[10];              /**< \brief (Usbhs Offset: 0x560) Host Pipe Clear Register (n = 0) */
  __I  uint32_t    Reserved13[2];
  __O  uint32_t    HSTPIPIFR[10];              /**< \brief (Usbhs Offset: 0x590) Host Pipe Set Register (n = 0) */
  __I  uint32_t    Reserved14[2];
  __I  uint32_t    HSTPIPIMR[10];              /**< \brief (Usbhs Offset: 0x5C0) Host Pipe Mask Register (n = 0) */
  __I  uint32_t    Reserved15[2];
  __O  uint32_t    HSTPIPIER[10];              /**< \brief (Usbhs Offset: 0x5F0) Host Pipe Enable Register (n = 0) */
  __I  uint32_t    Reserved16[2];
  __O  uint32_t    HSTPIPIDR[10];              /**< \brief (Usbhs Offset: 0x620) Host Pipe Disable Register (n = 0) */
  __I  uint32_t    Reserved17[2];
  __IO uint32_t    HSTPIPINRQ[10];             /**< \brief (Usbhs Offset: 0x650) Host Pipe IN Request Register (n = 0) */
  __I  uint32_t    Reserved18[2];
  __IO uint32_t    HSTPIPERR[10];              /**< \brief (Usbhs Offset: 0x680) Host Pipe Error Register (n = 0) */
  __I  uint32_t    Reserved19[26];
       HwUsbHstDma HSTDMA[HWUSB_HSTDMA_NUMBER]; /**< \brief (Usbhs Offset: 0x710) n = 1 .. 7 */
  __I  uint32_t    Reserved20[32];
  __IO uint32_t    CTRL;                       /**< \brief (Usbhs Offset: 0x0800) General Control Register */
  __I  uint32_t    SR;                         /**< \brief (Usbhs Offset: 0x0804) General Status Register */
  __O  uint32_t    SCR;                        /**< \brief (Usbhs Offset: 0x0808) General Status Clear Register */
  __O  uint32_t    SFR;                        /**< \brief (Usbhs Offset: 0x080C) General Status Set Register */
  __IO uint32_t    TSTA1;                      /**< \brief (Usbhs Offset: 0x0810) General Test A1 Register */
  __IO uint32_t    TSTA2;                      /**< \brief (Usbhs Offset: 0x0814) General Test A2 Register */
  __I  uint32_t    VERSION;                    /**< \brief (Usbhs Offset: 0x0818) General Version Register */
  __I  uint32_t    Reserved21[4];
  __I  uint32_t    FSM;                        /**< \brief (Usbhs Offset: 0x082C) General Finite State Machine Register */
} HwUsbRegs;

// some E70 / 3X unified definitions

#define HWUSB_DEVEPT_TXIN         (1 << 0) /**< \brief (HWUSB_DEVEPT[10]) Transmitted IN Data Interrupt */
#define HWUSB_DEVEPT_RXOUT        (1 << 1) /**< \brief (HWUSB_DEVEPT[10]) Received OUT Data Interrupt */
#define HWUSB_DEVEPT_RXSTP        (1 << 2) /**< \brief (HWUSB_DEVEPT[10]) Received SETUP Interrupt */
#define HWUSB_DEVEPT_NAKOUT       (1 << 3) /**< \brief (HWUSB_DEVEPT[10]) NAKed OUT Interrupt */
#define HWUSB_DEVEPT_NAKIN        (1 << 4) /**< \brief (HWUSB_DEVEPT[10]) NAKed IN Interrupt */
#define HWUSB_DEVEPT_OVERF        (1 << 5) /**< \brief (HWUSB_DEVEPT[10]) Overflow Interrupt */
#define HWUSB_DEVEPT_STALLED      (1 << 6) /**< \brief (HWUSB_DEVEPT[10]) STALLed Interrupt */
#define HWUSB_DEVEPT_SHORTPACKET  (1 << 7) /**< \brief (HWUSB_DEVEPT[10]) Short Packet Interrupt */

#define HWUSB_DEVEPT_UNDERF       (1 << 2) /**< \brief (HWUSB_DEVEPT[10]) Underflow Interrupt */
#define HWUSB_DEVEPT_HBISOINERRI  (1 << 3) /**< \brief (HWUSB_DEVEPT[10]) High Bandwidth Isochronous IN Underflow Error Interrupt */
#define HWUSB_DEVEPT_HBISOFLUSHI  (1 << 4) /**< \brief (HWUSB_DEVEPT[10]) High Bandwidth Isochronous IN Flush Interrupt */
#define HWUSB_DEVEPT_CRCERRI      (1 << 6) /**< \brief (HWUSB_DEVEPT[10]) CRC Error Interrupt */
#define HWUSB_DEVEPT_ERRORTRANS   (1 << 10) /**< \brief (HWUSB_DEVEPT[10]) High-bandwidth Isochronous OUT Endpoint Transaction Error Interrupt */

#define HWUSB_DEVEPT_DTSEQ_Pos    8
#define HWUSB_DEVEPT_DTSEQ_Msk    (0x3u << HWUSB_DEVEPT_DTSEQ_Pos) /**< \brief (HWUSB_DEVEPT[10]) Data Toggle Sequence */
#define HWUSB_DEVEPT_NBUSYBK_Pos  12
#define HWUSB_DEVEPT_NBUSYBK_Msk  (0x3u << HWUSB_DEVEPT_NBUSYBK_Pos) /**< \brief (HWUSB_DEVEPT[10]) Number of Busy Banks */
#define HWUSB_DEVEPT_FIFOCON      (1 << 14) /**< \brief (UOTGHS_DEVEPTIDR[10]) FIFO Control Clear */
#define HWUSB_DEVEPT_CURRBK_Pos   14
#define HWUSB_DEVEPT_CURRBK_Msk   (0x3u << HWUSB_DEVEPT_CURRBK_Pos) /**< \brief (HWUSB_DEVEPT[10]) Current Bank */
#define HWUSB_DEVEPT_RWALL        (1 << 16) /**< \brief (HWUSB_DEVEPT[10]) Read/Write Allowed */
#define HWUSB_DEVEPT_CTRLDIR      (1 << 17) /**< \brief (HWUSB_DEVEPT[10]) Control Direction */
#define HWUSB_DEVEPT_CFGOK        (1 << 18) /**< \brief (HWUSB_DEVEPT[10]) Configuration OK Status */
#define HWUSB_DEVEPT_STALLRQ      (1 << 19) // stall request or stall request clear
#define HWUSB_DEVEPT_BYCT_Pos     20
#define HWUSB_DEVEPT_BYCT_Msk     (0x7ffu << HWUSB_DEVEPT_BYCT_Pos) /**< \brief (HWUSB_DEVEPT[10]) Byte Count */

#define HWUSB_DEVCTRL_ADDEN       (1 << 7) /**< \brief (USBHS_DEVCTRL) Address Enable */
#define HWUSB_DEVCTRL_DETACH      (1 << 8) /**< \brief (USBHS_DEVCTRL) Detach */
#define HWUSB_DEVCTRL_SPDCONF_Pos 10
#define HWUSB_DEVCTRL_SPDCONF_Msk (3 << HWUSB_DEVCTRL_SPDCONF_Pos) /**< \brief (HWUSB_DEVCTRL) Mode Configuration */
#define HWUSB_DEVCTRL_SPDCONF(value) ((HWUSB_DEVCTRL_SPDCONF_Msk & ((value) << HWUSB_DEVCTRL_SPDCONF_Pos)))
#define   HWUSB_DEVCTRL_SPDCONF_NORMAL     (0 << 10) /**< \brief (HWUSB_DEVCTRL) The peripheral starts in Full-speed mode and performs a high-speed reset to switch to High-speed mode if the host is high-speed-capable. */
#define   HWUSB_DEVCTRL_SPDCONF_LOW_POWER  (1 << 10) /**< \brief (HWUSB_DEVCTRL) For a better consumption, if high speed is not needed. */
#define   HWUSB_DEVCTRL_SPDCONF_HIGH_SPEED (2 << 10) /**< \brief (HWUSB_DEVCTRL) Forced high speed. */
#define   HWUSB_DEVCTRL_SPDCONF_FORCED_FS  (3 << 10) /**< \brief (HWUSB_DEVCTRL) The peripheral remains in Full-speed mode whatever the host speed capability. */
#define HWUSB_DEVCTRL_LS          (1 << 12) /**< \brief (HWUSB_DEVCTRL) Low-Speed Mode Force */
#define HWUSB_DEVCTRL_TSTJ        (1 << 13) /**< \brief (HWUSB_DEVCTRL) Test mode J */
#define HWUSB_DEVCTRL_TSTK        (1 << 14) /**< \brief (HWUSB_DEVCTRL) Test mode K */
#define HWUSB_DEVCTRL_TSTPCKT     (1 << 15) /**< \brief (HWUSB_DEVCTRL) Test packet mode */
#define HWUSB_DEVCTRL_OPMODE2     (1 << 16) /**< \brief (HWUSB_DEVCTRL) Specific Operational mode */


#define HWUSB_DEVIRQ_SUSP         (1 << 0) /**< \brief (USBHS_DEVIER) Suspend Interrupt Enable */
#define HWUSB_DEVIRQ_MSOF         (1 << 1) /**< \brief (USBHS_DEVIER) Micro Start of Frame Interrupt Enable */
#define HWUSB_DEVIRQ_SOF          (1 << 2) /**< \brief (USBHS_DEVIER) Start of Frame Interrupt Enable */
#define HWUSB_DEVIRQ_EORST        (1 << 3) /**< \brief (USBHS_DEVIER) End of Reset Interrupt Enable */
#define HWUSB_DEVIRQ_WAKEUP       (1 << 4) /**< \brief (USBHS_DEVIER) Wake-Up Interrupt Enable */
#define HWUSB_DEVIRQ_EORSM        (1 << 5) /**< \brief (USBHS_DEVIER) End of Resume Interrupt Enable */
#define HWUSB_DEVIRQ_UPRSM        (1 << 6) /**< \brief (USBHS_DEVIER) Upstream Resume Interrupt Enable */

#define HWUSB_CTRL_VBUSHWC        (1 <<  8) /**< \brief (USBHS_CTRL) VBUS Hardware Control */
#define HWUSB_CTRL_FRZCLK         (1 << 14) /**< \brief (USBHS_CTRL) Freeze USB Clock */
#define HWUSB_CTRL_USBE           (1 << 15) /**< \brief (USBHS_CTRL) USBHS Enable */
#define HWUSB_CTRL_UIMOD          (1 << 25) /**< \brief (USBHS_CTRL) USBHS Mode */
#define HWUSB_CTRL_UIMOD_HOST     (0 << 25) /**< \brief (USBHS_CTRL) The module is in USB Host mode. */
#define HWUSB_CTRL_UIMOD_DEVICE   (1 << 25) /**< \brief (USBHS_CTRL) The module is in USB Device mode. */

#define HWUSB_SR_VBUSRQ             (1 << 9) /**< \brief (HWUSB_SR) VBus Request (Host mode only) */
#define HWUSB_SR_SPEED_Pos          12
#define HWUSB_SR_SPEED_Msk          (3 << HWUSB_SR_SPEED_Pos) /**< \brief (HWUSB_SR) Speed Status (Device mode only) */
#define   HWUSB_SR_SPEED_FULL_SPEED (0 << 12) /**< \brief (HWUSB_SR) Full-Speed mode */
#define   HWUSB_SR_SPEED_HIGH_SPEED (1 << 12) /**< \brief (HWUSB_SR) High-Speed mode */
#define   HWUSB_SR_SPEED_LOW_SPEED  (2 << 12) /**< \brief (HWUSB_SR) Low-Speed mode */
#define HWUSB_SR_CLKUSABLE          (1 << 14) /**< \brief (HWUSB_SR) UTMI Clock Usable */


class THwUsbEndpoint_atsam_hs : public THwUsbEndpoint_pre
{
public:

	__IO uint8_t *       fifo_reg;
	__IO uint32_t *      cfg_reg;
	__IO uint32_t *      isr_reg;
	__IO uint32_t *      icr_reg;
	__IO uint32_t *      imr_reg;
	__IO uint32_t *      ier_reg;
	__IO uint32_t *      idr_reg;

	virtual ~THwUsbEndpoint_atsam_hs() { }

	bool ConfigureHwEp();
  int  ReadRecvData(void * buf, uint32_t buflen);
	int  StartSendData(void * buf, unsigned len);

	void SendAck();
  void Stall();
  void Nak();

  inline bool IsSetupRequest()  { return (*isr_reg & HWUSB_DEVEPT_RXSTP); }

  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
};

class THwUsbCtrl_atsam_hs : public THwUsbCtrl_pre
{
public:
	HwUsbRegs *         regs = nullptr;
	uint32_t            irq_mask = 0;

	virtual ~THwUsbCtrl_atsam_hs() { }

	bool InitHw();

	void HandleIrq();

	void DisableIrq();
	void EnableIrq();
	void SetDeviceAddress(uint8_t aaddr);
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_atsam_hs
#define HWUSBCTRL_IMPL       THwUsbCtrl_atsam_hs

#endif // if defined(USBHS)

#endif // def HWUSBCTRL_ATSAM_HS_H_
