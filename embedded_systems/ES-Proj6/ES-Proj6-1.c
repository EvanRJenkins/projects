#include <msp430g2553.h>

unsigned int pwm_set_duration(unsigned char duration_percent);
unsigned int pwm_set_duty(unsigned char duty_percent);

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set the DCO to 1MHz
    BCSCTL2 = ~SELS;  // Set SMCLK to DCO
    DCOCTL = CALDCO_1MHZ;    // Set DCO to minimum frequency

    P1DIR |= BIT6;  // Init P1.6 to output
    P1OUT &= ~BIT6;  // Start with the pin LOW

    TACCR0 = 20000;  // Initialize CCR0 (Duration)

    TACCR1 = 10000;  // Initialize CCR1 (Duty cycle)

    // Enable capture interrupts
    TACCTL0 = CCIE;
    TACCTL1 = CCIE;

    // Use SMCLK (VLO), up mode, clear timer
    TACTL = TASSEL_2 | MC_1 | TACLR;

    TACCR0 = pwm_set_duration(20);

    TACCR1 = (TACCR0 / 100) * 50;  // Set duty proportional to duration

    __bis_SR_register(LPM0_bits | GIE);  // LPM0 and enable interrupts
}

unsigned int pwm_set_duration(unsigned char duration_percent)
{
    if (duration_percent >= 100)
    {
        return 65535;
    }

    return ((65535 / 100) * duration_percent);

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
