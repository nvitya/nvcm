/*
 * @brief LPC43xx SPI driver
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

#ifndef __SPI_43XX_H_
#define __SPI_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SPI_43XX CHIP: LPC43xx SPI driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * This module is present in LPC43xx MCUs only.
 * @{
 */

/**
 * @brief SPI register block structure
 */
typedef struct {					/*!< SPI Structure          */
	__IO uint32_t  CR;				/*!< SPI Control Register. This register controls the operation of the SPI. */
	__I  uint32_t  SR;				/*!< SPI Status Register. This register shows the status of the SPI. */
	__IO uint32_t  DR;				/*!< SPI Data Register. This bi-directional register provides the transmit and receive data for the SPI. Transmit data is provided to the SPI0 by writing to this register. Data received by the SPI0 can be read from this register. */
	__IO uint32_t  CCR;				/*!< SPI Clock Counter Register. This register controls the frequency of a master's SCK0. */
	__I  uint32_t  RESERVED0[3];
	__IO uint32_t  INT;				/*!< SPI Interrupt Flag. This register contains the interrupt flag for the SPI interface. */
} LPC_SPI_T;

/*
 * Macro defines for SPI Control register
 */
/* SPI CFG Register BitMask */
#define SPI_CR_BITMASK       ((uint32_t) 0xFFC)
/** Enable of controlling the number of bits per transfer  */
#define SPI_CR_BIT_EN         ((uint32_t) (1 << 2))
/** Mask of field of bit controlling */
#define SPI_CR_BITS_MASK      ((uint32_t) 0xF00)
/** Set the number of bits per a transfer */
#define SPI_CR_BITS(n)        ((uint32_t) ((n << 8) & 0xF00))	/* n is in range 8-16 */
/** SPI Clock Phase Select*/
#define SPI_CR_CPHA_FIRST     ((uint32_t) (0))	/*Capture data on the first edge, Change data on the following edge*/
#define SPI_CR_CPHA_SECOND    ((uint32_t) (1 << 3))	/*Change data on the first edge, Capture data on the following edge*/
/** SPI Clock Polarity Select*/
#define SPI_CR_CPOL_LO        ((uint32_t) (0))	/* The rest state of the clock (between frames) is low.*/
#define SPI_CR_CPOL_HI        ((uint32_t) (1 << 4))	/* The rest state of the clock (between frames) is high.*/
/** SPI Slave Mode Select */
#define SPI_CR_SLAVE_EN       ((uint32_t) 0)
/** SPI Master Mode Select */
#define SPI_CR_MASTER_EN      ((uint32_t) (1 << 5))
/** SPI MSB First mode enable */
#define SPI_CR_MSB_FIRST_EN   ((uint32_t) 0)	/*Data will be transmitted and received in standard order (MSB first).*/
/** SPI LSB First mode enable */
#define SPI_CR_LSB_FIRST_EN   ((uint32_t) (1 << 6))	/*Data will be transmitted and received in reverse order (LSB first).*/
/** SPI interrupt enable */
#define SPI_CR_INT_EN         ((uint32_t) (1 << 7))

/*
 * Macro defines for SPI Status register
 */
/** SPI STAT Register BitMask */
#define SPI_SR_BITMASK        ((uint32_t) 0xF8)
/** Slave abort Flag */
#define SPI_SR_ABRT           ((uint32_t) (1 << 3))	/* When 1, this bit indicates that a slave abort has occurred. */
/* Mode fault Flag */
#define SPI_SR_MODF           ((uint32_t) (1 << 4))	/* when 1, this bit indicates that a Mode fault error has occurred. */
/** Read overrun flag*/
#define SPI_SR_ROVR           ((uint32_t) (1 << 5))	/* When 1, this bit indicates that a read overrun has occurred. */
/** Write collision flag. */
#define SPI_SR_WCOL           ((uint32_t) (1 << 6))	/* When 1, this bit indicates that a write collision has occurred.. */
/** SPI transfer complete flag. */
#define SPI_SR_SPIF           ((uint32_t) (1 << 7))		/* When 1, this bit indicates when a SPI data transfer is complete.. */
/** SPI error flag */
#define SPI_SR_ERROR          (SPI_SR_ABRT | SPI_SR_MODF | SPI_SR_ROVR | SPI_SR_WCOL)
/*
 * Macro defines for SPI Test Control Register register
 */
