#include <msp430g2553.h>

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set the DCO to 1MHz
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1DIR |= BIT6;
    P1OUT &= ~BIT6; // Start with the pin LOW
   
    TACCR0 = 20000;

    TACCR1 = 5000;

    TACCTL0 = CCIE;
    TACCTL1 = CCIE;

    TACTL = TASSEL_2 | MC_1 | TACLR;

    __bis_SR_register(LPM0_bits | GIE);
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    P1OUT |= BIT6;  // Set the PWM pin HIGH
}


#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{

    switch(__even_in_range(TAIV, 10))
    {
        case 0: break;               // No interrupt
        case 2:                      // CCR1 interrupt flag is set
            P1OUT &= ~BIT6;          // Set the PWM pin LOW
            break;
        case 4: break;               // CCR2 not used
        case 10: break;              // Timer overflow (TAIFG) not used
    }
}
