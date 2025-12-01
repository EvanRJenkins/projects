#include <msp430g2452.h>
#include "74AHC164.h"


/*
Main function
*/
int main(void) {
  /*
  Init pins
  */
  P1DIR |= 0x0F;  // Set bits 0-3 as outputs
  P1OUT |= BIT3;  // Set LD_N high
  P1OUT &= ~(BIT0 | BIT1 | BIT2);  // Set others low

  SIPO_reg_init(&P1OUT, &P1OUT, &P1OUT);
  SIPO_pin_init(BIT0, BIT2, BIT1);



}
