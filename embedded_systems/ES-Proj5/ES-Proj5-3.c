#include <msp430g2553.h>

typedef enum { PROGRAM_1, PROGRAM_2, PROGRAM_3 } program_t;

volatile program_t current_program = PROGRAM_1;
volatile unsigned int current_program_time = 40; // 40 * 250 ms = abt 10 seconds

void program_transition_blink(unsigned char num_blinks);

#define TACCR1_CCIFG 0x02

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer
    
    // Clock setup
    BCSCTL3 |= LFXT1S_2;        // ACLK = VLO (12kHz)
    
    // GPIO setup
    P1DIR |= (BIT0 | BIT6);     // Init LEDs to output
    P1OUT &= ~(BIT0 | BIT6);    // Init LEDs off
    
    // Timer_A setup
    TACTL = TASSEL_1 + MC_2 + TACLR;  // ACLK, Continuous Mode, Clear timer
    TACCR0 = 6000;              // 500 ms period for LED blink
    TACCR1 = 3000;              // 250 ms period for program transition
    TACCTL1 = CCIE;             // Enable CCR1 interrupt
    
    __enable_interrupt();       // Enable global interrupts

    while (1) {
        switch (current_program) {
            case PROGRAM_1:
                program_transition_blink(1);
                TACCTL0 &= ~CCIE;   // Disable CCR0 interrupt
                P1OUT &= ~BIT6;     // Confirm green LED is off
                while (current_program == PROGRAM_1) {
                    P1OUT ^= BIT6;  // Toggle green LED
                    __delay_cycles(500000); // Active mode delay
                }
                break;

            case PROGRAM_2:
                program_transition_blink(2);
                TACCTL0 |= CCIE;    // Enable CCR0 interrupt
                while (current_program == PROGRAM_2) {
                    __low_power_mode_0(); // Enter LPM0
                }
                break;

            case PROGRAM_3:
                program_transition_blink(3);
                TACCTL0 |= CCIE;    // Enable CCR0 interrupt
                while (current_program == PROGRAM_3) {
                    __low_power_mode_3(); // Enter LPM3
                }
                break;
        }
    }
}

void program_transition_blink(unsigned char num_blinks) {
    unsigned char i;
    for (i = 0; i < num_blinks; ++i) {
        P1OUT |= BIT0;          // Turn on red LED
        __delay_cycles(100000); // ~100ms on delay
        P1OUT &= ~BIT0;         // Turn off red LED
        __delay_cycles(100000); // 100ms off delay
    }
    __delay_cycles(200000);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    P1OUT ^= BIT6;              // Toggle green LED
    TACCR0 += 6000;             // Add offset for next interrupt
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {
    if (TAIV == TACCR1_CCIFG) {  // Check interrupt vector

        TACCR1 += 3000;     // Add offset for next interrupt
        --current_program_time;
        if (current_program_time == 0) {
            current_program_time = 40;  // Reset to 10 s
            ++current_program;
            if (current_program > PROGRAM_3) {
                current_program = PROGRAM_1;
            }
            __low_power_mode_off_on_exit(); // Wake up
        }

    }
}
