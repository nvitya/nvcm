/*
 * @brief LPC18xx/43xx UART chip driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __UART_18XX_43XX_H_
#define __UART_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup UART_18XX_43XX CHIP: LPC18xx/43xx UART driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief USART register block structure
 */
typedef struct {					/*!< USARTn Structure       */

	union {
		__IO uint32_t  DLL;			/*!< Divisor Latch LSB. Least significant byte of the baud rate divisor value. The full divisor is used to generate a baud rate from the fractional rate divider (DLAB = 1). */
		__O  uint32_t  THR;			/*!< Transmit Holding Register. The next character to be transmitted is written here (DLAB = 0). */
		__I  uint32_t  RBR;			/*!< Receiver Buffer Register. Contains the next received character to be read (DLAB = 0). */
	};

	union {
		__IO uint32_t IER;			/*!< Interrupt Enable Register. Contains individual interrupt enable bits for the 7 potential UART interrupts (DLAB = 0). */
		__IO uint32_t DLM;			/*!< Divisor Latch MSB. Most significant byte of the baud rate divisor value. The full divisor is used to generate a baud rate from the fractional rate divider (DLAB = 1). */
	};

	union {
		__O  uint32_t FCR;			/*!< FIFO Control Register. Controls UART FIFO usage and modes. */
		__I  uint32_t IIR;			/*!< Interrupt ID Register. Identifies which interrupt(s) are pending. */
	};

	__IO uint32_t LCR;				/*!< Line Control Register. Contains controls for frame formatting and break generation. */
	__IO uint32_t MCR;				/*!< Modem Control Register. Only present on USART ports with full modem support. */
	__I  uint32_t LSR;				/*!< Line Status Register. Contains flags for transmit and receive status, including line errors. */
	__I  uint32_t MSR;				/*!< Modem Status Register. Only present on USART ports with full modem support. */
	__IO uint32_t SCR;				/*!< Scratch Pad Register. Eight-bit temporary storage for software. */
	__IO uint32_t ACR;				/*!< Auto-baud Control Register. Contains controls for the auto-baud feature. */
	__IO uint32_t ICR;				/*!< IrDA control register (not all UARTS) */
	__IO uint32_t FDR;				/*!< Fractional Divider Register. Generates a clock input for the baud rate divider. */
	__IO uint32_t OSR;				/*!< Oversampling Register. Controls the degree of oversampling during each bit time. Only on some UARTS. */
	__IO uint32_t TER1;				/*!< Transmit Enable Register. Turns off USART transmitter for use with software flow control. */
	uint32_t  RESERVED0[3];
    __IO uint32_t HDEN;				/*!< Half-duplex enable Register- only on some UARTs */
	__I  uint32_t RESERVED1[1];
	__IO uint32_t SCICTRL;			/*!< Smart card interface control register- only on some UARTs */

	__IO uint32_t RS485CTRL;		/*!< RS-485/EIA-485 Control. Contains controls to configure various aspects of RS-485/EIA-485 modes. */
	__IO uint32_t RS485ADRMATCH;	/*!< RS-485/EIA-485 address match. Contains the address match value for RS-485/EIA-485 mode. */
	__IO uint32_t RS485DLY;			/*!< RS-485/EIA-485 direction control delay. */

	union {
		__IO uint32_t SYNCCTRL;		/*!< Synchronous mode control register. Only on USARTs. */
		__I  uint32_t FIFOLVL;		/*!< FIFO Level register. Provides the current fill levels of the transmit and receive FIFOs. */
	};

	__IO uint32_t TER2;				/*!< Transmit Enable Register. Only on LPC177X_8X UART4 and LPC18XX/43XX USART0/2/3. */
} LPC_USART_T;


/**
 * @brief Macro defines for UART Receive Buffer register
 */
#define UART_RBR_MASKBIT    (0xFF)		        /*!< UART Received Buffer mask bit (8 bits) */

/**
 * @brief Macro defines for UART Divisor Latch LSB register
 */
#define UART_LOAD_DLL(div)  ((div) & 0xFF)		/*!< Macro for loading LSB of divisor */
#define UART_DLL_MASKBIT    (0xFF)	            /*!< Divisor latch LSB bit mask */

/**
 * @brief Macro defines for UART Divisor Latch MSB register
 */
#define UART_LOAD_DLM(div)  (((div) >> 8) & 0xFF)	/*!< Macro for loading MSB of divisors */
#define UART_DLM_MASKBIT    (0xFF)		            /*!< Divisor latch MSB bit mask */

/**
 * @brief Macro defines for UART Interrupt Enable Register
 */
