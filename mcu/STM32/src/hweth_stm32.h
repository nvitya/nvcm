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
 *  file:     hweth_stm32.h
 *  brief:    STM32 Ethernet MAC
 *  version:  1.00
 *  date:     2018-05-30
 *  authors:  nvitya
*/

#ifndef HWETH_STM32_H_
#define HWETH_STM32_H_

#define HWETH_PRE_ONLY
#include "hweth.h"

typedef struct
{
	volatile uint32_t STATUS;		// RDES status word
	volatile uint32_t CTRL;			// Buffer 1/2 byte counts
	volatile uint32_t B1ADD;		// Buffer 1 address
	volatile uint32_t B2ADD;		// Buffer 2 or next descriptor address
	volatile uint32_t EXTSTAT;	// Extended Status
	volatile uint32_t _RES;	  	// Reserved
	volatile uint32_t TSL;			// Timestamp value low
	volatile uint32_t TSH;			// Timestamp value high
//
} HW_ETH_DMA_DESC;

typedef struct                      /*!< ETHERNET Structure */
{
	__IO uint32_t  MAC_CONFIG;				/*!< MAC configuration register */
	__IO uint32_t  MAC_FRAME_FILTER;		/*!< MAC frame filter */
	__IO uint32_t  MAC_HASHTABLE_HIGH;		/*!< Hash table high register */
	__IO uint32_t  MAC_HASHTABLE_LOW;		/*!< Hash table low register */
	__IO uint32_t  MAC_MII_ADDR;			/*!< MII address register */
	__IO uint32_t  MAC_MII_DATA;			/*!< MII data register */
	__IO uint32_t  MAC_FLOW_CTRL;			/*!< Flow control register */
	__IO uint32_t  MAC_VLAN_TAG;			/*!< VLAN tag register */
	__I  uint32_t  RESERVED0;
	__I  uint32_t  MAC_DEBUG;				/*!< Debug register */
	__IO uint32_t  MAC_RWAKE_FRFLT;			/*!< Remote wake-up frame filter */
	__IO uint32_t  MAC_PMT_CTRL_STAT;		/*!< PMT control and status */
	__I  uint32_t  RESERVED1[2];
	__I  uint32_t  MAC_INTR;				/*!< Interrupt status register */
	__IO uint32_t  MAC_INTR_MASK;			/*!< Interrupt mask register */
	__IO uint32_t  MAC_ADDR0_HIGH;			/*!< MAC address 0 high register */
	__IO uint32_t  MAC_ADDR0_LOW;			/*!< MAC address 0 low register */
	__I  uint32_t  RESERVED2[430];
	__IO uint32_t  MAC_TIMESTP_CTRL;		/*!< Time stamp control register */
	__IO uint32_t  SUBSECOND_INCR;			/*!< Sub-second increment register */
	__I  uint32_t  SECONDS;					/*!< System time seconds register */
	__I  uint32_t  NANOSECONDS;				/*!< System time nanoseconds register */
	__IO uint32_t  SECONDSUPDATE;			/*!< System time seconds update register */
	__IO uint32_t  NANOSECONDSUPDATE;		/*!< System time nanoseconds update register */
	__IO uint32_t  ADDEND;					/*!< Time stamp addend register */
	__IO uint32_t  TARGETSECONDS;			/*!< Target time seconds register */
	__IO uint32_t  TARGETNANOSECONDS;		/*!< Target time nanoseconds register */
	__IO uint32_t  HIGHWORD;				/*!< System time higher word seconds register */
	__I  uint32_t  TIMESTAMPSTAT;			/*!< Time stamp status register */
	__IO uint32_t  PPSCTRL;					/*!< PPS control register */
	__I  uint32_t  AUXNANOSECONDS;			/*!< Auxiliary time stamp nanoseconds register */
	__I  uint32_t  AUXSECONDS;				/*!< Auxiliary time stamp seconds register */
	__I  uint32_t  RESERVED3[562];
	__IO uint32_t  DMA_BUS_MODE;			/*!< Bus Mode Register      */
	__IO uint32_t  DMA_TRANS_POLL_DEMAND;	/*!< Transmit poll demand register */
	__IO uint32_t  DMA_REC_POLL_DEMAND;		/*!< Receive poll demand register */
	__IO uint32_t  DMA_REC_DES_ADDR;		/*!< Receive descriptor list address register */
	__IO uint32_t  DMA_TRANS_DES_ADDR;		/*!< Transmit descriptor list address register */
	__IO uint32_t  DMA_STAT;				/*!< Status register */
	__IO uint32_t  DMA_OP_MODE;				/*!< Operation mode register */
	__IO uint32_t  DMA_INT_EN;				/*!< Interrupt enable register */
	__I  uint32_t  DMA_MFRM_BUFOF;			/*!< Missed frame and buffer overflow register */
	__IO uint32_t  DMA_REC_INT_WDT;			/*!< Receive interrupt watchdog timer register */
	__I  uint32_t  RESERVED4[8];
	__I  uint32_t  DMA_CURHOST_TRANS_DES;	/*!< Current host transmit descriptor register */
	__I  uint32_t  DMA_CURHOST_REC_DES;		/*!< Current host receive descriptor register */
	__I  uint32_t  DMA_CURHOST_TRANS_BUF;	/*!< Current host transmit buffer address register */
	__I  uint32_t  DMA_CURHOST_REC_BUF;		/*!< Current host receive buffer address register */
//
} HW_ETH_REGS;

