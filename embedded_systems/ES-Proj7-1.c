#include <msp430g2553.h>

// UART baud rate configuration
#define BAUD_RATE 9600
#define SMCLK_FREQ    1000000  // SMCLK = 1 MHz

volatile char rx_char = 0;

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set DCO to 1MHz
    BCSCTL2 &= ~SELS;         // SMCLK = DCO
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // Configure LEDs on P1.0 (red) and P1.6 (green)
    P1DIR |= BIT0 | BIT6;     // Outputs
    P1OUT &= ~(BIT0 | BIT6);  // Initially off

    // Configure UART pins
    P1SEL |= BIT1 | BIT2;     // P1.1 = RXD, P1.2 = TXD
    P1SEL2 |= BIT1 | BIT2;

    // Configure USCI_A0 for UART
    UCA0CTL1 |= UCSSEL_2;     // SMCLK
    UCA0BR0 = (SMCLK_FREQ / BAUD_RATE) & 0xFF; 
    UCA0BR1 = (SMCLK_FREQ / BAUD_RATE) >> 8;
    UCA0MCTL = UCBRS0;        // Modulation
    UCA0CTL1 &= ~UCSWRST;     // Initialize USCI

    // Enable USCI_A0 RX interrupt
    IE2 |= UCA0RXIE;

    __bis_SR_register(GIE);   // Enable global interrupts

    while (1)
    {
        __bis_SR_register(LPM0_bits); // LPM until ISR
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    rx_char = UCA0RXBUF; // Read received char

    switch (rx_char)
    {
        case 'r':
            P1OUT ^= BIT0;   // Toggle red
            break;

        case 'g':
            P1OUT ^= BIT6;   // Toggle green
            break;

        default:
            // Ignore everything else
            break;
    }

    __bic_SR_register_on_exit(LPM0_bits); // Wake up -> LPM
}
