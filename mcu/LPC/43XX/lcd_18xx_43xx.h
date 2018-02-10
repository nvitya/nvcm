/*
 * @brief LPC18xx/43xx LCD chip driver
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

#ifndef __LCD_18XX_43XX_H_
#define __LCD_18XX_43XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup LCD_18XX_43XX CHIP: LPC18xx/43xx LCD driver
 * @ingroup CHIP_18XX_43XX_Drivers
 * @{
 */

/**
 * @brief LCD Controller register block structure
 */
typedef struct {				/*!< LCD Structure          */
	__IO uint32_t  TIMH;		/*!< Horizontal Timing Control register */
	__IO uint32_t  TIMV;		/*!< Vertical Timing Control register */
	__IO uint32_t  POL;			/*!< Clock and Signal Polarity Control register */
	__IO uint32_t  LE;			/*!< Line End Control register */
	__IO uint32_t  UPBASE;		/*!< Upper Panel Frame Base Address register */
	__IO uint32_t  LPBASE;		/*!< Lower Panel Frame Base Address register */
	__IO uint32_t  CTRL;		/*!< LCD Control register   */
	__IO uint32_t  INTMSK;		/*!< Interrupt Mask register */
	__I  uint32_t  INTRAW;		/*!< Raw Interrupt Status register */
	__I  uint32_t  INTSTAT;		/*!< Masked Interrupt Status register */
	__O  uint32_t  INTCLR;		/*!< Interrupt Clear register */
	__I  uint32_t  UPCURR;		/*!< Upper Panel Current Address Value register */
	__I  uint32_t  LPCURR;		/*!< Lower Panel Current Address Value register */
	__I  uint32_t  RESERVED0[115];
	__IO uint16_t PAL[256];		/*!< 256x16-bit Color Palette registers */
	__I  uint32_t  RESERVED1[256];
	__IO uint32_t CRSR_IMG[256];/*!< Cursor Image registers */
	__IO uint32_t  CRSR_CTRL;	/*!< Cursor Control register */
	__IO uint32_t  CRSR_CFG;	/*!< Cursor Configuration register */
	__IO uint32_t  CRSR_PAL0;	/*!< Cursor Palette register 0 */
	__IO uint32_t  CRSR_PAL1;	/*!< Cursor Palette register 1 */
	__IO uint32_t  CRSR_XY;		/*!< Cursor XY Position register */
	__IO uint32_t  CRSR_CLIP;	/*!< Cursor Clip Position register */
	__I  uint32_t  RESERVED2[2];
	__IO uint32_t  CRSR_INTMSK;	/*!< Cursor Interrupt Mask register */
	__O  uint32_t  CRSR_INTCLR;	/*!< Cursor Interrupt Clear register */
	__I  uint32_t  CRSR_INTRAW;	/*!< Cursor Raw Interrupt Status register */
	__I  uint32_t  CRSR_INTSTAT;/*!< Cursor Masked Interrupt Status register */
} LPC_LCD_T;

/**
 * @brief LCD Palette entry format
 */
typedef struct {
	uint32_t Rl : 5;
	uint32_t Gl : 5;
	uint32_t Bl : 5;
	uint32_t Il : 1;
	uint32_t Ru : 5;
	uint32_t Gu : 5;
	uint32_t Bu : 5;
	uint32_t Iu : 1;
} LCD_PALETTE_ENTRY_T;

/**
 * @brief LCD Panel type
 */
typedef enum {
	LCD_TFT = 0x02,		/*!< standard TFT */
	LCD_MONO_4 = 0x01,	/*!< 4-bit STN mono */
	LCD_MONO_8 = 0x05,	/*!< 8-bit STN mono */
	LCD_CSTN = 0x00		/*!< color STN */
} LCD_PANEL_OPT_T;

/**
 * @brief LCD Color Format
 */
typedef enum {
	LCD_COLOR_FORMAT_RGB = 0,
	LCD_COLOR_FORMAT_BGR
} LCD_COLOR_FORMAT_OPT_T;

/** LCD Interrupt control mask register bits */
#define LCD_INTMSK_FUFIM   0x2	/*!< FIFO underflow interrupt enable */
#define LCD_INTMSK_LNBUIM  0x4	/*!< LCD next base address update interrupt enable */
#define LCD_INTMSK_VCOMPIM 0x8	/*!< Vertical compare interrupt enable */
#define LCD_INTMSK_BERIM   0x10	/*!< AHB master error interrupt enable */

#define CLCDC_LCDCTRL_ENABLE    _BIT(0)		/*!< LCD control enable bit */
#define CLCDC_LCDCTRL_PWR       _BIT(11)	/*!< LCD control power enable bit */

/**
 * @brief A structure for LCD Configuration
 */
typedef struct {
	uint8_t  HBP;	/*!< Horizontal back porch in clocks */
	uint8_t  HFP;	/*!< Horizontal front porch in clocks */
	uint8_t  HSW;	/*!< HSYNC pulse width in clocks */
	uint16_t PPL;	/*!< Pixels per line */
	uint8_t  VBP;	/*!< Vertical back porch in clocks */
	uint8_t  VFP;	/*!< Vertical front porch in clocks */
	uint8_t  VSW;	/*!< VSYNC pulse width in clocks */
	uint16_t LPP;	/*!< Lines per panel */
	uint8_t  IOE;	/*!< Invert output enable, 1 = invert */
	uint8_t  IPC;	/*!< Invert panel clock, 1 = invert */
	uint8_t  IHS;	/*!< Invert HSYNC, 1 = invert */
	uint8_t  IVS;	/*!< Invert VSYNC, 1 = invert */
	uint8_t  ACB;	/*!< AC bias frequency in clocks (not used) */
	uint8_t  BPP;	/*!< Maximum bits per pixel the display supports */
	LCD_PANEL_OPT_T  LCD;	/*!< LCD panel type */
	LCD_COLOR_FORMAT_OPT_T  color_format;	/*!<BGR or RGB */
	uint8_t  Dual;	/*!< Dual panel, 1 = dual panel display */
} LCD_CONFIG_T;

/**
 * @brief LCD Cursor Size
 */
typedef enum {
	LCD_CURSOR_32x32 = 0,
	LCD_CURSOR_64x64
} LCD_CURSOR_SIZE_OPT_T;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __LCD_18XX_43XX_H_ */
