/*
 * @brief  LPC18xx/43xx High speed ADC driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

#ifndef __HSADC_18XX_43XX_H_
#define __HSADC_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief High speed ADC interrupt control structure
 */
typedef struct {
	__O  uint32_t CLR_EN;			/*!< Interrupt clear mask */
	__O  uint32_t SET_EN;			/*!< Interrupt set mask */
	__I  uint32_t MASK;				/*!< Interrupt mask */
	__I  uint32_t STATUS;			/*!< Interrupt status */
	__O  uint32_t CLR_STAT;			/*!< Interrupt clear status */
	__O  uint32_t SET_STAT;			/*!< Interrupt set status */
	uint32_t RESERVED[2];
} HSADCINTCTRL_T;

/**
 * @brief HSADC register block structure
 */
typedef struct {					/*!< HSADC Structure */
	__O  uint32_t FLUSH;			/*!< Flushes FIFO */
	__IO uint32_t DMA_REQ;			/*!< Set or clear DMA write request */
	__I  uint32_t FIFO_STS;			/*!< Indicates FIFO fill level status */
	__IO uint32_t FIFO_CFG;			/*!< Configures FIFO fill level */
	__O  uint32_t TRIGGER;			/*!< Enable software trigger to start descriptor processing */
	__IO uint32_t DSCR_STS;			/*!< Indicates active descriptor table and descriptor entry */
	__IO uint32_t POWER_DOWN;		/*!< Set or clear power down mode */
	__IO uint32_t CONFIG;			/*!< Configures external trigger mode, store channel ID in FIFO and walk-up recovery time from power down */
	__IO uint32_t THR[2];			/*!< Configures window comparator A or B levels */
	__I  uint32_t LAST_SAMPLE[6];	/*!< Contains last converted sample of input M [M=0..5) and result of window comparator */
	uint32_t RESERVED0[49];
	__IO uint32_t ADC_SPEED;		/*!< ADC speed control */
	__IO uint32_t POWER_CONTROL;	/*!< Configures ADC power vs. speed, DC-in biasing, output format and power gating */
	uint32_t RESERVED1[61];
	__I  uint32_t FIFO_OUTPUT[16];	/*!< FIFO output mapped to 16 consecutive address locations */
	uint32_t RESERVED2[48];
	__IO uint32_t DESCRIPTOR[2][8];	/*!< Table 0 and 1 descriptors */
	uint32_t RESERVED3[752];
	HSADCINTCTRL_T INTS[2];			/*!< Interrupt 0 and 1 control and status registers */
} LPC_HSADC_T;

#define HSADC_MAX_SAMPLEVAL 0xFFF

/* HSADC trigger configuration mask types */
typedef enum {
	HSADC_CONFIG_TRIGGER_OFF = 0,				/*!< ADCHS triggers off */
	HSADC_CONFIG_TRIGGER_SW = 1,				/*!< ADCHS software trigger only */
	HSADC_CONFIG_TRIGGER_EXT = 2,				/*!< ADCHS external trigger only */
	HSADC_CONFIG_TRIGGER_BOTH = 3				/*!< ADCHS both software and external triggers allowed */
} HSADC_TRIGGER_MASK_T;

/* HSADC trigger configuration mode types */
typedef enum {
	HSADC_CONFIG_TRIGGER_RISEEXT = (0 << 2),	/*!< ADCHS rising external trigger */
	HSADC_CONFIG_TRIGGER_FALLEXT = (1 << 2),	/*!< ADCHS falling external trigger */
	HSADC_CONFIG_TRIGGER_LOWEXT = (2 << 2),		/*!< ADCHS low external trigger */
	HSADC_CONFIG_TRIGGER_HIGHEXT = (3 << 2)		/*!< ADCHS high external trigger */
} HSADC_TRIGGER_MODE_T;

/* HSADC trigger configuration sync types */
typedef enum {
	HSADC_CONFIG_TRIGGER_NOEXTSYNC = (0 << 4),	/*!< do not synchronize external trigger input */
	HSADC_CONFIG_TRIGGER_EXTSYNC = (1 << 4),	/*!< synchronize external trigger input */
} HSADC_TRIGGER_SYNC_T;

/* HSADC trigger configuration channel ID */
typedef enum {
	HSADC_CHANNEL_ID_EN_NONE = (0 << 5),	/*!< do not add channel ID to FIFO output data */
	HSADC_CHANNEL_ID_EN_ADD = (1 << 5),		/*!< add channel ID to FIFO output data */
} HSADC_CHANNEL_ID_EN_T;

/* AC-DC coupling selection for vin_neg and vin_pos sides */
typedef enum {
	HSADC_CHANNEL_NODCBIAS = 0,		/*!< No DC bias */
	HSADC_CHANNEL_DCBIAS = 1,		/*!< DC bias on vin_neg side */
} HSADC_DCBIAS_T;

