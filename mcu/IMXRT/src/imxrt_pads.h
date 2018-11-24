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
 *  file:     imxrt_pads.h
 *  brief:    Short numeric pad definitions for the IMXRT processors for easier handling
 *  version:  1.00
 *  date:     2018-11-23
 *  authors:  nvitya
*/

#ifndef __IMXRT_PADS_H_
#define __IMXRT_PADS_H_

// pad number = (<SW_MUX_CTL_PAD register address> - 0x401F8014) / 4

#define IMXRT_GENPAD_MUX_REG_START    0x401F8014

#if defined(MCUSF_1020)

  #define IMXRT_GENPAD_CTRL_REG_START   0x401F8188
  #define IMXRT_GENPAD_IOSEL_REG_START  0x401F82FC

#elif defined(MCUSF_1050)

  #define IMXRT_GENPAD_CTRL_REG_START   0x401F8204
  #define IMXRT_GENPAD_IOSEL_REG_START  0x401F83F4

#else
  #error "Define pin setup register addresses"
#endif

#define IMXRT_SPECPAD_MUX_REG_START   0x400A8000
#define IMXRT_SPECPAD_CTRL_REG_START  0x400A8018

// the beginning is same for 1020 and 1050

#define PAD_GPIO_EMC_00     0
#define PAD_GPIO_EMC_01     1
#define PAD_GPIO_EMC_02     2
#define PAD_GPIO_EMC_03     3
#define PAD_GPIO_EMC_04     4
#define PAD_GPIO_EMC_05     5
#define PAD_GPIO_EMC_06     6
#define PAD_GPIO_EMC_07     7
#define PAD_GPIO_EMC_08     8
#define PAD_GPIO_EMC_09     9
#define PAD_GPIO_EMC_10     10
#define PAD_GPIO_EMC_11     11
#define PAD_GPIO_EMC_12     12
#define PAD_GPIO_EMC_13     13
#define PAD_GPIO_EMC_14     14
#define PAD_GPIO_EMC_15     15
#define PAD_GPIO_EMC_16     16
#define PAD_GPIO_EMC_17     17
#define PAD_GPIO_EMC_18     18
#define PAD_GPIO_EMC_19     19
#define PAD_GPIO_EMC_20     20
#define PAD_GPIO_EMC_21     21
#define PAD_GPIO_EMC_22     22
#define PAD_GPIO_EMC_23     23
#define PAD_GPIO_EMC_24     24
#define PAD_GPIO_EMC_25     25
#define PAD_GPIO_EMC_26     26
#define PAD_GPIO_EMC_27     27
#define PAD_GPIO_EMC_28     28
#define PAD_GPIO_EMC_29     29
#define PAD_GPIO_EMC_30     30
#define PAD_GPIO_EMC_31     31
#define PAD_GPIO_EMC_32     32
#define PAD_GPIO_EMC_33     33
#define PAD_GPIO_EMC_34     34
#define PAD_GPIO_EMC_35     35
#define PAD_GPIO_EMC_36     36
#define PAD_GPIO_EMC_37     37
#define PAD_GPIO_EMC_38     38
#define PAD_GPIO_EMC_39     39
#define PAD_GPIO_EMC_40     40
#define PAD_GPIO_EMC_41     41
#define PAD_GPIO_AD_B0_00     42
#define PAD_GPIO_AD_B0_01     43
#define PAD_GPIO_AD_B0_02     44
#define PAD_GPIO_AD_B0_03     45
#define PAD_GPIO_AD_B0_04     46
#define PAD_GPIO_AD_B0_05     47
#define PAD_GPIO_AD_B0_06     48
#define PAD_GPIO_AD_B0_07     49
#define PAD_GPIO_AD_B0_08     50
#define PAD_GPIO_AD_B0_09     51
#define PAD_GPIO_AD_B0_10     52
#define PAD_GPIO_AD_B0_11     53
#define PAD_GPIO_AD_B0_12     54
#define PAD_GPIO_AD_B0_13     55
#define PAD_GPIO_AD_B0_14     56
#define PAD_GPIO_AD_B0_15     57
#define PAD_GPIO_AD_B1_00     58
#define PAD_GPIO_AD_B1_01     59
#define PAD_GPIO_AD_B1_02     60
#define PAD_GPIO_AD_B1_03     61
#define PAD_GPIO_AD_B1_04     62
#define PAD_GPIO_AD_B1_05     63
#define PAD_GPIO_AD_B1_06     64
#define PAD_GPIO_AD_B1_07     65
#define PAD_GPIO_AD_B1_08     66
#define PAD_GPIO_AD_B1_09     67
#define PAD_GPIO_AD_B1_10     68
#define PAD_GPIO_AD_B1_11     69
#define PAD_GPIO_AD_B1_12     70
#define PAD_GPIO_AD_B1_13     71
#define PAD_GPIO_AD_B1_14     72
#define PAD_GPIO_AD_B1_15     73

