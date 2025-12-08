#include <msp430g2452.h>
#include "init.h"

void MSP430G2452_init() {
  WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
  /*
  Init clock
  */
  BCSCTL3 |= LFXT1S_2; // Set LFXT1 to VLO mode (Must be first)
  IFG1 &= ~OFIFG; // Clear Oscillator Fault Flag to allow clock switch
  __delay_cycles(50); // Short delay to let VLO stabilize
  BCSCTL2 |= SELM_3 + DIVM_1 + SELS + DIVS_1; // Switch MCLK/SMCLK to VLO /8
  __delay_cycles(500); // Short delay to let VLO stabilize
  __bis_SR_register(SCG0 + SCG1);
  /*
  Init Timer_A
  */
  TACTL |= (TASSEL_1 | ID_0);  // Set Timer_A clock to ACLK (VLO /1)
  TACTL &= ~(MC_0);  // Set Timer_A to stop mode
  TACTL |= TACLR;  // Clear Timer_A
  /*
  Init pins
  */
  P1DIR |= (BIT0 | BIT1 | BIT2 | BIT3);  // Set P1.0-P1.3 direction to output
  P1OUT |= (BIT2 | BIT3);  // Set MR_N and LD_N high
  P1OUT &= ~(BIT0 | BIT1);  // Set others low
  P2DIR &= ~(BIT1 | BIT2| BIT3 | BIT4);  // Set P2.3 and P2.4 direction to input
  P2REN |= (BIT1 | BIT2| BIT3 | BIT4);  // Enable resistor for P2.3 and P2.4
  P2OUT |= (BIT1 | BIT2| BIT3 | BIT4);  // Set P2.3 and P2.4 resistors to pull-up mode
}