/** HSADC FIFO registers bit fields for unpacked sample in lower 16 bits */
#define HSADC_FIFO_SAMPLE_MASK      (0xFFF)					/*!< 12-bit sample mask (unpacked) */
#define HSADC_FIFO_SAMPLE(val)      ((val) & 0xFFF)			/*!< Macro for stripping out unpacked sample data */
#define HSADC_FIFO_CHAN_ID_MASK     (0x7000)				/*!< Channel ID mask */
#define HSADC_FIFO_CHAN_ID(val)     (((val) >> 12) & 0x7)	/*!< Macro for stripping out sample data */
#define HSADC_FIFO_EMPTY            (0x1 << 15)				/*!< FIFO empty (invalid sample) */
#define HSADC_FIFO_SHIFTPACKED(val) ((val) >> 16)			/*!< Shifts the packed FIFO sample into the lower 16-bits of a word */
#define HSADC_FIFO_PACKEDMASK       (1UL << 31)				/*!< Packed sample check mask */

/** HSADC descriptor registers bit fields and support macros */
#define HSADC_DESC_CH(ch)           (ch)				/*!< Converter input channel */
#define HSADC_DESC_HALT             (1 << 3)			/*!< Descriptor halt after conversion bit */
#define HSADC_DESC_INT              (1 << 4)			/*!< Raise interrupt when ADC result is available bit */
#define HSADC_DESC_POWERDOWN        (1 << 5)			/*!< Power down after this conversion bit */
#define HSADC_DESC_BRANCH_NEXT      (0 << 6)			/*!< Continue with next descriptor */
#define HSADC_DESC_BRANCH_FIRST     (1 << 6)			/*!< Branch to the first descriptor */
#define HSADC_DESC_BRANCH_SWAP      (2 << 6)			/*!< Swap tables and branch to the first descriptor of the new table */
#define HSADC_DESC_MATCH(val)       ((val) << 8)		/*!< Match value used to trigger a descriptor */
#define HSADC_DESC_THRESH_NONE      (0 << 22)			/*!< No threshold detection performed */
#define HSADC_DESC_THRESH_A         (1 << 22)			/*!< Use A threshold detection */
#define HSADC_DESC_THRESH_B         (2 << 22)			/*!< Use B threshold detection */
#define HSADC_DESC_RESET_TIMER      (1 << 24)			/*!< Reset descriptor timer */
#define HSADC_DESC_UPDATE_TABLE     (1UL << 31)			/*!< Update table with all 8 descriptors of this table */

/* Interrupt selection for interrupt 0 set - these interrupts and statuses
   should only be used with the interrupt 0 register set */
#define HSADC_INT0_FIFO_FULL         (1 << 0)		/*!< number of samples in FIFO is more than FIFO_LEVEL */
#define HSADC_INT0_FIFO_EMPTY        (1 << 1)		/*!< FIFO is empty */
#define HSADC_INT0_FIFO_OVERFLOW     (1 << 2)		/*!< FIFO was full; conversion sample is not stored and lost */
#define HSADC_INT0_DSCR_DONE         (1 << 3)		/*!< The descriptor INTERRUPT field was enabled and its sample is converted */
#define HSADC_INT0_DSCR_ERROR        (1 << 4)		/*!< The ADC was not fully woken up when a sample was converted and the conversion results is unreliable */
#define HSADC_INT0_ADC_OVF           (1 << 5)		/*!< Converted sample value was over range of the 12 bit output code */
#define HSADC_INT0_ADC_UNF           (1 << 6)		/*!< Converted sample value was under range of the 12 bit output code */

/* Interrupt selection for interrupt 1 set - these interrupts and statuses
   should only be used with the interrupt 1 register set */
#define HSADC_INT1_THCMP_BRANGE(ch)  (1 << ((ch * 5) + 0))	/*!< Input channel result below range */
#define HSADC_INT1_THCMP_ARANGE(ch)  (1 << ((ch * 5) + 1))	/*!< Input channel result above range */
#define HSADC_INT1_THCMP_DCROSS(ch)  (1 << ((ch * 5) + 2))	/*!< Input channel result downward threshold crossing detected */
#define HSADC_INT1_THCMP_UCROSS(ch)  (1 << ((ch * 5) + 3))	/*!< Input channel result upward threshold crossing detected */
#define HSADC_INT1_OVERRUN(ch)       (1 << ((ch * 5) + 4))	/*!< New conversion on channel completed and has overwritten the previous contents of register LAST_SAMPLE [0] before it has been read */

#ifdef __cplusplus
}
#endif

#endif /* __HSADC_18XX_43XX_H_ */
