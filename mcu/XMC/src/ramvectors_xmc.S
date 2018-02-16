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
 *  file:     ramvectors_xmc.S
 *  brief:    By XMC1xxx the real vector table resides in ROM so this weird solution must be used
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

    .syntax unified
    .cpu cortex-m0

    .section ".text_RAMVECTORS","ax",%progbits

    .align 1

    .globl  __ramvectors
__ramvectors:

    .long 0
    .long 0
    .long 0

HardFault_Veneer:
    LDR R0, =HardFault_Handler
    MOV PC,R0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
SVC_Veneer:
    LDR R0, =SVC_Handler
    MOV PC,R0
    .long 0
    .long 0
PendSV_Veneer:
    LDR R0, =PendSV_Handler
    MOV PC,R0
SysTick_Veneer:
    LDR R0, =SysTick_Handler
    MOV PC,R0

// IRQs

    LDR R0, =IRQ_Handler_00
    MOV PC,R0
    LDR R0, =IRQ_Handler_01
    MOV PC,R0
    LDR R0, =IRQ_Handler_02
    MOV PC,R0
    LDR R0, =IRQ_Handler_03
    MOV PC,R0
    LDR R0, =IRQ_Handler_04
    MOV PC,R0
    LDR R0, =IRQ_Handler_05
    MOV PC,R0
    LDR R0, =IRQ_Handler_06
    MOV PC,R0
    LDR R0, =IRQ_Handler_07
    MOV PC,R0
    LDR R0, =IRQ_Handler_08
    MOV PC,R0
    LDR R0, =IRQ_Handler_09
    MOV PC,R0

    LDR R0, =IRQ_Handler_10
    MOV PC,R0
    LDR R0, =IRQ_Handler_11
    MOV PC,R0
    LDR R0, =IRQ_Handler_12
    MOV PC,R0
    LDR R0, =IRQ_Handler_13
    MOV PC,R0
    LDR R0, =IRQ_Handler_14
    MOV PC,R0
    LDR R0, =IRQ_Handler_15
    MOV PC,R0
    LDR R0, =IRQ_Handler_16
    MOV PC,R0
    LDR R0, =IRQ_Handler_17
    MOV PC,R0
    LDR R0, =IRQ_Handler_18
    MOV PC,R0
    LDR R0, =IRQ_Handler_19
    MOV PC,R0

    LDR R0, =IRQ_Handler_20
    MOV PC,R0
    LDR R0, =IRQ_Handler_21
    MOV PC,R0
    LDR R0, =IRQ_Handler_22
    MOV PC,R0
    LDR R0, =IRQ_Handler_23
    MOV PC,R0
    LDR R0, =IRQ_Handler_24
    MOV PC,R0
    LDR R0, =IRQ_Handler_25
    MOV PC,R0
    LDR R0, =IRQ_Handler_26
    MOV PC,R0
    LDR R0, =IRQ_Handler_27
    MOV PC,R0
    LDR R0, =IRQ_Handler_28
    MOV PC,R0
    LDR R0, =IRQ_Handler_29
    MOV PC,R0

    LDR R0, =IRQ_Handler_30
    MOV PC,R0
    LDR R0, =IRQ_Handler_31
    MOV PC,R0
    LDR R0, =IRQ_Handler_32
    MOV PC,R0
    LDR R0, =IRQ_Handler_33
    MOV PC,R0
    LDR R0, =IRQ_Handler_34
    MOV PC,R0
    LDR R0, =IRQ_Handler_35
    MOV PC,R0
    LDR R0, =IRQ_Handler_36
    MOV PC,R0
    LDR R0, =IRQ_Handler_37
    MOV PC,R0
    LDR R0, =IRQ_Handler_38
    MOV PC,R0
    LDR R0, =IRQ_Handler_39
    MOV PC,R0

	.pool

    .end