/*Enable SPI Test Mode */
#define SPI_TCR_TEST(n)       ((uint32_t) ((n & 0x3F) << 1))

/*
 * Macro defines for SPI Interrupt register
 */
/** SPI interrupt flag */
#define SPI_INT_SPIF          ((uint32_t) (1 << 0))

/**
 * Macro defines for SPI Data register
 */
/** Receiver Data  */
#define SPI_DR_DATA(n)        ((uint32_t) ((n) & 0xFFFF))

/** @brief SPI Mode*/
typedef enum {
	SPI_MODE_MASTER = SPI_CR_MASTER_EN,			/* Master Mode */
	SPI_MODE_SLAVE = SPI_CR_SLAVE_EN,			/* Slave Mode */
} SPI_MODE_T;

/** @brief SPI Clock Mode*/
typedef enum {
	SPI_CLOCK_CPHA0_CPOL0 = SPI_CR_CPOL_LO | SPI_CR_CPHA_FIRST,		/**< CPHA = 0, CPOL = 0 */
	SPI_CLOCK_CPHA0_CPOL1 = SPI_CR_CPOL_HI | SPI_CR_CPHA_FIRST,		/**< CPHA = 0, CPOL = 1 */
	SPI_CLOCK_CPHA1_CPOL0 = SPI_CR_CPOL_LO | SPI_CR_CPHA_SECOND,	/**< CPHA = 1, CPOL = 0 */
	SPI_CLOCK_CPHA1_CPOL1 = SPI_CR_CPOL_HI | SPI_CR_CPHA_SECOND,	/**< CPHA = 1, CPOL = 1 */
	SPI_CLOCK_MODE0 = SPI_CLOCK_CPHA0_CPOL0,/**< alias */
	SPI_CLOCK_MODE1 = SPI_CLOCK_CPHA1_CPOL0,/**< alias */
	SPI_CLOCK_MODE2 = SPI_CLOCK_CPHA0_CPOL1,/**< alias */
	SPI_CLOCK_MODE3 = SPI_CLOCK_CPHA1_CPOL1,/**< alias */
} SPI_CLOCK_MODE_T;

/** @brief SPI Data Order Mode*/
typedef enum {
	SPI_DATA_MSB_FIRST = SPI_CR_MSB_FIRST_EN,			/* Standard Order */
	SPI_DATA_LSB_FIRST = SPI_CR_LSB_FIRST_EN,			/* Reverse Order */
} SPI_DATA_ORDER_T;

/*
 * @brief Number of bits per frame
 */
typedef enum {
	SPI_BITS_8 = SPI_CR_BITS(8),		/**< 8 bits/frame */
	SPI_BITS_9 = SPI_CR_BITS(9),		/**< 9 bits/frame */
	SPI_BITS_10 = SPI_CR_BITS(10),		/**< 10 bits/frame */
	SPI_BITS_11 = SPI_CR_BITS(11),		/**< 11 bits/frame */
	SPI_BITS_12 = SPI_CR_BITS(12),		/**< 12 bits/frame */
	SPI_BITS_13 = SPI_CR_BITS(13),		/**< 13 bits/frame */
	SPI_BITS_14 = SPI_CR_BITS(14),		/**< 14 bits/frame */
	SPI_BITS_15 = SPI_CR_BITS(15),		/**< 15 bits/frame */
	SPI_BITS_16 = SPI_CR_BITS(16),		/**< 16 bits/frame */
} SPI_BITS_T;

#ifdef __cplusplus
}
#endif

#endif /* __SPI_43XX_H_ */
