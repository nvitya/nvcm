/*
 * @brief LPC18xx/43xx 16/32-bit Timer/PWM driver
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

#ifndef __TIMER_18XX_43XX_H_
#define __TIMER_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup TIMER_18XX_43XX CHIP: LPC18xx/43xx 16/32-bit Timer driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief 32-bit Standard timer register block structure
 */
typedef struct {					/*!< TIMERn Structure       */
	__IO uint32_t IR;				/*!< Interrupt Register. The IR can be written to clear interrupts. The IR can be read to identify which of eight possible interrupt sources are pending. */
	__IO uint32_t TCR;				/*!< Timer Control Register. The TCR is used to control the Timer Counter functions. The Timer Counter can be disabled or reset through the TCR. */
	__IO uint32_t TC;				/*!< Timer Counter. The 32 bit TC is incremented every PR+1 cycles of PCLK. The TC is controlled through the TCR. */
	__IO uint32_t PR;				/*!< Prescale Register. The Prescale Counter (below) is equal to this value, the next clock increments the TC and clears the PC. */
	__IO uint32_t PC;				/*!< Prescale Counter. The 32 bit PC is a counter which is incremented to the value stored in PR. When the value in PR is reached, the TC is incremented and the PC is cleared. The PC is observable and controllable through the bus interface. */
	__IO uint32_t MCR;				/*!< Match Control Register. The MCR is used to control if an interrupt is generated and if the TC is reset when a Match occurs. */
	__IO uint32_t MR[4];			/*!< Match Register. MR can be enabled through the MCR to reset the TC, stop both the TC and PC, and/or generate an interrupt every time MR matches the TC. */
	__IO uint32_t CCR;				/*!< Capture Control Register. The CCR controls which edges of the capture inputs are used to load the Capture Registers and whether or not an interrupt is generated when a capture takes place. */
	__IO uint32_t CR[4];			/*!< Capture Register. CR is loaded with the value of TC when there is an event on the CAPn.0 input. */
	__IO uint32_t EMR;				/*!< External Match Register. The EMR controls the external match pins MATn.0-3 (MAT0.0-3 and MAT1.0-3 respectively). */
	__I  uint32_t RESERVED0[12];
	__IO uint32_t CTCR;				/*!< Count Control Register. The CTCR selects between Timer and Counter mode, and in Counter mode selects the signal and edge(s) for counting. */
} LPC_TIMER_T;

/** Macro to clear interrupt pending */
#define TIMER_IR_CLR(n)         _BIT(n)

/** Macro for getting a timer match interrupt bit */
#define TIMER_MATCH_INT(n)      (_BIT((n) & 0x0F))
/** Macro for getting a capture event interrupt bit */
#define TIMER_CAP_INT(n)        (_BIT((((n) & 0x0F) + 4)))

/** Timer/counter enable bit */
#define TIMER_ENABLE            ((uint32_t) (1 << 0))
/** Timer/counter reset bit */
#define TIMER_RESET             ((uint32_t) (1 << 1))

/** Bit location for interrupt on MRx match, n = 0 to 3 */
#define TIMER_INT_ON_MATCH(n)   (_BIT(((n) * 3)))
/** Bit location for reset on MRx match, n = 0 to 3 */
#define TIMER_RESET_ON_MATCH(n) (_BIT((((n) * 3) + 1)))
/** Bit location for stop on MRx match, n = 0 to 3 */
#define TIMER_STOP_ON_MATCH(n)  (_BIT((((n) * 3) + 2)))

/** Bit location for CAP.n on CRx rising edge, n = 0 to 3 */
#define TIMER_CAP_RISING(n)     (_BIT(((n) * 3)))
/** Bit location for CAP.n on CRx falling edge, n = 0 to 3 */
#define TIMER_CAP_FALLING(n)    (_BIT((((n) * 3) + 1)))
/** Bit location for CAP.n on CRx interrupt enable, n = 0 to 3 */
#define TIMER_INT_ON_CAP(n)     (_BIT((((n) * 3) + 2)))

/**
 * @brief Standard timer initial match pin state and change state
 */
typedef enum IP_TIMER_PIN_MATCH_STATE {
	TIMER_EXTMATCH_DO_NOTHING = 0,	/*!< Timer match state does nothing on match pin */
	TIMER_EXTMATCH_CLEAR      = 1,	/*!< Timer match state sets match pin low */
	TIMER_EXTMATCH_SET        = 2,	/*!< Timer match state sets match pin high */
	TIMER_EXTMATCH_TOGGLE     = 3	/*!< Timer match state toggles match pin */
} TIMER_PIN_MATCH_STATE_T;

/**
 * @brief Standard timer clock and edge for count source
 */
typedef enum IP_TIMER_CAP_SRC_STATE {
	TIMER_CAPSRC_RISING_PCLK  = 0,	/*!< Timer ticks on PCLK rising edge */
	TIMER_CAPSRC_RISING_CAPN  = 1,	/*!< Timer ticks on CAPn.x rising edge */
	TIMER_CAPSRC_FALLING_CAPN = 2,	/*!< Timer ticks on CAPn.x falling edge */
	TIMER_CAPSRC_BOTH_CAPN    = 3	/*!< Timer ticks on CAPn.x both edges */
} TIMER_CAP_SRC_STATE_T;

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_18XX_43XX_H_ */