/*
 * @brief MAC_CONFIG register bit defines
 */
#define HWETH_MAC_CFG_RE     (1 << 2)		/*!< Receiver enable */
#define HWETH_MAC_CFG_TE     (1 << 3)		/*!< Transmitter Enable */
#define HWETH_MAC_CFG_DF     (1 << 4)		/*!< Deferral Check */
#define HWETH_MAC_CFG_BL(n)  ((n) << 5)	/*!< Back-Off Limit */
#define HWETH_MAC_CFG_ACS    (1 << 7)		/*!< Automatic Pad/CRC Stripping */
#define HWETH_MAC_CFG_LUD    (1 << 8)		/*!< Link Up/Down, 1 = up */
#define HWETH_MAC_CFG_DR     (1 << 9)		/*!< Disable Retry */
#define HWETH_MAC_CFG_IPC    (1 << 10)	/*!< Checksum Offload */
#define HWETH_MAC_CFG_DM     (1 << 11)	/*!< Duplex Mode, 1 = full, 0 = half */
#define HWETH_MAC_CFG_LM     (1 << 12)	/*!< Loopback Mode */
#define HWETH_MAC_CFG_DO     (1 << 13)	/*!< Disable Receive Own */
#define HWETH_MAC_CFG_FES    (1 << 14)	/*!< Speed, 1 = 100Mbps, 0 = 10Mbos */
#define HWETH_MAC_CFG_PS     (1 << 15)	/*!< Port select, must always be 1 */
#define HWETH_MAC_CFG_DCRS   (1 << 16)	/*!< Disable carrier sense during transmission */
#define HWETH_MAC_CFG_IFG(n) ((n) << 17)	/*!< Inter-frame gap, 40..96, n incs by 8 */
#define HWETH_MAC_CFG_JE     (1 << 20)	/*!< Jumbo Frame Enable */
#define HWETH_MAC_CFG_JD     (1 << 22)	/*!< Jabber Disable */
#define HWETH_MAC_CFG_WD     (1 << 23)	/*!< Watchdog Disable */

/*
 * @brief MAC_FRAME_FILTER register bit defines
 */
#define HWETH_MAC_FF_PR      (1 << 0)		/*!< Promiscuous Mode */
#define HWETH_MAC_FF_DAIF    (1 << 3)		/*!< DA Inverse Filtering */
#define HWETH_MAC_FF_PM      (1 << 4)		/*!< Pass All Multicast */
#define HWETH_MAC_FF_DBF     (1 << 5)		/*!< Disable Broadcast Frames */
#define HWETH_MAC_FF_PCF(n)  ((n) << 6)	/*!< Pass Control Frames, n = see user manual */
#define HWETH_MAC_FF_SAIF    (1 << 8)		/*!< SA Inverse Filtering */
#define HWETH_MAC_FF_SAF     (1 << 9)		/*!< Source Address Filter Enable */
#define HWETH_MAC_FF_RA      (1UL << 31)	/*!< Receive all */

/*
 * @brief MAC_MII_ADDR register bit defines
 */
#define HWETH_MAC_MIIA_GB    (1 << 0)		/*!< MII busy */
#define HWETH_MAC_MIIA_W     (1 << 1)		/*!< MII write */
#define HWETH_MAC_MIIA_CR(n) ((n) << 2)	/*!< CSR clock range, n = see manual */
#define HWETH_MAC_MIIA_GR(n) ((n) << 6)	/*!< MII register. n = 0..31 */
#define HWETH_MAC_MIIA_PA(n) ((n) << 11)	/*!< Physical layer address, n = 0..31 */

/*
 * @brief MAC_MII_DATA register bit defines
 */
