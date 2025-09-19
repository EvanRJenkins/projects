#include <msp430g2553.h>

#define RED_DELAY 20000

void main(void) {
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

  P1DIR |= (BIT0 | BIT6); // Set LEDs' directions to output
  P1OUT &= ~(BIT0 | BIT6);

  P1DIR &= ~BIT3; // Set P1.3 direction to input
  P1REN |= BIT3;  // Enable resistor on button pin
  P1OUT |= BIT3;  // Configure resistor to pull-up
  P1IE |= BIT3;   // Enable button interrupt
  P1IES |= BIT3;  // Interrupt on falling edge
  P1IFG &= ~BIT3; // Clear button interrupt flag

  TACTL = TASSEL_2 + TACLR; // Use SMCLK, cleared, paused
  TACCR0 = 30000;           // Set debounce delay period (about 30 ms)
  TACCTL0 = CCIE;           // Enable CCR0 interrupt

  __enable_interrupt(); // Enable interrupts globally

  while (1) {
    volatile long i;

    P1OUT ^= BIT0; // Toggle red LED

    for (i = RED_DELAY; i > 0; --i) // Delay
      ;
  }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
  if ((TACTL & MC_1) == 0) {
    P1IE &= ~BIT3; // Disable interrupt
    TACTL |= MC_1; // Start in up mode
  }
  P1IFG &= ~BIT3; // Clear button flag
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
  TACTL &= ~MC_3; // Pause

  if ((P1IN & BIT3) == 0) {
    P1OUT ^= BIT6; // Toggle green LED
  }

  P1IFG &= ~BIT3; // Clear flag
  P1IE |= BIT3;   // Re-enable interrupt
}
