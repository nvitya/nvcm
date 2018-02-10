// mcu_defs.h / IMXRT

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_1050)

  // The industrial versions can work only up to 528 MHz
  // the 600 MHz is a kind of overclocking, which requires higher core voltage
  //#define MAX_CLOCK_SPEED  600000000

  #define MAX_CLOCK_SPEED  528000000

#endif

#define HW_GPIO_REGS      GPIO_Type
#define HW_UART_REGS      LPUART_Type
#define HW_SPI_REGS       LPSPI_Type
#define HW_QSPI_REGS			FLEXSPI_Type

inline void __attribute__((always_inline)) mcu_preinit_code()
{
  // Disable Watchdog
  if (WDOG1->WCR & WDOG_WCR_WDE_MASK)
  {
      WDOG1->WCR &= ~WDOG_WCR_WDE_MASK;
  }
  if (WDOG2->WCR & WDOG_WCR_WDE_MASK)
  {
      WDOG2->WCR &= ~WDOG_WCR_WDE_MASK;
  }
  RTWDOG->CNT = 0xD928C520U; // 0xD928C520U is the update key
  RTWDOG->TOVAL = 0xFFFF;
  RTWDOG->CS = (uint32_t) ((RTWDOG->CS) & ~RTWDOG_CS_EN_MASK) | RTWDOG_CS_UPDATE_MASK;
}

#endif // __MCU_DEFS_H
