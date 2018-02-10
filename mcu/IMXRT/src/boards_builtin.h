// boards_builtin.h / IMXRT

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// IMXRT
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_EVK_IMXRT1050)

  #define BOARD_NAME "IMXRT1050-EVK"
  #define MCU_IMXRT1052
  #define MCU_INPUT_FREQ   24000000

#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
