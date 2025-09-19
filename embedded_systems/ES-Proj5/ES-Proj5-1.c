#include <msp430g2553.h>

const unsigned int current_speed[10] = {60000, 55000, 45000, 35000, 25000,
                                        15000, 8000,  4000,  2000,  1000};

volatile unsigned int speed_index = 0;
volatile unsigned int toggle_count = 0;

#define RED_DELAY 20000
#define NUM_TOGGLES 40
#define OVERFLOW_FLAG 0x0A

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

  P1DIR |= (BIT0 | BIT6);  // Set LEDs to output direction
  P1OUT &= ~(BIT0 | BIT6);

  TACTL = TASSEL_2 + MC_1 + TACLR + TAIE;  // SMCLK, Up , clear TAR, enable interrupt

  TACCR0 = current_speed[0];  // Init green LED toggle speed

  __enable_interrupt();  // Enable global interrupts

  while (1) {
    volatile long i;

    P1OUT ^= BIT0;  // Toggle red LED

    for (i = RED_DELAY; i > 0; --i)  // Delay
      ;
  }
}

#pragma vector = TIMER0_A1_VECTOR

__interrupt void TIMER_ISR(void) {
  if (TAIV == OVERFLOW_FLAG) {
    P1OUT ^= BIT6;  // Toggle green LED

    toggle_count++;

    if (toggle_count >= NUM_TOGGLES) {
      toggle_count = 0;

      speed_index++;

      if (speed_index >= 10) {
        speed_index = 0;
      }

      TACCR0 =
          current_speed[speed_index];  // Update the timer's period register
    }
  }
}
