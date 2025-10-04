#include <msp430g2553.h>

// Baud rate configuration
#define BAUD_RATE   9600
#define CLK_FREQ  1000000  // SMCLK = 1 MHz

// Globals
volatile unsigned int button_count = 0;
volatile unsigned int tx_index = 0;
volatile char message_buffer[32];   // "quote + count + CRLF"

const char quote[] = { "God loves you!" };

void send_message(void);

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
    P1IE  |= BIT3;            // Interrupt enable
    P1IES |= BIT3;            // Falling edge
    P1IFG &= ~BIT3;           // Clear flag

    // Configure UART pins
    P1SEL |= BIT1 | BIT2;     
    P1SEL2 |= BIT1 | BIT2;

    // Configure USCI_A0 for UART
    UCA0CTL1 |= UCSSEL_2;     // SMCLK
    UCA0BR0 = (CLK_FREQ / BAUD_RATE) & 0xFF;
    UCA0BR1 = (CLK_FREQ / BAUD_RATE) >> 8;
    UCA0MCTL = UCBRS0;        
    UCA0CTL1 &= ~UCSWRST;     

    __bis_SR_register(GIE);   // Enable global interrupts

    while (1)
    {
        __bis_SR_register(LPM0_bits); // LPM0 until ISR
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)  // UART TX ISR
{
    if (message_buffer[tx_index] != '\0')
    {
        UCA0TXBUF = message_buffer[tx_index++];
    }
    else
    {
        IE2 &= ~UCA0TXIE;     // Stop TX interrupts
        tx_index = 0;         // Reset index
        __bic_SR_register_on_exit(LPM0_bits);
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)  // BUTTON PRESS ISR
{
    P1IE &= ~BIT3;                 // Disable button interrupts
    P1IFG &= ~BIT3;                // Clear flag

    // Setup Timer_A for debounce (~10ms)
    TACCR0 = 30000;                
    TACTL = TASSEL_2 | MC_1 | TACLR; 
    TACCTL0 = CCIE;                // Enable CCR0 interrupt
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    TACCTL0 &= ~CCIE;    // Disable CCR0 interrupt
    TACTL = MC_0;        // Stop timer

    if (!(P1IN & BIT3))  // Button still pressed (active low)
    {
        button_count++;
        P1OUT ^= BIT0;
        send_message();
    }

    P1IES |= BIT3;       // Re-arm for falling edge
    P1IFG &= ~BIT3;      // Clear any pending flags
    P1IE  |= BIT3;       // Re-enable button interrupt

    __bic_SR_register_on_exit(LPM0_bits);
}

void send_message(void)
{
    unsigned int i = 0;

    // Copy quote
    while (quote[i] != '\0')
    {
        message_buffer[i] = quote[i];
        i++;
    }

    // Just append CRLF
    message_buffer[i++] = '\r';
    message_buffer[i++] = '\n';
    message_buffer[i] = '\0';

    tx_index = 0;
    IE2 |= UCA0TXIE;   // Enable TX interrupt to start sending
}
