#include <msp430g2553.h>

unsigned int pwm_set_duration(unsigned char duration_percent);
#define DURATION_MS 4
#define DUTY_PERCENT_SLOW 25
#define DUTY_PERCENT_FAST 75

#define DURATION_PERCENT DURATION_MS * 2  // At 1 MHz, abt 1 ms pulse

typedef enum { SLOW, FAST } speed_t;

volatile speed_t current_speed = SLOW;

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set the DCO to 1MHz
    BCSCTL2 &= ~SELS;  // Set SMCLK to DCO
    DCOCTL = CALDCO_1MHZ;    // Set DCO to 1 MHz

    P1DIR &= ~BIT3; // Set P1.3 direction to input
    P1REN |= BIT3;  // Enable resistor on button pin
    P1OUT |= BIT3;  // Configure resistor to pull-up
    P1IE |= BIT3;   // Enable button interrupt
    P1IES |= BIT3;  // Interrupt on falling edge
    P1IFG &= ~BIT3; // Clear button interrupt flag

    P1DIR |= BIT6;  // Init P1.6 to output
    P1OUT &= ~BIT6;  // Start with the pin LOW

    TACCR0 = 20000;  // Initialize CCR0 (Duration)

    TACCR1 = 10000;  // Initialize CCR1 (Duty cycle)

    // Enable capture interrupts
    TACCTL0 = CCIE;
    TACCTL1 = CCIE;

    // Use SMCLK (VLO), up mode, clear timer
    TACTL = TASSEL_2 | MC_1 | TACLR;

    __bis_SR_register(GIE);  // Enable interrupts


    while (1)
    {
        switch (current_speed)
        {
            case SLOW:
                TACCR0 = pwm_set_duration(DURATION_PERCENT);  // About 4 ms duration
                TACCR1 = (TACCR0 / 100) * DUTY_PERCENT_SLOW;  // Set duty proportional to duration
                break;
            
            case FAST:
                TACCR0 = pwm_set_duration(DURATION_PERCENT);  // About 4 ms duration
                TACCR1 = (TACCR0 / 100) * DUTY_PERCENT_FAST;  // Set duty proportional to duration
                break;

            default:
                current_speed = SLOW;
                break;
        }
    }
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

#pragma vector = PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
    if ((P1IES & BIT3) == 0) {   // Check if triggered by rising edge
        current_speed = !current_speed; // Toggle speed
    }

    P1IES ^= BIT3;

    P1IFG &= ~BIT3;
}
