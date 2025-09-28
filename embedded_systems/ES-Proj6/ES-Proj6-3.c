#include <msp430g2553.h>

#define STEP_DELAY_SLOW 10  // ms delay slow speed
#define STEP_DELAY_FAST 3   // ms delay fast speed

typedef enum { OFF, SLOW, FAST } speed_t;

volatile speed_t current_speed = OFF;
volatile unsigned int step_index = 0;

// Speed sequence LUT
const unsigned char step_pattern[4] = {
    0b0001,  // P2.0
    0b0010,  // P2.1
    0b0100,  // P2.2
    0b1000   // P2.3
};

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set DCO to 1MHz
    BCSCTL2 &= ~SELS;         // SMCLK = DCO
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // Configure pushbutton on P1.3
    P1DIR &= ~BIT3;           // Input
    P1REN |= BIT3;            // Enable resistor
    P1OUT |= BIT3;            // Pull-up
    P1IE  |= BIT3;            // Enable interrupt
    P1IES |= BIT3;            // Falling edge
    P1IFG &= ~BIT3;           // Clear flag

    // Configure status LED on P1.0
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // Configure stepper outputs on P2.0–P2.3
    P2DIR |= BIT0 | BIT1 | BIT2 | BIT3;
    P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Initialize Timer_A
    TACCTL0 = CCIE;           // Enable CCR0 interrupt
    TACTL = TASSEL_2 | MC_1 | TACLR; // SMCLK, up mode, clear

    __bis_SR_register(GIE);   // Enable global interrupts

    while (1)
    {
        switch (current_speed)
        {
            case OFF:
                TACTL &= ~MC_3; // Stop timer
                TACCTL0 &= ~CCIE;
                P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3); // Coils off
                break;

            case SLOW:
                TACCR0 = (1000 * STEP_DELAY_SLOW) - 1;
                TACCTL0 |= CCIE;
                TACTL |= MC_1; // Up mode
                break;

            case FAST:
                TACCR0 = (1000 * STEP_DELAY_FAST) - 1;
                TACCTL0 |= CCIE;
                TACTL |= MC_1; // Up mode
                break;

            default:
                current_speed = OFF;
                break;
        }

        __bis_SR_register(LPM0_bits); // Enter low power mode until ISR
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    // Clear previous step
    P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Apply next step
    P2OUT |= step_pattern[step_index % 4];
    step_index = (step_index + 1) % 4;

    __bic_SR_register_on_exit(LPM0_bits); // Wake main
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
    P1IE &= ~BIT3;                 // Disable further button interrupts
    WDTCTL = WDT_MDLY_0_064;       // Set WDT ~64ms
    IE1 |= WDTIE;                  // Enable WDT interrupt
    P1IFG &= ~BIT3;                // Clear flag
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
    IE1 &= ~WDTIE;                 // Disable WDT interrupt
    WDTCTL = WDTPW | WDTHOLD;      // Stop WDT

    // On valid button release, cycle speed
    if (((P1IES & BIT3) == 0) && (P1IN & BIT3))
    {
        current_speed = (speed_t)((current_speed + 1) % 3);
        P1OUT ^= BIT0;             // Toggle indicator LED
    }

    P1IES ^= BIT3;                 // Toggle edge
    P1IE |= BIT3;                  // Re-enable button interrupt
    IFG1 &= ~WDTIFG;               // Clear flag

    __bic_SR_register_on_exit(LPM0_bits); // Wake up
}