#define HWETH_MAC_MIID_GDMSK (0xFFFF)		/*!< MII data mask */

/**
 * @brief MAC_FLOW_CONTROL register bit defines
 */
#define HWETH_MAC_FC_FCB     (1 << 0)		/*!< Flow Control Busy/Backpressure Activate */
#define HWETH_MAC_FC_TFE     (1 << 1)		/*!< Transmit Flow Control Enable */
#define HWETH_MAC_FC_RFE     (1 << 2)		/*!< Receive Flow Control Enable */
#define HWETH_MAC_FC_UP      (1 << 3)		/*!< Unicast Pause Frame Detect */
#define HWETH_MAC_FC_PLT(n)  ((n) << 4)	/*!< Pause Low Threshold, n = see manual */
#define HWETH_MAC_FC_DZPQ    (1 << 7)		/*!< Disable Zero-Quanta Pause */
#define HWETH_MAC_FC_PT(n)   ((n) << 16)	/*!< Pause time */

/*
 * @brief MAC_VLAN_TAG register bit defines
 */
#define HWETH_MAC_VT_VL(n)   ((n) << 0)	/*!< VLAN Tag Identifier for Receive Frames */
#define HWETH_MAC_VT_ETC     (1 << 7)		/*!< Enable 12-Bit VLAN Tag Comparison */

/*
 * @brief MAC_PMT_CTRL_STAT register bit defines
 */
#define HWETH_MAC_PMT_PD     (1 << 0)		/*!< Power-down */
#define HWETH_MAC_PMT_MPE    (1 << 1)		/*!< Magic packet enable */
#define HWETH_MAC_PMT_WFE    (1 << 2)		/*!< Wake-up frame enable */
#define HWETH_MAC_PMT_MPR    (1 << 5)		/*!< Magic Packet Received */
#define HWETH_MAC_PMT_WFR    (1 << 6)		/*!< Wake-up Frame Received */
#define HWETH_MAC_PMT_GU     (1 << 9)		/*!< Global Unicast */
#define HWETH_MAC_PMT_WFFRPR (1UL << 31)	/*!< Wake-up Frame Filter Register Pointer Reset */

/*
 * @brief DMA_BUS_MODE register bit defines
 */
#define HWETH_DMA_BM_SWR     (1 << 0)		/*!< Software reset */
#define HWETH_DMA_BM_DA      (1 << 1)		/*!< DMA arbitration scheme, 1 = TX has priority over TX */
#define HWETH_DMA_BM_DSL(n)  ((n) << 2)	/*!< Descriptor skip length, n = see manual */
#define HWETH_DMA_BM_ATDS    (1 << 7)		/*!< Alternate (Enhanced) descriptor size */
#define HWETH_DMA_BM_PBL(n)  ((n) << 8)	/*!< Programmable burst length, n = see manual */
#define HWETH_DMA_BM_PR(n)   ((n) << 14)	/*!< Rx-to-Tx priority ratio, n = see manual */
#define HWETH_DMA_BM_FB      (1 << 16)	/*!< Fixed burst */
#define HWETH_DMA_BM_RPBL(n) ((n) << 17)	/*!< RxDMA PBL, n = see manual */
#define HWETH_DMA_BM_USP     (1 << 23)	/*!< Use separate PBL */
#define HWETH_DMA_BM_PBL8X   (1 << 24)	/*!< 8 x PBL mode */
#define HWETH_DMA_BM_AAL     (1 << 25)	/*!< Address-aligned beats */
#define HWETH_DMA_BM_MB      (1 << 26)	/*!< Mixed burst */
#define HWETH_DMA_BM_TXPR    (1 << 27)	/*!< Transmit DMA has higher priority than receive DMA */


/*
 * @brief DMA_STAT register bit defines
 */
