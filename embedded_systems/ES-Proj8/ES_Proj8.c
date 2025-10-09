#include <msp430.h>

#define SLAVE_ADDR  0x3A     // I2C slave address
#define READ_ADDR 0xE1      // EEPROM start read address


volatile unsigned char RXData[4];  // Array to store secret code chars

unsigned char *p_RXData = RXData;
unsigned char RXCount = 0;
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;      // Stop watchdog timer

    // ----- Clock setup -----
    BCSCTL1 = CALBC1_1MHZ;         // DCO = 1 MHz
    DCOCTL  = CALDCO_1MHZ;

    // ----- Configure I2C pins -----
    // P1.6 = SCL, P1.7 = SDA
    P1SEL  |= BIT6 | BIT7;         // Select I2C function
    P1SEL2 |= BIT6 | BIT7;
    P1DIR  &= ~(BIT6 | BIT7);      // Inputs (open-drain)
    P1REN  &= ~(BIT6 | BIT7);      // Disable internal pull-ups/downs
    P1OUT  &= ~(BIT6 | BIT7);

    // ----- USCI_B0 I2C Master setup -----
    UCB0CTL1 |= UCSWRST;           // Hold in reset during config
    UCB0CTL0  = UCMST | UCMODE_3 | UCSYNC; // I2C master, synchronous mode
    UCB0CTL1  = UCSSEL_2 | UCSWRST;        // SMCLK source
    UCB0BR0   = 10;                // 1 MHz / 10 = 100 kHz SCL
    UCB0BR1   = 0;
    UCB0I2CSA = SLAVE_ADDR;        // Set Slave address
    UCB0CTL1 &= ~UCSWRST;          // Release USCI for operation

    // ----- Generate START and address -----
    UCB0CTL1 &= ~UCTR;      // RX mode 
    UCB0CTL1 |= UCTXSTT;    // START condition
    while (UCB0CTL1 & UCTXSTT);    // Wait for START to finish (address sent)

    // ----- Generate STOP immediately -----
    UCB0CTL1 |= UCTXSTP;           // Send STOP
    while (UCB0CTL1 & UCTXSTP);    // Wait for STOP complete

  while (1)
  {
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
    while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts



  }
}

// USCI_B0 Data ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if (RXCount)
    {
        *p_RXData++ = UCB0RXBUF;                       // Get RX data and ++ptr
        --RXCount;
    }
    else  // End communication and disable USCI
    {

    }
  __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
