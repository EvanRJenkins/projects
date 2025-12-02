#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
FSM type definition
*/
typedef enum {
  IDLE,
  COMMAND,
  MENU,
  COUNT,
  RANDOM
} state_t;
/*
State variable instantiation
*/
volatile state_t system_state = IDLE;
/*
Main function
*/
int main(void) {
  MSP430G2452_init();
  /*
  Init SIPO
  */
  SIPO_reg_init(&P1OUT, &P1OUT, &P1OUT);
  SIPO_pin_init(BIT0, BIT2, BIT1);
  SIPO_shift(0x14);
  /*
  FSM loop
  */
  switch (system_state) {
    case IDLE:  // LPM and wait
      break;
    case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
      break;
    case MENU:  // Configure RANDOM number range, mode (COUNT or RANDOM, etc.)
      break;
    case COUNT:  // Increment counter and display for some time
      break;
    case RANDOM:  // Make "random" number in set range and display for some time
      break;
  }
  return 0;  // End of program
}
// End of file