#define HWETH_DMA_ST_TI      (1 << 0)		/*!< Transmit interrupt */
#define HWETH_DMA_ST_TPS     (1 << 1)		/*!< Transmit process stopped */
#define HWETH_DMA_ST_TU      (1 << 2)		/*!< Transmit buffer unavailable */
#define HWETH_DMA_ST_TJT     (1 << 3)		/*!< Transmit jabber timeout */
#define HWETH_DMA_ST_OVF     (1 << 4)		/*!< Receive overflow */
#define HWETH_DMA_ST_UNF     (1 << 5)		/*!< Transmit underflow */
#define HWETH_DMA_ST_RI      (1 << 6)		/*!< Receive interrupt */
#define HWETH_DMA_ST_RU      (1 << 7)		/*!< Receive buffer unavailable */
#define HWETH_DMA_ST_RPS     (1 << 8)		/*!< Received process stopped */
#define HWETH_DMA_ST_RWT     (1 << 9)		/*!< Receive watchdog timeout */
#define HWETH_DMA_ST_ETI     (1 << 10)	/*!< Early transmit interrupt */
#define HWETH_DMA_ST_FBI     (1 << 13)	/*!< Fatal bus error interrupt */
#define HWETH_DMA_ST_ERI     (1 << 14)	/*!< Early receive interrupt */
#define HWETH_DMA_ST_AIE     (1 << 15)	/*!< Abnormal interrupt summary */
#define HWETH_DMA_ST_NIS     (1 << 16)	/*!< Normal interrupt summary */
#define HWETH_DMA_ST_ALL     (0x1E7FF)	/*!< All interrupts */

/*
 * @brief DMA_OP_MODE register bit defines
 */
#define HWETH_DMA_OM_SR      (1 << 1)		/*!< Start/stop receive */
#define HWETH_DMA_OM_OSF     (1 << 2)		/*!< Operate on second frame */
#define HWETH_DMA_OM_RTC(n)  ((n) << 3)	/*!< Receive threshold control, n = see manual */
#define HWETH_DMA_OM_FUF     (1 << 6)		/*!< Forward undersized good frames */
#define HWETH_DMA_OM_FEF     (1 << 7)		/*!< Forward error frames */
#define HWETH_DMA_OM_ST      (1 << 13)	/*!< Start/Stop Transmission Command */
#define HWETH_DMA_OM_TTC(n)  ((n) << 14)	/*!< Transmit threshold control, n = see manual */
#define HWETH_DMA_OM_FTF     (1 << 20)	/*!< Flush transmit FIFO */
#define HWETH_DMA_OM_TSF     (1 << 21)	/*!< Transmit store and forward */
#define HWETH_DMA_OM_DFF     (1 << 24)	/*!< Disable flushing of received frames */
#define HWETH_DMA_OM_RSF     (1 << 25)	/*!< Receive store and forward */
#define HWETH_DMA_OM_DT      (1 << 26)	/*!< Disable Dropping of TCP/IP Checksum Error Frames */

/*
 * @brief DMA_INT_EN register bit defines
 */
#define HWETH_DMA_IE_TIE     (1 << 0)		/*!< Transmit interrupt enable */
#define HWETH_DMA_IE_TSE     (1 << 1)		/*!< Transmit stopped enable */
#define HWETH_DMA_IE_TUE     (1 << 2)		/*!< Transmit buffer unavailable enable */
#define HWETH_DMA_IE_TJE     (1 << 3)		/*!< Transmit jabber timeout enable */
#define HWETH_DMA_IE_OVE     (1 << 4)		/*!< Overflow interrupt enable */
#define HWETH_DMA_IE_UNE     (1 << 5)		/*!< Underflow interrupt enable */
#define HWETH_DMA_IE_RIE     (1 << 6)		/*!< Receive interrupt enable */
#define HWETH_DMA_IE_RUE     (1 << 7)		/*!< Receive buffer unavailable enable */
#define HWETH_DMA_IE_RSE     (1 << 8)		/*!< Received stopped enable */
#define HWETH_DMA_IE_RWE     (1 << 9)		/*!< Receive watchdog timeout enable */
#define HWETH_DMA_IE_ETE     (1 << 10)	/*!< Early transmit interrupt enable */
#define HWETH_DMA_IE_FBE     (1 << 13)	/*!< Fatal bus error enable */
#define HWETH_DMA_IE_ERE     (1 << 14)	/*!< Early receive interrupt enable */
#define HWETH_DMA_IE_AIE     (1 << 15)	/*!< Abnormal interrupt summary enable */
#define HWETH_DMA_IE_NIE     (1 << 16)	/*!< Normal interrupt summary enable */

#define HWETH_DMADES_OWN        (1UL << 31)  // OWN bit
#define HWETH_DMADES_RCH          (1 << 14)  // Rx Second Address Chained
#define HWETH_DMADES_TCH          (1 << 20)  // Tx Second Address Chained
#define HWETH_DMADES_RER          (1 << 25)  // Receive End of ring
#define HWETH_DMADES_TER          (1 << 21)  // Transmit End of ring
#define HWETH_DMADES_CIC(n)     ((n) << 27)  // Checksum Insertion Control, normal descriptor


