/*
 * @brief LPC18xx/43xx SSP driver
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

#ifndef __SSP_18XX_43XX_H_
#define __SSP_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SSP_18XX_43XX CHIP: LPC18xx/43xx SSP driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief SSP register block structure
 */
typedef struct {			/*!< SSPn Structure         */
	__IO uint32_t CR0;		/*!< Control Register 0. Selects the serial clock rate, bus type, and data size. */
	__IO uint32_t CR1;		/*!< Control Register 1. Selects master/slave and other modes. */
	__IO uint32_t DR;		/*!< Data Register. Writes fill the transmit FIFO, and reads empty the receive FIFO. */
	__I  uint32_t SR;		/*!< Status Register        */
	__IO uint32_t CPSR;		/*!< Clock Prescale Register */
	__IO uint32_t IMSC;		/*!< Interrupt Mask Set and Clear Register */
	__I  uint32_t RIS;		/*!< Raw Interrupt Status Register */
	__I  uint32_t MIS;		/*!< Masked Interrupt Status Register */
	__O  uint32_t ICR;		/*!< SSPICR Interrupt Clear Register */
	__IO uint32_t DMACR;	/*!< SSPn DMA control register */
} LPC_SSP_T;

/**
 * Macro defines for CR0 register
 */

/** SSP data size select, must be 4 bits to 16 bits */
#define SSP_CR0_DSS(n)          ((uint32_t) ((n) & 0xF))
/** SSP control 0 Motorola SPI mode */
#define SSP_CR0_FRF_SPI         ((uint32_t) (0 << 4))
/** SSP control 0 TI synchronous serial mode */
#define SSP_CR0_FRF_TI          ((uint32_t) (1 << 4))
/** SSP control 0 National Micro-wire mode */
#define SSP_CR0_FRF_MICROWIRE   ((uint32_t) (2 << 4))
/** SPI clock polarity bit (used in SPI mode only), (1) = maintains the
   bus clock high between frames, (0) = low */
#define SSP_CR0_CPOL_LO     ((uint32_t) (0))
#define SSP_CR0_CPOL_HI     ((uint32_t) (1 << 6))
/** SPI clock out phase bit (used in SPI mode only), (1) = captures data
   on the second clock transition of the frame, (0) = first */
#define SSP_CR0_CPHA_FIRST  ((uint32_t) (0))
#define SSP_CR0_CPHA_SECOND ((uint32_t) (1 << 7))
/** SSP serial clock rate value load macro, divider rate is
   PERIPH_CLK / (cpsr * (SCR + 1)) */
#define SSP_CR0_SCR(n)      ((uint32_t) ((n & 0xFF) << 8))
/** SSP CR0 bit mask */
#define SSP_CR0_BITMASK     ((uint32_t) (0xFFFF))
/** SSP CR0 bit mask */
#define SSP_CR0_BITMASK     ((uint32_t) (0xFFFF))
/** SSP serial clock rate value load macro, divider rate is
   PERIPH_CLK / (cpsr * (SCR + 1)) */
#define SSP_CR0_SCR(n)      ((uint32_t) ((n & 0xFF) << 8))

/**
 * Macro defines for CR1 register
 */

/** SSP control 1 loopback mode enable bit */
#define SSP_CR1_LBM_EN      ((uint32_t) (1 << 0))
/** SSP control 1 enable bit */
#define SSP_CR1_SSP_EN      ((uint32_t) (1 << 1))
/** SSP control 1 slave enable */
#define SSP_CR1_SLAVE_EN    ((uint32_t) (1 << 2))
#define SSP_CR1_MASTER_EN   ((uint32_t) (0))
/** SSP control 1 slave out disable bit, disables transmit line in slave
   mode */
#define SSP_CR1_SO_DISABLE  ((uint32_t) (1 << 3))
/** SSP CR1 bit mask */
#define SSP_CR1_BITMASK     ((uint32_t) (0x0F))

