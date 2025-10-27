#include "msp430.h"

#define FALSE 0
#define TRUE 1

#define LCM_DIR P2DIR
#define LCM_OUT P2OUT

#define LCM_PIN_RS BIT0 // P2.0
#define LCM_PIN_EN BIT1 // P2.1
#define LCM_PIN_D4 BIT2 // P2.2
#define LCM_PIN_D5 BIT3 // P2.3
#define LCM_PIN_D6 BIT4 // P2.4
#define LCM_PIN_D7 BIT5 // P2.5

#define LCD_DATA_MASK (LCM_PIN_D4 | LCM_PIN_D5 | LCM_PIN_D6 | LCM_PIN_D7)
#define LCD_PIN_MASK (LCM_PIN_RS | LCM_PIN_EN | LCD_DATA_MASK)

// Delay const for 1 MHz clock
#define PULSE_DELAY 200     // delay 0.2 ms
#define START_DELAY 100000 // delay 100 ms


void PulseLcm()
{
    LCM_OUT &= ~LCM_PIN_EN; // pull EN bit low
    __delay_cycles(PULSE_DELAY);
    LCM_OUT |= LCM_PIN_EN; // pull EN bit high
    __delay_cycles(PULSE_DELAY);
    LCM_OUT &= ~LCM_PIN_EN; // pull EN bit low again
    __delay_cycles(PULSE_DELAY);
}

void SendByte(char ByteToSend, int IsData)
{
    // Clear RS/EN and data pins
    LCM_OUT &= ~(LCD_PIN_MASK);

    LCM_OUT |= ((ByteToSend & 0xF0) >> 2);

    if (IsData == TRUE)
        LCM_OUT |= LCM_PIN_RS;
    
    PulseLcm(); // Latch the data

    // Clear RS/EN and data pins
    LCM_OUT &= ~(LCD_PIN_MASK);

    // Set data bits
    LCM_OUT |= ((ByteToSend & 0x0F) << 2);

    if (IsData == TRUE)
        LCM_OUT |= LCM_PIN_RS;

    PulseLcm(); // Latch data
}

void LcmSetCursorPosition(char Row, char Col)
{
    char address;
    if (Row == 0)
        address = 0;
    else
        address = 0x40;
    address |= Col;
    SendByte(0x80 | address, FALSE);
}

void ClearLcmScreen()
{
    SendByte(0x01, FALSE);
    SendByte(0x02, FALSE);
}

void PrintStr(char *Text)
{
    char *c = Text;
    while ((c != 0) && (*c != 0))
    {
        SendByte(*c, TRUE);
        c++;
    }
}

void InitializeLcm(void)
{
    LCM_DIR |= LCD_PIN_MASK;
    LCM_OUT &= ~(LCD_PIN_MASK);
    __delay_cycles(START_DELAY);
    LCM_OUT &= ~LCM_PIN_RS;
    LCM_OUT &= ~LCM_PIN_EN;
    LCM_OUT = LCM_PIN_D5;  // 4 bit mode
    PulseLcm();
    SendByte(0x28, FALSE); // 4-bit, 2-line
    SendByte(0x0E, FALSE); // Display on, cursor on
    SendByte(0x06, FALSE); // Auto-increment cursor
}


// Globals for ADC interrupt
volatile unsigned int adc_samples[4];
volatile unsigned char sample_counter = 0;
volatile unsigned int final_adc_value = 0;
volatile int new_voltage = FALSE;

void watchdog_config();
void clk_config();
void pin_config();
void timer_config();
void adc_config();
void uart_config();
void uart_tx_str(char *str);
void adc_avg_to_str(unsigned int adc_val, char *buffer);

int main()
{
    char volt_str[10]; // Buffer for voltage string

    // Configure hardware
    watchdog_config();
    clk_config();          // 1MHz clock
    pin_config();         // P1 for ADC and UART
    uart_config();   // RS232
    adc_config();        // ADC on P1.3
    timer_config();      // 4Hz sampling timer
    InitializeLcm();     // LCD on Port 2

    __enable_interrupt(); // GIE

    // Default text
    ClearLcmScreen();
    PrintStr("Voltmeter");
    LcmSetCursorPosition(1, 0);
    PrintStr("0.00 V");

    while (1)
    {
        // Sleep until 4 ADC samples held
        __bis_SR_register(LPM0_bits);

        // CPU awake, check flag
        if (new_voltage)
        {
            new_voltage = FALSE; // Clear flag

            // Convert avg ADC value to str
            adc_avg_to_str(final_adc_value, volt_str);

            // Update LCD
            LcmSetCursorPosition(1, 0);
            PrintStr(volt_str);

            // Update UART
            uart_tx_str(volt_str);
            uart_tx_str("\r\n");
        }
    }
}


