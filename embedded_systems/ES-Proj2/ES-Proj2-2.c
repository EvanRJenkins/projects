#include <msp430.h>
#define LED1 BIT0
#define DELAYLOOPS 10000

#define STATE_OFF 0
#define STATE_ON 1

void main(void) {
  volatile unsigned int
      LoopCounter; // "volatile" ensures will not be optimized away
  unsigned int currentState = STATE_OFF;
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
                            // Set
  P1DIR = LED1;  // Set P1.0 (red LED) to output and P1.3 (button) to input
  P1OUT = ~LED1; // start with LED1 off, LED2 on
  P1REN |= BIT3; // enable input resistor
  P1OUT |= BIT3; // set pull-up resistor
  while (1)      // loop forever
  {

    switch (currentState) // Finite state machine
    {

    case STATE_OFF:
      if (!(P1IN & BIT3)) // If button rising edge
      {
        for (LoopCounter = 0; LoopCounter < DELAYLOOPS;
             ++LoopCounter) // Wait for DELAY_LOOPS cycles
        {
          ;
        }
        P1OUT |= BIT0;           // SET LED
        currentState = STATE_ON; // Go to on
      } else {
        for (LoopCounter = 0; LoopCounter < DELAYLOOPS / 2;
             ++LoopCounter) // Wait for DELAY_LOOPS cycles
        {
          ;
        }
        P1OUT ^= BIT0; // Toggle LED
      }
      break;

    case STATE_ON:
      if (P1IN & BIT3) // If button falling edge
      {
        P1OUT &= ~BIT0;           // Reset LED
        currentState = STATE_OFF; // Go to off state
      } else {
        currentState = STATE_ON; // Else stay
      }
      break;
    }
  }
}
