// mcu_defs.h

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_43XX)

  #define MAX_CLOCK_SPEED  204000000

#endif

#define HW_DMA_MAX_COUNT  2048

#define INDEPENDENT_GPIO
#define HW_GPIO_REGS  LPC_GPIO_T

#define HW_UART_REGS  LPC_USART_T
#define HW_SPI_REGS   LPC_SSP_T
#define HW_DMA_REGS 	GPDMA_CH_T

// The register definition of the SPIFI is missing from the official LPC43xx header...

typedef struct SPIFI_REGS_T
{
	volatile    uint32_t CTRL;				/**< SPIFI control register */
	volatile    uint32_t CMD;					/**< SPIFI command register */
	volatile    uint32_t ADDR;				/**< SPIFI address register */
	volatile    uint32_t IDATA;				/**< SPIFI intermediate data register */
	volatile    uint32_t CLIMIT;			/**< SPIFI cache limit register */
	union
	{
		volatile    uint8_t  DATA8;			/**< SPIFI 8 bit data */
		volatile    uint16_t DATA16;		/**< SPIFI 16 bit data */
		volatile    uint32_t DATA;			/**< SPIFI 32 bit data */
	};
	volatile    uint32_t MCMD;				/**< SPIFI memory command register */
	volatile    uint32_t STAT;				/**< SPIFI status register */
//
} SPIFI_REGS_T;

#define HW_QSPI_REGS  SPIFI_REGS_T

inline void __attribute__((always_inline)) mcu_preinit_code()
{
}

#endif // __MCU_DEFS_H
