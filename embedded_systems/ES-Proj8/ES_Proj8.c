#include <msp430.h>

#define SLAVE_ADDR 0x50  // 7-bit I2C address

volatile unsigned char RXData[3];
volatile unsigned char data_index = 0;

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
    UCB0CTL1 |= UCSWRST;                     // Enable SW reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;    // I2C master, sync mode
    UCB0CTL1 = UCSSEL_2 | UCSWRST;           // SMCLK
    UCB0BR0 = 10;                            // 100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;                  // Slave Address
    UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset
    IE2 |= UCB0RXIE;                         // Enable RX interrupt

    data_index = 0;

    // Start read operation
    UCB0CTL1 &= ~UCTR;   // Receiver mode
    UCB0CTL1 |= UCTXSTT; // Send START

    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts

    // Done receiving
    __no_operation(); // Breakpoint here to check RXData[]
}

// USCI_B0 RX ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if (UCB0RXBUF != 0xFF)
  {
    RXData[data_index++] = UCB0RXBUF;
  }
    if (data_index > 2) {
        UCB0CTL1 |= UCTXSTP;                  // Send STOP
        IFG2 &= ~UCB0RXIFG;                   // Clear flag
        __bic_SR_register_on_exit(CPUOFF);    // Wake up
    }
}