// Generic PHY registers
#define HWETH_PHY_BCR_REG        0x0	/*!< Basic Control Register */
#define HWETH_PHY_BSR_REG        0x1	/*!< Basic Status Reg */
#define HWETH_PHY_PHYID1_REG     0x2	/*!< PHY ID 1 Reg  */
#define HWETH_PHY_PHYID2_REG     0x3	/*!< PHY ID 2 Reg */

// BCR bit definitions
#define HWETH_PHY_BCR_RESET          (1 << 15)	/*!< 1= S/W Reset */
#define HWETH_PHY_BCR_LOOPBACK       (1 << 14)	/*!< 1=loopback Enabled */
#define HWETH_PHY_BCR_100M_SPEED_SELECT   (1 << 13)	/*!< 1=Select 100MBps */
#define HWETH_PHY_BCR_AUTONEG        (1 << 12)	/*!< 1=Enable auto-negotiation */
#define HWETH_PHY_BCR_POWER_DOWN     (1 << 11)	/*!< 1=Power down PHY */
#define HWETH_PHY_BCR_ISOLATE        (1 << 10)	/*!< 1=Isolate PHY */
#define HWETH_PHY_BCR_RESTART_AUTONEG (1 << 9)	/*!< 1=Restart auto-negoatiation */
#define HWETH_PHY_BCR_DUPLEX_MODE    (1 << 8)	/*!< 1=Full duplex mode */

// BSR bit definitions
#define HWETH_PHY_BSR_AUTONEG_COMP   (1 << 5)	/*!< Auto-negotation complete */
#define HWETH_PHY_BSR_RMT_FAULT      (1 << 4)	/*!< Fault */
#define HWETH_PHY_BSR_AUTONEG_ABILITY (1 << 3)	/*!< Auto-negotation supported */
#define HWETH_PHY_BSR_LINK_STATUS    (1 << 2)	/*!< 1=Link active */
#define HWETH_PHY_BSR_JABBER_DETECT  (1 << 1)	/*!< Jabber detect */
#define HWETH_PHY_BSR_EXTEND_CAPAB   (1 << 0)	/*!< Supports extended capabilities */

#define HWETH_PHY_SPEEDINFO_REG       0x1F
#define HWETH_PHY_SPEEDINFO_100M    0x0008
#define HWETH_PHY_SPEEDINFO_FULLDX  0x0010

#define HWETH_PHY_SPEEDINFO_MASK    (HWETH_PHY_SPEEDINFO_100M | HWETH_PHY_SPEEDINFO_FULLDX)


class THwEth_stm32 : public THwEth_pre
{
public:
	void       Start();
	void       Stop();

public:
	uint32_t   CalcMdcClock(void);
	void       SetMacAddress(uint8_t * amacaddr);
	void       SetSpeed(bool speed100);
	void       SetDuplex(bool full);

	void       AdjustSpeed(uint16_t aphy_speedinfo);

	void       InitDescList(bool istx, int bufnum, HW_ETH_DMA_DESC * pdesc_list, uint8_t * pbuffer);

public:
	uint8_t    phy_poll_state = 0;

	void       PhyStatusPoll(void); // this must be called regularly !

	void       SetupMii(uint32_t div, uint8_t addr);
	void       StartMiiWrite(uint8_t reg, uint16_t data);
	void       StartMiiRead(uint8_t reg);
	bool       IsMiiBusy();

	bool       PhyInit();

	bool       MiiWaitBusy(int amaxms);

	bool       MiiWrite(uint8_t reg, uint16_t data); // blocking mii write (for setup only)
	bool       MiiRead(uint8_t reg, uint16_t * data); // blocking mii read (for setup only)
	bool 			 PhyWaitReset();

public:
	HW_ETH_REGS *      regs = nullptr;

	HW_ETH_DMA_DESC *  rx_desc_list = nullptr;
	HW_ETH_DMA_DESC *  tx_desc_list = nullptr;

	bool               Init(void * prxdesclist, uint32_t rxcnt, void * ptxdesclist, uint32_t txcnt);
	void               AssignRxBuf(uint32_t idx, void * pdata, uint32_t datalen);

	bool               TryRecv(uint32_t * pidx, void * * ppdata, uint32_t * pdatalen);
	void               ReleaseRxBuf(uint32_t idx);
	bool               TrySend(uint32_t * pidx, void * pdata, uint32_t datalen);


	HW_ETH_DMA_DESC *  actual_rx_desc;

	uint32_t           phy_config;
	uint16_t           phy_speedinfo;
	uint16_t           phy_speedinfo_prev;
	uint16_t           phy_bsr_value;
};

#define HWETH_IMPL THwEth_stm32

#endif // def HWETH_STM32_H_