void watchdog_config()
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
}

void clk_config()
{
    // Set DCO to 1MHz
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
}

void pin_config()
{
    // Set event loop pin
    P1DIR |= BIT4;   // Set P1.4 as output
    P1OUT &= ~BIT4;  // Start low

    // Configure P1.3 as ADC input
    P1DIR &= ~BIT3; // Set P1.3 as input
    P1SEL |= BIT3;  // Enable A3 analog input
    P1SEL2 &= ~BIT3;

    // Configure P1.1 RX and P1.2 TX
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
}

void uart_config()
{
    // Config for 9600 Baud
    UCA0CTL1 |= UCSSEL_2; // Use SMCLK (1MHz)
    UCA0BR0 = 104;        // 1MHz / 9600 = 104.16
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_1;   // Mod UCBRSx = 1 (0.16)
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI
}

void adc_config()
{
    // VCC/VSS ref, 16x sample/hold, enable interrupts
    ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON + ADC10IE;
    // Input P1.3, one CH, single conv
    ADC10CTL1 = INCH_3 + CONSEQ_0;
    ADC10AE0 |= BIT3; // Enable analog input P1.3
}

void timer_config()
{
    // Configure Timer_A for 4Hz interrupt
    // SMCLK (1MHz) / 8 (divider) = 125,000 Hz
    // 125,000 / 31250 = 4 Hz
    TA0CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Up Mode, /8 Divider
    TA0CCR0 = 31250 - 1;             // Set timer period for 4Hz
    TA0CCTL0 = CCIE;                 // Enable CCR0 interrupt
}


void uart_tx_str(char *str)
{
    while (*str)
    {
        while (!(IFG2 & UCA0TXIFG)); // Wait for TX buffer empty
        UCA0TXBUF = *str++;          // Send char and ++pointer
    }

    while (!(IFG2 & UCA0TXIFG)); // Wait until last byte done
    P1OUT &= ~BIT4;               // Reset P1.4 low for timing test
}

void adc_avg_to_str(unsigned int adc_val, char *buffer)
{
    // V = adc_val * (3000mV / 1023)
    unsigned long millivolts = (unsigned long)adc_val * 3000;
    millivolts = millivolts / 1023;

    // Extract each digit
    // e.g., 1234mV (1.23V)
    unsigned int v  = millivolts / 1000;         // 1234 / 1000 = 1
    unsigned int d1 = (millivolts % 1000) / 100; // 234 / 100 = 2
    unsigned int d2 = (millivolts % 100) / 10;  // 34 / 10 = 3
    
    // Convert digits (0-9) to ASCII characters ('0'-'9')
    buffer[0] = v + '0';
    buffer[1] = '.';
    buffer[2] = d1 + '0';
    buffer[3] = d2 + '0';
    buffer[4] = ' ';
    buffer[5] = 'V';
    buffer[6] = '\0'; // Null terminator
}

// INTERRUPTS

// Timer A ISR
// Calls 4 times per second starting ADC conversion
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_ISR()
{
    P1OUT |= BIT4;            // Set P1.4 for timing test
    // Start ADC conversion
    ADC10CTL0 |= ENC + ADC10SC;
}

// ADC ISR
// Calls after conversion is complete
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR()
{
    adc_samples[sample_counter] = ADC10MEM; // Store new sample
    sample_counter++;

    // If 4 samples, average them
    if (sample_counter >= 4)
    {
        sample_counter = 0; // Rst count

        // Take average
        unsigned long temp_avg = 0;
        temp_avg = adc_samples[0] + adc_samples[1] + adc_samples[2] + adc_samples[3];
        
        final_adc_value = temp_avg >> 2; // Div by 2 with bit shift

        // Set flag for main
        new_voltage = TRUE;

        // Wake up CPU
        __bic_SR_register_on_exit(LPM0_bits);
    }
}
