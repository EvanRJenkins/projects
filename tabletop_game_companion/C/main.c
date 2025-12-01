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
  /*
  Init Timer_A
  */
  TACTL |= TASSEL_3;  // Set Timer_A clock to SMCLK (VLO)
  TACTL &= ~(MC_0);  // Set Timer_A to stop mode
  TACTL |= TACLR;  // Clear Timer_A
  /*
  Init pins
  */
  P1DIR |= (BIT0 | BIT1 | BIT2 | BIT3);  // Set bits 0-3 as outputs
  P1OUT |= (BIT2 | BIT3);  // Set MR_N and LD_N high
  P1OUT &= ~(BIT0 | BIT1);  // Set others low
  /*
  Init SIPO
  */
  SIPO_reg_init(&P1OUT, &P1OUT, &P1OUT);
  SIPO_pin_init(BIT0, BIT2, BIT1);
}
