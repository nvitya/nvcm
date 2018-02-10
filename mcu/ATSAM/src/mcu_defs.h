// mcu_defs.h / ATSAM

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

// SUB-FAMILIES
#if defined(MCUSF_3X)

  #define MAX_CLOCK_SPEED   84000000
  #define HW_HAS_PDMA

#elif defined(MCUSF_E70)

  #define MAX_CLOCK_SPEED  300000000

#elif defined(MCUSF_4S)

  #define MAX_CLOCK_SPEED  120000000
  #define HW_HAS_PDMA

#endif

#define HW_GPIO_REGS      Pio
#define HW_UART_REGS      Uart
#define HW_UART_ALT_REGS  Usart
#define HW_SPI_REGS       Spi
#define HW_QSPI_REGS      Qspi

#ifdef DMACCH_NUM_NUMBER
  #define HW_DMA_REGS  DmacCh_num
#elif defined(XDMAC)
  #define HW_DMA_REGS  XdmacChid
#else
  #define HW_DMA_REGS  void  // the older versions don't have central DMA
#endif

#ifdef HW_HAS_PDMA

  // DMA channel coupled to the peripheral, in documentation referred as PDC: Peripheral DMA Controller
  // (older Atmel chips)
  typedef struct HW_PDMA_REGS
  {
    __IO uint32_t RPR;      /**< \brief (Offset: 0x100) Receive Pointer Register */
    __IO uint32_t RCR;      /**< \brief (Offset: 0x104) Receive Counter Register */
    __IO uint32_t TPR;      /**< \brief (Offset: 0x108) Transmit Pointer Register */
    __IO uint32_t TCR;      /**< \brief (Offset: 0x10C) Transmit Counter Register */
    __IO uint32_t RNPR;     /**< \brief (Offset: 0x110) Receive Next Pointer Register */
    __IO uint32_t RNCR;     /**< \brief (Offset: 0x114) Receive Next Counter Register */
    __IO uint32_t TNPR;     /**< \brief (Offset: 0x118) Transmit Next Pointer Register */
    __IO uint32_t TNCR;     /**< \brief (Offset: 0x11C) Transmit Next Counter Register */
    __O  uint32_t PTCR;     /**< \brief (Offset: 0x120) Transfer Control Register */
    __I  uint32_t PTSR;     /**< \brief (Offset: 0x124) Transfer Status Register */
  //
  } HW_PDMA_REGS;

  #define HW_DMA_ALT_REGS  HW_PDMA_REGS

#endif

inline void __attribute__((always_inline)) mcu_preinit_code()
{
  // some Atmel processors start with watchdog enabled
  WDT->WDT_MR |= WDT_MR_WDDIS;  // Turn off the WDT, can not be turned back on until reset

  #ifdef MCUSF_E70
    // configure memory

    //   0k ITCM +   0k DTCM = 00
    //  32k ITCM +  32k DTCM = 01
    //  64k ITCM +  64k DTCM = 10
    // 128k ITCM + 128k DTCM = 11
    // 128 + 128:
    EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_SGPB | EEFC_FCR_FARG(7));  // set bit 7
    EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_SGPB | EEFC_FCR_FARG(8));  // set bit 8

    // enable the TCM in the core
    __DSB();
    __ISB();
    SCB->ITCMCR = (SCB_ITCMCR_EN_Msk | SCB_ITCMCR_RMW_Msk | SCB_ITCMCR_RETEN_Msk);
    SCB->DTCMCR = (SCB_DTCMCR_EN_Msk | SCB_DTCMCR_RMW_Msk	| SCB_DTCMCR_RETEN_Msk);
    __DSB();
    __ISB();

  #endif
}

#endif // __MCU_DEFS_H
