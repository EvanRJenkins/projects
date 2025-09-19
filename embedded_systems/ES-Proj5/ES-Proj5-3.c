#include <msp430g2553.h>

volatile unsigned char current_program = 1;
volatile unsigned int current_program_time =
    40; // 40 * 250 ms = approx. 10 seconds

void blink_indicator(unsigned char count);

void main(void) {
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

  BCSCTL3 |= LFXT1S_2; // ACLK = VLO

  P1DIR |= (BIT0 | BIT6); // Set LEDs to output direction
  P1OUT &= ~(BIT0 | BIT6);

  WDTCTL = WDT_ADLY_250; // Set WDT interval to ~250ms from ACLK
  IE1 |= WDTIE;          // Enable WDT interrupt

  __enable_interrupt(); // Enable global interrupts

  while (1) {
    switch (current_program) {
    case 1:
      blink_indicator(1);
      TACTL = TASSEL_2 + TACLR; // Pause timer A
      while (current_program == 1) {
        P1OUT ^= BIT6;
        __delay_cycles(500000); // Blink delay active
      }
      break;

    case 2:
      blink_indicator(2);
      // Configure Timer_A with SMCLK (1 MHz) and /8 divider
      TACTL =
          TASSEL_2 + MC_1 + TACLR + ID_3; // SMCLK, Up Mode, Clear, /8 divider
      TACCR0 = 62500;                     // 500ms period for blinking
      TACCTL0 = CCIE;                     // Enable CCR0 interrupt
      // Enter LPM0
      break;

    case 3:
      blink_indicator(3);
      // Configure Timer_A with ACLK (12 kHz)
      TACTL = TASSEL_1 + MC_1 + TACLR;    // ACLK, Up Mode, Clear
      TACCR0 = 6000;                      // 500ms period for blinking
      TACCTL0 = CCIE;                     // Enable CCR0 interrupt
      // Enter LPM3
      break;
    }
  }
}

void blink_indicator(unsigned char count) {
  unsigned char i;
  for (i = 0; i < count; i++) {
    P1OUT |= BIT0;
    __delay_cycles(100000); // 100 ms on delay
    P1OUT &= ~BIT0;
    __delay_cycles(100000); // 100 ms off delay
  }
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void) {
  current_program_time--;
  if (current_program_time == 0) {
    current_program_time = 40; // Reset
    current_program++;
    if (current_program > 3) {
      current_program = 1;
    }
    // Wake up CPU
  }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
  P1OUT ^= BIT6; // Toggle green LED
}
