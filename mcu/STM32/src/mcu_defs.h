// mcu_defs.h

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_F0)

  #define MAX_CLOCK_SPEED  48000000

#elif	defined(MCUSF_L0)

  #define MAX_CLOCK_SPEED  32000000

#elif	defined(MCUSF_F1)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  72000000
  #endif

#elif	defined(MCUSF_F3)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  72000000
  #endif

#elif	defined(MCUSF_F4)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  180000000
  #endif

#elif	defined(MCUSF_F7)

    #define MAX_CLOCK_SPEED  216000000

#endif

#define HW_GPIO_REGS  GPIO_TypeDef
#define HW_UART_REGS  USART_TypeDef
#define HW_SPI_REGS   SPI_TypeDef

#if defined(MCUSF_F1) || defined(MCUSF_F0) || defined(MCUSF_L0) || defined(MCUSF_F3)
  #define HW_DMA_REGS 	DMA_Channel_TypeDef
#else
  #define HW_DMA_REGS   DMA_Stream_TypeDef
#endif

#if __CORTEX_M < 3
  #if defined(TIM14)
    #define CLOCKCNT       (TIM14->CNT)      // use the worst timer for clock counting
  #else
    #define CLOCKCNT       (TIM21->CNT)
  #endif
  #define CLOCKCNT_BITS  16
#endif

inline void __attribute__((always_inline)) mcu_preinit_code()
{
}

#endif // __MCU_DEFS_H