#if defined(MCUSF_1020)

#define PAD_GPIO_SD_B0_00     74
#define PAD_GPIO_SD_B0_01     75
#define PAD_GPIO_SD_B0_02     76
#define PAD_GPIO_SD_B0_03     77
#define PAD_GPIO_SD_B0_04     78
#define PAD_GPIO_SD_B0_05     79
#define PAD_GPIO_SD_B0_06     80

#define PAD_GPIO_SD_B1_00     81   // 8158
#define PAD_GPIO_SD_B1_01     82
#define PAD_GPIO_SD_B1_02     83
#define PAD_GPIO_SD_B1_03     84
#define PAD_GPIO_SD_B1_04     85
#define PAD_GPIO_SD_B1_05     86
#define PAD_GPIO_SD_B1_06     87
#define PAD_GPIO_SD_B1_07     88
#define PAD_GPIO_SD_B1_08     89
#define PAD_GPIO_SD_B1_09     90
#define PAD_GPIO_SD_B1_10     91
#define PAD_GPIO_SD_B1_11     92   // 8184

#define PAD_COUNT  93

#elif defined(MCUSF_1050)

#define PAD_GPIO_B0_00     74
#define PAD_GPIO_B0_01     75
#define PAD_GPIO_B0_02     76
#define PAD_GPIO_B0_03     77
#define PAD_GPIO_B0_04     78
#define PAD_GPIO_B0_05     79
#define PAD_GPIO_B0_06     80
#define PAD_GPIO_B0_07     81
#define PAD_GPIO_B0_08     82
#define PAD_GPIO_B0_09     83
#define PAD_GPIO_B0_10     84
#define PAD_GPIO_B0_11     85
#define PAD_GPIO_B0_12     86
#define PAD_GPIO_B0_13     87
#define PAD_GPIO_B0_14     88
#define PAD_GPIO_B0_15     89
#define PAD_GPIO_B1_00     90
#define PAD_GPIO_B1_01     91
#define PAD_GPIO_B1_02     92
#define PAD_GPIO_B1_03     93
#define PAD_GPIO_B1_04     94
#define PAD_GPIO_B1_05     95
#define PAD_GPIO_B1_06     96
#define PAD_GPIO_B1_07     97
#define PAD_GPIO_B1_08     98
#define PAD_GPIO_B1_09     99
#define PAD_GPIO_B1_10     100
#define PAD_GPIO_B1_11     101
#define PAD_GPIO_B1_12     102
#define PAD_GPIO_B1_13     103
#define PAD_GPIO_B1_14     104
#define PAD_GPIO_B1_15     105
#define PAD_GPIO_SD_B0_00     106
#define PAD_GPIO_SD_B0_01     107
#define PAD_GPIO_SD_B0_02     108
#define PAD_GPIO_SD_B0_03     109
#define PAD_GPIO_SD_B0_04     110
#define PAD_GPIO_SD_B0_05     111
#define PAD_GPIO_SD_B1_00     112
#define PAD_GPIO_SD_B1_01     113
#define PAD_GPIO_SD_B1_02     114
#define PAD_GPIO_SD_B1_03     115
#define PAD_GPIO_SD_B1_04     116
#define PAD_GPIO_SD_B1_05     117
#define PAD_GPIO_SD_B1_06     118
#define PAD_GPIO_SD_B1_07     119
#define PAD_GPIO_SD_B1_08     120
#define PAD_GPIO_SD_B1_09     121
#define PAD_GPIO_SD_B1_10     122
#define PAD_GPIO_SD_B1_11     123

#define PAD_COUNT  124

#else

#error "Undefined MCU subfamily pad numbers"

#endif

// special pads:

#define PAD_WAKEUP            0x100
#define PAD_PMIC_ON_REQ       0x101
#define PAD_PMIC_STBY_REQ     0x102

#endif
