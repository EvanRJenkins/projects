#include <msp430g2452.h>
#include "74AHC164.h"
/*
FSM type definition
*/
typedef enum {
  IDLE;
  COMMAND;
  MENU;
  COUNT;
  RANDOM;
} state_t;
/*
State variable instantiation
*/
volatile state_t system_state = IDLE;
/*
Main function
*/
int main(void) {
  WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
  /*
  Init clock
  */
  BCSCTL2 |= SELM_3;  // Select VLO as MCLK source
  BCSCTL2 |= SELS;  // Selec VLO as SMCLK source
  BCSCTL3 = LFXT1S_2;  // Selec VLO as ACLK source
  /*
  Init Timer_A
  */
  TACTL |= TASSEL_3;  // Set Timer_A clock to SMCLK (VLO)
  TACTL &= ~(MC_0);  // Set Timer_A to stop mode
  TACTL |= TACLR;  // Clear Timer_A
  /*
  Init pins
  */
  P1DIR |= (BIT0 | BIT1 | BIT2 | BIT3);  // Set P1.0-P1.3 direction to output
  P1OUT |= (BIT2 | BIT3);  // Set MR_N and LD_N high
  P1OUT &= ~(BIT0 | BIT1);  // Set others low
  P2DIR &= ~(BIT3 | BIT4);  // Set P2.3 and P2.4 direction to input
  P2REN |= (BIT3 | BIT4);  // Enable resistor for P2.3 and P2.4
  P2OUT |= (BIT3 | BIT4);  // Set P2.3 and P2.4 resistors to pull-up mode
  /*
  Init SIPO
  */
  SIPO_reg_init(&P1OUT, &P1OUT, &P1OUT);
  SIPO_pin_init(BIT0, BIT2, BIT1);
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
