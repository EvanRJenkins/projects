#include <msp430g2452.h>
#include "74AHC164.h"

/*
Pin and register definitions
for use with 74AHC164.h
*/
#define DSA_PIN = 00000001;
#define CP_PIN = 00000010;
#define MR_N_PIN 00000100;

unsigned char *pDSA_REG = &P1OUT;
unsigned char *pMR_N_REG = &P1OUT;
unsigned char *pCP_REG = &P1OUT;






int main(void) {
  /*
  Init pins
  */
  P1DIR |= 0x0F;  // Set bits 0-3 as outputs
  P1OUT |= BIT3;  // Set LD_N high
  P1OUT &= ~(BIT0 | BIT1 | BIT2);  // Set others low






}