#define UART_IER_RBRINT      (1 << 0)	/*!< RBR Interrupt enable */
#define UART_IER_THREINT     (1 << 1)	/*!< THR Interrupt enable */
#define UART_IER_RLSINT      (1 << 2)	/*!< RX line status interrupt enable */
#define UART_IER_MSINT       (1 << 3)	/*!< Modem status interrupt enable - valid for 11xx, 17xx/40xx UART1, 18xx/43xx UART1  only */
#define UART_IER_CTSINT      (1 << 7)	/*!< CTS signal transition interrupt enable - valid for 17xx/40xx UART1, 18xx/43xx UART1 only */
#define UART_IER_ABEOINT     (1 << 8)	/*!< Enables the end of auto-baud interrupt */
#define UART_IER_ABTOINT     (1 << 9)	/*!< Enables the auto-baud time-out interrupt */
#define UART_IER_BITMASK     (0x307)	/*!< UART interrupt enable register bit mask  - valid for 13xx, 17xx/40xx UART0/2/3, 18xx/43xx UART0/2/3 only*/
#define UART1_IER_BITMASK    (0x30F)	/*!< UART1 interrupt enable register bit mask - valid for 11xx only */
#define UART2_IER_BITMASK    (0x38F)	/*!< UART2 interrupt enable register bit mask - valid for 17xx/40xx UART1, 18xx/43xx UART1 only */

/**
 * @brief Macro defines for UART Interrupt Identification Register
 */
#define UART_IIR_INTSTAT_PEND   (1 << 0)	/*!< Interrupt pending status - Active low */
#define UART_IIR_FIFO_EN        (3 << 6)	/*!< These bits are equivalent to FCR[0] */
#define UART_IIR_ABEO_INT       (1 << 8)	/*!< End of auto-baud interrupt */
#define UART_IIR_ABTO_INT       (1 << 9)	/*!< Auto-baud time-out interrupt */
#define UART_IIR_BITMASK        (0x3CF)		/*!< UART interrupt identification register bit mask */

/* Interrupt ID bit definitions */
#define UART_IIR_INTID_MASK     (7 << 1)	/*!< Interrupt identification: Interrupt ID mask */
#define UART_IIR_INTID_RLS      (3 << 1)	/*!< Interrupt identification: Receive line interrupt */
#define UART_IIR_INTID_RDA      (2 << 1)	/*!< Interrupt identification: Receive data available interrupt */
#define UART_IIR_INTID_CTI      (6 << 1)	/*!< Interrupt identification: Character time-out indicator interrupt */
#define UART_IIR_INTID_THRE     (1 << 1)	/*!< Interrupt identification: THRE interrupt */
#define UART_IIR_INTID_MODEM    (0 << 1)	/*!< Interrupt identification: Modem interrupt */

/**
 * @brief Macro defines for UART FIFO Control Register
 */
#define UART_FCR_FIFO_EN        (1 << 0)	/*!< UART FIFO enable */
#define UART_FCR_RX_RS          (1 << 1)	/*!< UART RX FIFO reset */
#define UART_FCR_TX_RS          (1 << 2)	/*!< UART TX FIFO reset */
#define UART_FCR_DMAMODE_SEL    (1 << 3)	/*!< UART DMA mode selection - valid for 17xx/40xx, 18xx/43xx only */
#define UART_FCR_BITMASK        (0xCF)		/*!< UART FIFO control bit mask */

#define UART_TX_FIFO_SIZE       (16)

/* FIFO trigger level bit definitions */
#define UART_FCR_TRG_LEV0       (0)			/*!< UART FIFO trigger level 0: 1 character */
#define UART_FCR_TRG_LEV1       (1 << 6)	/*!< UART FIFO trigger level 1: 4 character */
#define UART_FCR_TRG_LEV2       (2 << 6)	/*!< UART FIFO trigger level 2: 8 character */
#define UART_FCR_TRG_LEV3       (3 << 6)	/*!< UART FIFO trigger level 3: 14 character */

/**
 * @brief Macro defines for UART Line Control Register
 */
/* UART word length select bit definitions */
#define UART_LCR_WLEN_MASK      (3 << 0)		/*!< UART word length select bit mask */
#define UART_LCR_WLEN5          (0 << 0)		/*!< UART word length select: 5 bit data mode */
#define UART_LCR_WLEN6          (1 << 0)		/*!< UART word length select: 6 bit data mode */
#define UART_LCR_WLEN7          (2 << 0)		/*!< UART word length select: 7 bit data mode */
#define UART_LCR_WLEN8          (3 << 0)		/*!< UART word length select: 8 bit data mode */

/* UART Stop bit select bit definitions */
#define UART_LCR_SBS_MASK       (1 << 2)		/*!< UART stop bit select: bit mask */
#define UART_LCR_SBS_1BIT       (0 << 2)		/*!< UART stop bit select: 1 stop bit */
#define UART_LCR_SBS_2BIT       (1 << 2)		/*!< UART stop bit select: 2 stop bits (in 5 bit data mode, 1.5 stop bits) */

