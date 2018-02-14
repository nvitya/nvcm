
#include "platform.h"

#if !defined(CUSTOM_FLASH_PROTECTION)

//*****************************************************************************
// Flash Configuration block : 16-byte flash configuration field that stores
// default protection settings (loaded on reset) and security information that
// allows the MCU to restrict access to the Flash Memory module.
// Placed at address 0x400 by the linker script.
//*****************************************************************************

__attribute__ ((section(".flash_specials"),used))
unsigned char kinetis_flash_protection[16] =
{
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // backdoor comparison key
		0xFF, 0xFF, 0xFF, 0xFF, // program flash protection bytes

		// flash security byte (FSEC)
		0x82,  // backdoor key enabled, unsecure status

		// FOPT register, very important to set
#if defined(MCUSF_KL03)
		(0
			| (0 << 6) // 00 = Boot from flash
			| (1 << 5) // 1 = Fast initialization
			| (1 << 3) // 1 = Dedicated reset pin
			| (0 << 2) // 0 = NMI disabled
			| (1 << 1) // 1 = boot source configured here
			| (1 << 4) | (1 << 0)  // RUN Mode: fast clock selected
		),
#elif defined(MCUSF_KV30)
		(0
			| (0 << 6)    // (00 = reserved)
			| (1 << 5) // 1 = Fast initialization
			| (0 << 3)    // (00 = reserved)
			| (0 << 2) // 0 = NMI disabled
			| (0 << 1)    // (0 = reserved)
			| (1 << 0) // 1 = normal boot, 0 = low power boot
		),
#elif defined(MCUSF_K20)
		(0
			| (0 << 2) // 0 = NMI disabled
			| (1 << 1) // 1 = EZport disabled
			| (1 << 0) // LPBOOT: 1 = Normal boot, 0 = low power boot
		),
#else
      #error "Define FOPT contents for this KINETIS MCU"
#endif
		0xFF,  // EEPROM protection byte (FEPROT)
		0xFF   // Data flash protection byte (FDPROT)
};

#endif
