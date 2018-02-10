// boards_builtin.h

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// STM32
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_MIN_F103)

  #define BOARD_NAME "STM32F103C8 Minimum Develompent Board"
  #define MCU_STM32F103C8
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_DISCOVERY_F072)

  #define BOARD_NAME "STM32F072 Discovery"
  #define MCU_STM32F072RB

#elif defined(BOARD_DEV_STM32F407VG)

  #define BOARD_NAME "STM32F407VG Minimal Board"
  #define MCU_STM32F407VG
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_DEV_STM32F407ZE)

  #define BOARD_NAME "STM32F407ZE Development Board"
  #define MCU_STM32F407ZE
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_NUCLEO_F446)

  #define BOARD_NAME "STM32F446 Nucleo-144"
  #define MCU_STM32F446ZE
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_NUCLEO_F746)

  #define BOARD_NAME "STM32F746 Nucleo-144"
  #define MCU_STM32F746ZG
  #define MCU_INPUT_FREQ   8000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