/** SSP CPSR bit mask */
#define SSP_CPSR_BITMASK    ((uint32_t) (0xFF))
/**
 * Macro defines for DR register
 */

/** SSP data bit mask */
#define SSP_DR_BITMASK(n)   ((n) & 0xFFFF)

/**
 * Macro defines for SR register
 */

/** SSP SR bit mask */
#define SSP_SR_BITMASK  ((uint32_t) (0x1F))

/** ICR bit mask */
#define SSP_ICR_BITMASK ((uint32_t) (0x03))

/**
 * @brief SSP Type of Status
 */
typedef enum _SSP_STATUS {
	SSP_STAT_TFE = ((uint32_t)(1 << 0)),/**< TX FIFO Empty */
	SSP_STAT_TNF = ((uint32_t)(1 << 1)),/**< TX FIFO not full */
	SSP_STAT_RNE = ((uint32_t)(1 << 2)),/**< RX FIFO not empty */
	SSP_STAT_RFF = ((uint32_t)(1 << 3)),/**< RX FIFO full */
	SSP_STAT_BSY = ((uint32_t)(1 << 4)),/**< SSP Busy */
} SSP_STATUS_T;

/**
 * @brief SSP Type of Interrupt Mask
 */
typedef enum _SSP_INTMASK {
	SSP_RORIM = ((uint32_t)(1 << 0)),	/**< Overun */
	SSP_RTIM = ((uint32_t)(1 << 1)),/**< TimeOut */
	SSP_RXIM = ((uint32_t)(1 << 2)),/**< Rx FIFO is at least half full */
	SSP_TXIM = ((uint32_t)(1 << 3)),/**< Tx FIFO is at least half empty */
	SSP_INT_MASK_BITMASK = ((uint32_t)(0xF)),
} SSP_INTMASK_T;

/**
 * @brief SSP Type of Mask Interrupt Status
 */
typedef enum _SSP_MASKINTSTATUS {
	SSP_RORMIS = ((uint32_t)(1 << 0)),	/**< Overun */
	SSP_RTMIS = ((uint32_t)(1 << 1)),	/**< TimeOut */
	SSP_RXMIS = ((uint32_t)(1 << 2)),	/**< Rx FIFO is at least half full */
	SSP_TXMIS = ((uint32_t)(1 << 3)),	/**< Tx FIFO is at least half empty */
	SSP_MASK_INT_STAT_BITMASK = ((uint32_t)(0xF)),
} SSP_MASKINTSTATUS_T;

/**
 * @brief SSP Type of Raw Interrupt Status
 */
typedef enum _SSP_RAWINTSTATUS {
	SSP_RORRIS = ((uint32_t)(1 << 0)),	/**< Overun */
	SSP_RTRIS = ((uint32_t)(1 << 1)),	/**< TimeOut */
	SSP_RXRIS = ((uint32_t)(1 << 2)),	/**< Rx FIFO is at least half full */
	SSP_TXRIS = ((uint32_t)(1 << 3)),	/**< Tx FIFO is at least half empty */
	SSP_RAW_INT_STAT_BITMASK = ((uint32_t)(0xF)),
} SSP_RAWINTSTATUS_T;

typedef enum _SSP_INTCLEAR {
	SSP_RORIC = 0x0,
	SSP_RTIC = 0x1,
	SSP_INT_CLEAR_BITMASK = 0x3,
} SSP_INTCLEAR_T;

typedef enum _SSP_DMA {
	SSP_DMA_RX = (1u),	/**< DMA RX Enable */
	SSP_DMA_TX = (1u << 1),	/**< DMA TX Enable */
	SSP_DMA_BITMASK = ((uint32_t)(0x3)),
} SSP_DMA_T;

/*
 * @brief SSP clock format
 */
