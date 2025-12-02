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
  Enable P2.1 interrupt
  */
  __bis_SR_register(GIE);
  P2IES |= BIT1;  // Set edge select to high-low
  P2IFG &= ~BIT1;  // Clear flag
  P2IE |= BIT1;  // Enable interrupt for P2.1
  /*
  FSM loop
  */
  while (1) {
    switch (system_state) {
      case IDLE:  // LPM and wait
        break;
      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        SIPO_shift(0x02);  // Testing interrupt
        break;
      case MENU:  // Configure RANDOM number range, mode (COUNT or RANDOM, etc.)
       break;
      case COUNT:  // Increment counter and display for some time
        break;
      case RANDOM:  // Make "random" number in set range and display for some time
        break;
   }
  }
  return 0;  // End of program
}
/*
Interrupts
*/

#pragma vector = PORT2_VECTOR
__interrupt void Port_2_ISR(void) {
  if (P2IFG & BIT1) {  // If falling edge interrupt
    if (P2IES & BIT1) {
        // Switch this to start debounce timer
      P2IES &= ~BIT1;  // Switch flag trigger to rising edge
    }
    else {  // If rising edge interrupt
      system_state = COMMAND;  // Go to COMMAND state
      P2IES |= BIT1;  // Switch trigger back
    }
    P2IFG &= ~BIT1;
  }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {  // For button debounce
  

}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {  // For features
  

}

