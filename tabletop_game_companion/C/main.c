#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Constant definitions
*/
#define RANDOM_INTERVAL 100  // Placeholder value
#define DEBOUCE_COUNT 1000  // Placeholder value
#define TAIV_2 2  // TACCR1 CCIFG
unsigned char debounce_highlow_done = 0;// TEMP FLAG
unsigned char shift_done = 1;// TEMP FLAG
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
Global variable instantiations
*/
volatile state_t system_state = IDLE;
volatile unsigned char rng_num = 0x00;
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
  Init Timer_A
  */
  TACTL = TASSEL_2 + MC_2;  // SMCLK, continuous mode
  //TACCR1 = TAR + RANDOM_INTERVAL;  // RNG reference point
  //TACCTL1 = CCIE;  // Enable Timer_A interrupt
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
        if (shift_done == 1)
        {
          SIPO_shift(0x02);  // Testing interrupt
          shift_done = 0;
        }
        break;
      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        if (shift_done == 0)
        {
          SIPO_shift(rng_num);  // Testing interrupt
          shift_done = 1;
        }
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
    if (P2IES & ~BIT1) {  // If rising transition
    rng_num = TA0R;  // Save random number
    system_state = COMMAND;
    }
    P2IE &= ~BIT1;  // Disable button interrupt
    P2IES ^= BIT1;  // Switch relevant edge
    TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
    TACCTL0 = CCIE;  // Enable Timer_A interrupt
    P2IFG &= ~BIT1;  // Clear any accumulated button flags
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {  // For button debounce (highest priority)
  TACCTL0 &= ~CCIE;  // Disable Timer_A interrupt
  P2IFG &= ~BIT1;  // Clear button interrupt flag
  P2IE |= BIT1;  // Enable button interrupt
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {  // For features
  if (TAIV == TAIV_2) {  // Define 2!
    // Progress random number algorithm
  }
}
