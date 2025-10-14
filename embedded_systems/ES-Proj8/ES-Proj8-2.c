#include <msp430.h>

#define SLAVE_ADDR 0x50  // 7-bit I2C address
#define READ_ADDR 0xE0


typedef enum {DUMMY_WRITE, SEND_ADDR, READ_DATA} state_t;

volatile state_t current_state = DUMMY_WRITE;

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
    
    switch (current_state)
    {

      case DUMMY_WRITE:
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
        __bis_SR_register(GIE);  // Enter LPM0 w/ interrupts
        break;

      case SEND_ADDR:

        __bis_SR_register(GIE);  // Enter LPM0 w/ interrupts
        break;

      case READ_DATA:
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        break;

      default:

        break;
    }

  }


}

// USCI_B0 RX ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  switch (current_state)
  {
    case DUMMY_WRITE:
      UCB0TXBUF = READ_ADDR;
      current_state = SEND_ADDR;
      break;
    case SEND_ADDR:
      current_state = READ_DATA;
      break;
  }

  IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag

}