/* UART Parity enable bit definitions */
#define UART_LCR_PARITY_EN      (1 << 3)		/*!< UART Parity Enable */
#define UART_LCR_PARITY_DIS     (0 << 3)		/*!< UART Parity Disable */
#define UART_LCR_PARITY_ODD     (0 << 4)		/*!< UART Parity select: Odd parity */
#define UART_LCR_PARITY_EVEN    (1 << 4)		/*!< UART Parity select: Even parity */
#define UART_LCR_PARITY_F_1     (2 << 4)		/*!< UART Parity select: Forced 1 stick parity */
#define UART_LCR_PARITY_F_0     (3 << 4)		/*!< UART Parity select: Forced 0 stick parity */
#define UART_LCR_BREAK_EN       (1 << 6)		/*!< UART Break transmission enable */
#define UART_LCR_DLAB_EN        (1 << 7)		/*!< UART Divisor Latches Access bit enable */
#define UART_LCR_BITMASK        (0xFF)			/*!< UART line control bit mask */

/**
 * @brief Macro defines for UART Modem Control Register
 */
#define UART_MCR_DTR_CTRL       (1 << 0)		/*!< Source for modem output pin DTR */
#define UART_MCR_RTS_CTRL       (1 << 1)		/*!< Source for modem output pin RTS */
#define UART_MCR_LOOPB_EN       (1 << 4)		/*!< Loop back mode select */
#define UART_MCR_AUTO_RTS_EN    (1 << 6)		/*!< Enable Auto RTS flow-control */
#define UART_MCR_AUTO_CTS_EN    (1 << 7)		/*!< Enable Auto CTS flow-control */
#define UART_MCR_BITMASK        (0xD3)			/*!< UART bit mask value */

/**
 * @brief Macro defines for UART Line Status Register
 */
#define UART_LSR_RDR        (1 << 0)	/*!< Line status: Receive data ready */
#define UART_LSR_OE         (1 << 1)	/*!< Line status: Overrun error */
#define UART_LSR_PE         (1 << 2)	/*!< Line status: Parity error */
#define UART_LSR_FE         (1 << 3)	/*!< Line status: Framing error */
#define UART_LSR_BI         (1 << 4)	/*!< Line status: Break interrupt */
#define UART_LSR_THRE       (1 << 5)	/*!< Line status: Transmit holding register empty */
#define UART_LSR_TEMT       (1 << 6)	/*!< Line status: Transmitter empty */
#define UART_LSR_RXFE       (1 << 7)	/*!< Line status: Error in RX FIFO */
#define UART_LSR_TXFE       (1 << 8)	/*!< Line status: Error in RX FIFO */
#define UART_LSR_BITMASK    (0xFF)		/*!< UART Line status bit mask */
#define UART1_LSR_BITMASK   (0x1FF)		/*!< UART1 Line status bit mask - valid for 11xx, 18xx/43xx UART0/2/3 only */

/**
 * @brief Macro defines for UART Modem Status Register
 */
#define UART_MSR_DELTA_CTS      (1 << 0)	/*!< Modem status: State change of input CTS */
#define UART_MSR_DELTA_DSR      (1 << 1)	/*!< Modem status: State change of input DSR */
#define UART_MSR_LO2HI_RI       (1 << 2)	/*!< Modem status: Low to high transition of input RI */
#define UART_MSR_DELTA_DCD      (1 << 3)	/*!< Modem status: State change of input DCD */
#define UART_MSR_CTS            (1 << 4)	/*!< Modem status: Clear To Send State */
#define UART_MSR_DSR            (1 << 5)	/*!< Modem status: Data Set Ready State */
#define UART_MSR_RI             (1 << 6)	/*!< Modem status: Ring Indicator State */
#define UART_MSR_DCD            (1 << 7)	/*!< Modem status: Data Carrier Detect State */
#define UART_MSR_BITMASK        (0xFF)		/*!< Modem status: MSR register bit-mask value */

/**
 * @brief Macro defines for UART Auto baudrate control register
 */
#define UART_ACR_START              (1 << 0)	/*!< UART Auto-baud start */
#define UART_ACR_MODE               (1 << 1)	/*!< UART Auto baudrate Mode 1 */
#define UART_ACR_AUTO_RESTART       (1 << 2)	/*!< UART Auto baudrate restart */
#define UART_ACR_ABEOINT_CLR        (1 << 8)	/*!< UART End of auto-baud interrupt clear */
#define UART_ACR_ABTOINT_CLR        (1 << 9)	/*!< UART Auto-baud time-out interrupt clear */
#define UART_ACR_BITMASK            (0x307)		/*!< UART Auto Baudrate register bit mask */

/**
 * Autobaud modes
 */
