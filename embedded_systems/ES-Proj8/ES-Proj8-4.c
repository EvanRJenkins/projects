#include <msp430.h>

#define SLAVE_ADDR 0x50  // 7-bit I2C EEPROM address
#define READ_ADDR  0xE0  // 8-bit EEPROM word address
typedef enum {DUMMY_WRITE, SEND_ADDR, READ_DATA, DONE} state_t;

volatile state_t current_state = DUMMY_WRITE;
volatile unsigned char RXData;

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
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;    // I2C Master, synchronous
    UCB0CTL1 = UCSSEL_2 | UCSWRST;           // Use SMCLK
    UCB0BR0 = 20;                            // ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;                  // Slave Address
    UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset
    IE2 |= UCB0TXIE;                         // Enable TX interrupt

    while (1)
    {
        switch (current_state)
        {
          case DUMMY_WRITE:
            UCB0CTL1 |= UCTR + UCTXSTT;    // TX mode + START condition
            __bis_SR_register(GIE);
            break;

          case SEND_ADDR:
            __bis_SR_register(GIE);
            break;

          case READ_DATA:
            IE2 &= ~UCB0TXIE;              // Disable TX interrupt
            IE2 |= UCB0RXIE;               // Enable RX interrupt
            UCB0CTL1 &= ~UCTR;             // Switch to RX mode
            UCB0CTL1 |= UCTXSTT;           // Repeated START

            // Wait for START to complete before issuing STOP
            while (UCB0CTL1 & UCTXSTT);    // Wait until STT cleared
            UCB0CTL1 |= UCTXSTP;           // Send STOP
            __bis_SR_register(GIE);
            break;

          case DONE:
            break;
        }
    }
}

// USCI_B0 ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  switch (current_state)
  {
    case DUMMY_WRITE:
      UCB0TXBUF = READ_ADDR;   // Send EEPROM word address
      current_state = SEND_ADDR;
      break;

    case SEND_ADDR:
      current_state = READ_DATA;
      break;

      case READ_DATA:
        RXData = UCB0RXBUF;
        current_state = DONE;
        break;
    }
    IFG2 &= ~UCB0TXIFG;                 // Clear TX flag
}
