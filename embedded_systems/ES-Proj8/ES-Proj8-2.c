#include <msp430.h>

#define SLAVE_ADDR 0x50  // 7-bit I2C address
#define READ_ADDR 0xE0

volatile unsigned char RXData[3];
volatile unsigned char done = 0;

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
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;           // SMCLK
    UCB0BR0 = 20;
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;                  // Slave Address
    UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset
    IE2 |= UCB0TXIE;                         // Enable TX interrupt

    

    UCB0CTL1 |= UCTR;   // TX mode
    while (1)
  {
    
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    done = 1;
  }


}

// USCI_B0 RX ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  UCB0TXBUF = READ_ADDR;
  if (done)
  {
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
  }
  IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
}