typedef enum CHIP_SSP_CLOCK_FORMAT {
	SSP_CLOCK_CPHA0_CPOL0 = (0 << 6),		/**< CPHA = 0, CPOL = 0 */
	SSP_CLOCK_CPHA0_CPOL1 = (1u << 6),		/**< CPHA = 0, CPOL = 1 */
	SSP_CLOCK_CPHA1_CPOL0 = (2u << 6),		/**< CPHA = 1, CPOL = 0 */
	SSP_CLOCK_CPHA1_CPOL1 = (3u << 6),		/**< CPHA = 1, CPOL = 1 */
	SSP_CLOCK_MODE0 = SSP_CLOCK_CPHA0_CPOL0,/**< alias */
	SSP_CLOCK_MODE1 = SSP_CLOCK_CPHA1_CPOL0,/**< alias */
	SSP_CLOCK_MODE2 = SSP_CLOCK_CPHA0_CPOL1,/**< alias */
	SSP_CLOCK_MODE3 = SSP_CLOCK_CPHA1_CPOL1,/**< alias */
} CHIP_SSP_CLOCK_MODE_T;

/*
 * @brief SSP frame format
 */
typedef enum CHIP_SSP_FRAME_FORMAT {
	SSP_FRAMEFORMAT_SPI = (0 << 4),			/**< Frame format: SPI */
	CHIP_SSP_FRAME_FORMAT_TI = (1u << 4),			/**< Frame format: TI SSI */
	SSP_FRAMEFORMAT_MICROWIRE = (2u << 4),	/**< Frame format: Microwire */
} CHIP_SSP_FRAME_FORMAT_T;

/*
 * @brief Number of bits per frame
 */
typedef enum CHIP_SSP_BITS {
	SSP_BITS_4 = (3u << 0),		/*!< 4 bits/frame */
	SSP_BITS_5 = (4u << 0),		/*!< 5 bits/frame */
	SSP_BITS_6 = (5u << 0),		/*!< 6 bits/frame */
	SSP_BITS_7 = (6u << 0),		/*!< 7 bits/frame */
	SSP_BITS_8 = (7u << 0),		/*!< 8 bits/frame */
	SSP_BITS_9 = (8u << 0),		/*!< 9 bits/frame */
	SSP_BITS_10 = (9u << 0),	/*!< 10 bits/frame */
	SSP_BITS_11 = (10u << 0),	/*!< 11 bits/frame */
	SSP_BITS_12 = (11u << 0),	/*!< 12 bits/frame */
	SSP_BITS_13 = (12u << 0),	/*!< 13 bits/frame */
	SSP_BITS_14 = (13u << 0),	/*!< 14 bits/frame */
	SSP_BITS_15 = (14u << 0),	/*!< 15 bits/frame */
	SSP_BITS_16 = (15u << 0),	/*!< 16 bits/frame */
} CHIP_SSP_BITS_T;

/*
 * @brief SSP mode
 */
typedef enum CHIP_SSP_MODE {
	SSP_MODE_MASTER = (0 << 2),	/**< Master mode */
	SSP_MODE_SLAVE = (1u << 2),	/**< Slave mode */
} CHIP_SSP_MODE_T;

/** SSP configuration parameter defines */
/** Clock phase control bit */
#define SSP_CPHA_FIRST          SSP_CR0_CPHA_FIRST
#define SSP_CPHA_SECOND         SSP_CR0_CPHA_SECOND

/** Clock polarity control bit */
/* There's no bug here!!!
 * - If bit[6] in SSPnCR0 is 0: SSP controller maintains the bus clock low between frames.
 * That means the active clock is in HI state.
 * - If bit[6] in SSPnCR0 is 1 (SSP_CR0_CPOL_HI): SSP controller maintains the bus clock
 * high between frames. That means the active clock is in LO state.
 */
#define SSP_CPOL_HI             SSP_CR0_CPOL_LO
#define SSP_CPOL_LO             SSP_CR0_CPOL_HI

/** SSP master mode enable */
#define SSP_SLAVE_MODE          SSP_CR1_SLAVE_EN
#define SSP_MASTER_MODE         SSP_CR1_MASTER_EN

#ifdef __cplusplus
}
#endif

#endif /* __SSP_18XX_43XX_H_ */
