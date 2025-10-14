#include <msp430.h>

#define SLAVE_ADDR 0x50  // 7-bit I2C EEPROM address
#define READ_ADDR  0xE0
typedef enum {DUMMY_WRITE, SEND_ADDR, RESTART, RX, DONE} state_t;

volatile state_t current_state = DUMMY_WRITE;
volatile unsigned char RXData[] = {0, 0, 0, 0};
volatile unsigned char RXCount = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Clock setup
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // I2C Pins: P1.6 = SCL, P1.7 = SDA
    P1SEL  |= BIT6 | BIT7;
    P1SEL2 |= BIT6 | BIT7;

    // USCI_B0 setup
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;      // I2C Master, synchronous
    UCB0CTL1 = UCSSEL_2 | UCSWRST;            // Use SMCLK
    UCB0BR0 = 20;                             // ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;                   // Slave Address
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset

    IE2 |= UCB0TXIE;                          // Enable TX interrupt

    // Button Setup
    P1DIR &= ~BIT3;     // Set P1.3 to input
    P1REN |= BIT3;      // Enable resistor for P1.3
    P1OUT |= BIT3;      // Set resistor to pull-up
    P1IE |= BIT3;       // P1.3 interrupt enabled
    P1IES |= BIT3;      // P1.3 Both edge
    P1IFG &= ~BIT3;     // P1.3 IFG cleared
    

    // Go to low-power until button press
    __bis_SR_register(LPM3_bits + GIE);


    while (1)
    {
        switch (current_state)
        {
            case DUMMY_WRITE:
                RXCount = 0;
                UCB0CTL1 |= UCTR + UCTXSTT;    // TX mode + START condition
                //__bis_SR_register(GIE);
                break;

            case SEND_ADDR:
                //__bis_SR_register(GIE);
                break;

            case RESTART:
                IE2 &= ~UCB0TXIE;           // Disable TX interrupt
                IE2 |= UCB0RXIE;            // Enable RX interrupt
                UCB0CTL1 &= ~UCTR;          // Switch to RX mode
                current_state = RX;
                UCB0CTL1 |= UCTXSTT;        // START


                //while (UCB0CTL1 & UCTXSTT)    // Wait for START to complete
                //{
                //;
                //}
                //__bis_SR_register(GIE);


                break;

            case RX:
                if (RXCount == 1) 
                {
                UCB0CTL1 |= UCTXSTP;
                current_state = DONE;
                }
                break;

            case DONE:
                IE2 &= ~UCB0RXIE;
                __bis_SR_register(LPM0_bits);
                break;
        }
    }
}

// USCI_B0 TX ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    switch (current_state)
    {
        case DUMMY_WRITE:
            UCB0TXBUF = READ_ADDR;   // Send EEPROM memory address
            current_state = SEND_ADDR;
            break;

        case SEND_ADDR:
            current_state = RESTART;
            break;
    }

    IFG2 &= ~UCB0TXIFG;              // Clear TX flag
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
    // Read the byte from the buffer
    RXData[RXCount] = UCB0RXBUF;
    ++RXCount;
    IFG2 &= ~UCB0RXIFG;

}


// Button ISR for debugging
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT3;
    __bic_SR_register_on_exit(LPM3_bits);
}
