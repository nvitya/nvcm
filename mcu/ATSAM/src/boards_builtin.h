// boards_builtin.h

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// ATMEL
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_ARDUINO_DUE)

  #define BOARD_NAME "Arduino DUE"
  #define MCU_ATSAM3X8E
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_XPLAINED_SAME70)

  #define BOARD_NAME "SAME70 XPlained"
  #define MCU_ATSAME70Q21
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_MIBO100_ATSAME70)

  #define BOARD_NAME "MIBO-100 ATSAME70N by nvitya"
	#define MCU_ATSAME70N20
  #define MCU_INPUT_FREQ   12000000

#elif defined(BOARD_MIBO64_ATSAM4S)

  #define BOARD_NAME "MIBO-64 ATSAM4S by nvitya"
  #define MCU_ATSAM4S2B
  #define MCU_INPUT_FREQ   12000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
