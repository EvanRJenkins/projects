#include <msp430.h>

#define SERVO_MIN_US 1000     // 1nms pulse width
#define SERVO_MAX_US 2000    // 2nms pulse width
#define SERVO_PERIOD_US 20000    // 20nms period (50Hz)

typedef enum {
    POSITION_LEFT,
    POSITION_RIGHT
} servo_position_t;

volatile servo_position_t current_position = POSITION_LEFT;

void pwm_set_position(servo_position_t position);
void debounce_start(void);

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set the DCO to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // Set P1.6 as PWM output (TA0.1)
    P1DIR |= BIT6;
    P1SEL |= BIT6;

    // Configure Timer_A for PWM
    TA0CCR0 = SERVO_PERIOD_US - 1;      // Set PWM period
    TA0CCTL1 = OUTMOD_7;                // Reset/Set output mode
    TA0CCR1 = SERVO_MIN_US;             // Initial duty cycle
    TA0CTL = TASSEL_2 | MC_1 | TACLR;   // SMCLK, up mode, clear timer
    
    // Configure P1.3 as input with pull-up resistor
    P1DIR &= ~BIT3;
    P1REN |= BIT3;
    P1OUT |= BIT3;

    // Enable interrupt on falling edge (button press)
    P1IES |= BIT3;
    P1IE  |= BIT3;
    P1IFG &= ~BIT3;     // Clear interrupt flag

    // Enable global interrupts
    __enable_interrupt();

    while (1)
    {
        // Update PWM duty cycle based on current position
        pwm_set_position(current_position);

        // Enter low-power mode, wait for interrupt
        __bis_SR_register(LPM0_bits + GIE);
    }
}



void pwm_set_position(servo_position_t position)
{
    if (position == POSITION_LEFT)
        TA0CCR1 = SERVO_MIN_US;
    else
    {
        TA0CCR1 = SERVO_MAX_US;
    }
}


void debounce_start(void)
{
    WDTCTL = WDT_MDLY_8;   // Set WDT to 8 ms interval
    IE1 |= WDTIE;          // Enable WDT i
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    P1IE &= ~BIT3;           // Disable button interrupt
    debounce_start();        // Start debounce timer
    P1IFG &= ~BIT3;          // Clear button flag
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT
    IE1 &= ~WDTIE;              // Disable WDT interrupt

    // On button release, toggle position
    if ((P1IN & BIT3) && !(P1IES & BIT3))
    {
        current_position = (current_position == POSITION_LEFT)
                           ? POSITION_RIGHT
                           : POSITION_LEFT;
    }

    P1IES ^= BIT3;             // Toggle interrupt edge
    P1IE  |= BIT3;             // Re-enable button interrupt

    __bic_SR_register_on_exit(LPM0_bits);  // Exit LPM
}