#define UART_ACR_MODE0              (0)	/*!< Auto baudrate Mode 0 */
#define UART_ACR_MODE1              (1)	/*!< Auto baudrate Mode 1 */

/**
 * @brief Macro defines for UART RS485 Control register
 */
#define UART_RS485CTRL_NMM_EN       (1 << 0)	/*!< RS-485/EIA-485 Normal Multi-drop Mode (NMM) is disabled */
#define UART_RS485CTRL_RX_DIS       (1 << 1)	/*!< The receiver is disabled */
#define UART_RS485CTRL_AADEN        (1 << 2)	/*!< Auto Address Detect (AAD) is enabled */
#define UART_RS485CTRL_SEL_DTR      (1 << 3)	/*!< If direction control is enabled (bit DCTRL = 1), pin DTR is
												        used for direction control */
#define UART_RS485CTRL_DCTRL_EN     (1 << 4)	/*!< Enable Auto Direction Control */
#define UART_RS485CTRL_OINV_1       (1 << 5)	/*!< This bit reverses the polarity of the direction
												       control signal on the RTS (or DTR) pin. The direction control pin
												       will be driven to logic "1" when the transmitter has data to be sent */
#define UART_RS485CTRL_BITMASK      (0x3F)		/*!< RS485 control bit-mask value */

/**
 * @brief Macro defines for UART IrDA Control Register - valid for 11xx, 17xx/40xx UART0/2/3, 18xx/43xx UART3 only
 */
#define UART_ICR_IRDAEN         (1 << 0)			/*!< IrDA mode enable */
#define UART_ICR_IRDAINV        (1 << 1)			/*!< IrDA serial input inverted */
#define UART_ICR_FIXPULSE_EN    (1 << 2)			/*!< IrDA fixed pulse width mode */
#define UART_ICR_PULSEDIV(n)    ((n & 0x07) << 3)	/*!< PulseDiv - Configures the pulse when FixPulseEn = 1 */
#define UART_ICR_BITMASK        (0x3F)				/*!< UART IRDA bit mask */

/**
 * @brief Macro defines for UART half duplex register - ????
 */
#define UART_HDEN_HDEN          ((1 << 0))			/*!< enable half-duplex mode*/

/**
 * @brief Macro defines for UART Smart card interface Control Register - valid for 11xx, 18xx/43xx UART0/2/3 only
 */
#define UART_SCICTRL_SCIEN        (1 << 0)			/*!< enable asynchronous half-duplex smart card interface*/
#define UART_SCICTRL_NACKDIS      (1 << 1)			/*!< NACK response is inhibited*/
#define UART_SCICTRL_PROTSEL_T1   (1 << 2)			/*!< ISO7816-3 protocol T1 is selected*/
#define UART_SCICTRL_TXRETRY(n)   ((n & 0x07) << 5)	/*!< number of retransmission*/
#define UART_SCICTRL_GUARDTIME(n) ((n & 0xFF) << 8)	/*!< Extra guard time*/

/**
 * @brief Macro defines for UART Fractional Divider Register
 */
#define UART_FDR_DIVADDVAL(n)   (n & 0x0F)			/*!< Baud-rate generation pre-scaler divisor */
#define UART_FDR_MULVAL(n)      ((n << 4) & 0xF0)	/*!< Baud-rate pre-scaler multiplier value */
#define UART_FDR_BITMASK        (0xFF)				/*!< UART Fractional Divider register bit mask */

/**
 * @brief Macro defines for UART Tx Enable Register
 */
#define UART_TER1_TXEN      (1 << 7)		/*!< Transmit enable bit  - valid for 11xx, 13xx, 17xx/40xx only */
#define UART_TER2_TXEN      (1 << 0)		/*!< Transmit enable bit  - valid for 18xx/43xx only */

/**
 * @brief Macro defines for UART Synchronous Control Register - 11xx, 18xx/43xx UART0/2/3 only
 */
#define UART_SYNCCTRL_SYNC             (1 << 0)			/*!< enable synchronous mode*/
#define UART_SYNCCTRL_CSRC_MASTER      (1 << 1)  		/*!< synchronous master mode*/
#define UART_SYNCCTRL_FES              (1 << 2)			/*!< sample on falling edge*/
#define UART_SYNCCTRL_TSBYPASS         (1 << 3)			/*!< to be defined*/
#define UART_SYNCCTRL_CSCEN            (1 << 4)			/*!< Continuous running clock enable (master mode only)*/
#define UART_SYNCCTRL_STARTSTOPDISABLE (1 << 5)	        /*!< Do not send start/stop bit*/
#define UART_SYNCCTRL_CCCLR            (1 << 6)			/*!< stop continuous clock*/

#ifdef __cplusplus
}
#endif

#endif /* __UART_18XX_43XX_H_ */
