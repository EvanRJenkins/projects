#include <msp430g2553.h>

pwm_set(unsigned char duration_ms, unsigned char duty_cycle_percent);

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set the DCO to 1MHz
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    //BCSCTL2 = DIVM_3;

    P1DIR |= BIT6;  // Init P1.6 to output
    P1OUT &= ~BIT6;  // Start with the pin LOW

    TACCR0 = 20000;  // Initialize CCR0 (Duration)

    TACCR1 = 10000;  // Initialize CCR1 (Duty cycle)

    // Enable capture interrupts
    TACCTL0 = CCIE;
    TACCTL1 = CCIE;

    // Use SMCLK (1 MHz DCO), up mode, clear timer
    TACTL = TASSEL_2 | MC_1 | TACLR;


    __bis_SR_register(LPM0_bits | GIE);  // LPM0 and enable interrupts
}

pwm_set(unsigned char duration_ms, unsigned char duty_cycle_percent)
{
  unsigned int duration_counts = duration_ms * 1000;
  unsigned int percent_counts = duration_ms * duty_cycle_percent / 100;

  TACCR0 = duration_counts;  // Initialize CCR0 (Duration)

  TACCR1 = percent_counts;  // Initialize CCR1 (Duty cycle)
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    P1OUT |= BIT6;  // Set the PWM pin HIGH
}


#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{

    if (TAIV == 2)
  {
            P1OUT &= ~BIT6;          // Set the PWM pin LOW
  }


}
