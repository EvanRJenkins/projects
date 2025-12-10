#include <msp430g2452.h>
#include "init.h"

void MSP430G2452_init() {
  WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
  /*
  Init clock
  */
  BCSCTL3 |= LFXT1S_2; // Set LFXT1 to VLO mode (Must be first)
  IFG1 &= ~OFIFG; // Clear Oscillator Fault Flag to allow clock switch
  __delay_cycles(100); // Short delay to let VLO stabilize
  BCSCTL2 |= SELM_3 + DIVM_0 + SELS + DIVS_1; // Switch MCLK/SMCLK to VLO /0 and VLO /2
  __delay_cycles(500); // Short delay to let VLO stabilize
  __bis_SR_register(SCG0 + SCG1);
  /*
  Init Timer_A
  */
  TACTL |= TACLR;  // Clear Timer_A
  TACTL = TASSEL_1 + MC_2 + ID_2;  // Set Timer_A clock to ACLK (VLO /4)
  TACTL |= TACLR;  // Clear Timer_A
  /*
  Init pins
  */
  P1DIR |= (BIT0 | BIT1 | BIT2 | BIT3);  // Set P1.0-P1.3 direction to output
  P1OUT |= (BIT2 | BIT3);  // Set MR_N and LD_N high
  P1OUT &= ~(BIT0 | BIT1);  // Set others low
  P2DIR &= ~(BIT1 | BIT2);  // Set P2.1 and P2.2 direction to input
  P2REN |= (BIT1 | BIT2);  // Enable resistor for P2.1 and P2.2
  P2REN &= ~BIT3;  // Disable resistor for P2.3
  P2OUT |= (BIT1 | BIT2);  // Set P2.1 and P2.2 resistors to pull-up mode
  P2DIR |= BIT3;  // Set P2.3 to output for measuring timing
  P2OUT &= ~BIT3;  // Start P2.3 low to monitor low-high transition
  P2REN &= ~BIT3;  // Disable resistor for P2.3
}