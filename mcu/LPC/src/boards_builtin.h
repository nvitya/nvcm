// boards_builtin.h / LPC

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if defined(BOARD_XPRESSO_LPC4337)

  #define BOARD_NAME "LPCXpresso4337"
  #define MCU_LPC4337
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_XPLORER_LPC4330)

  #define BOARD_NAME "LPC4330-Xplorer"
  #define MCU_LPC4330
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif

#endif /* BOARDS_BUILTIN_H_ */
