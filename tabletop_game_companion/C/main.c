#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Definitions
*/
#define DEBOUCE_COUNT 100  // Placeholder value
#define COMMAND_TIMEOUT 10000  // Placeholder value
#define TAIV_2 2  // TACCR1 CCIFG
#define DEBOUNCE_DONE ((P2IES & active_pin) == 0)
/*
Shift buffer to fix MSB and LSB mismatch
*/
#define SHIFT_BUFFER(num)  ((((num) & 0x0F) << 4) | (((num) & 0xF0) >> 4))
/*
FSM type definition
*/
typedef enum {
  IDLE,
  COMMAND,
  COUNT,
  RANDOM
} state_t;
/*
Global variable instantiations
*/
volatile state_t system_state = IDLE;
volatile unsigned char rng_num = 0x00;
volatile unsigned char active_pin = 0;
volatile unsigned char current_count = 0;
/*
Helper functions
*/
unsigned char hex_to_decimal(unsigned char input) {  // Return %10 on 2-digit hex digits in char
  unsigned char upper = (input & 0xF0) >> 4;
  unsigned char lower = (input & 0x0F);
  upper = upper % 10;
  lower = lower % 10;
  return (upper << 4) | lower;
}
void display_scramble_1_digit(void) {  // RNG visual sequence that only shifts BCD
  unsigned char val = 0x08;
  unsigned char i;
  for (i = 0; i < 7; i++) {
    SIPO_reset();
    SIPO_shift(SHIFT_BUFFER(val));
    __delay_cycles(50); 
    val += 0x10; 
    if (val > 0x98) {
      val = 0x08;
    }
  }
}
void display_scramble_2_digits(void) {  // RNG visual sequence that only shifts BCD
  unsigned char val = 0x00;
  unsigned char i;
  for (i = 0; i < 20; i++) {
    SIPO_reset();
    SIPO_shift(SHIFT_BUFFER(val));
    __delay_cycles(50);
    val++;
    if ((val & 0x0F) > 0x09) {
      val += 0x06; 
    }
  }
}
/*
Main function
*/
int main(void) {
  MSP430G2452_init();
  /*
  Init SIPO
  */
  SIPO_reg_init(&P1OUT, &P1OUT, &P1OUT);
  SIPO_pin_init(BIT0, BIT2, BIT1);
  SIPO_shift(0x14);
  /*
  Init Timer_A
  */
  TA0CTL = TASSEL_1 + MC_2; // ACLK, continuous mode
  /*
  Enable P2.1 interrupt
  */
  active_pin = BIT1;
  P2IES |= BIT1;  // Set edge select to high-low
  P2IFG &= ~BIT1;  // Clear flag
  P2IE |= BIT1;  // Enable interrupt for P2.1
  __bis_SR_register(GIE);
  /*
  FSM loop
  */
  while (1) {
    switch (system_state) {
      case IDLE:  // LPM and wait to turn on
        SIPO_shift(SHIFT_BUFFER(0x8F));
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        SIPO_shift(SHIFT_BUFFER(0x20));
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COUNT:  // Make "random" number in set range and display for some time
        SIPO_shift(SHIFT_BUFFER(current_count));  // MAKE THIS %10 TO REMOVE LETTERS!!!
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case RANDOM:  // Increment counter and display for some time
        rng_num = hex_to_decimal(rng_num);
        display_scramble_1_digit();
        SIPO_shift(SHIFT_BUFFER(rng_num));
        __delay_cycles(5000);
        system_state = IDLE;
        __bis_SR_register(LPM3_bits + GIE);
        break;
   }
  }
  return 0;  // End of program
}
/*
Interrupts
*/
#pragma vector = PORT2_VECTOR
__interrupt void Port_2_ISR(void) {
    switch (system_state) {
      case IDLE:
        if (DEBOUNCE_DONE) {  // If rising transition
          system_state = COMMAND;
        }
        else {
          TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
          TACCTL0 = CCIE;  // Enable Timer_A interrupt 0
        }
        P2IE &= ~active_pin;  // Disable button interrupt
        P2IES ^= active_pin;  // Switch relevant edge
        P2IFG &= ~active_pin;  // Clear any accumulated button flags
        break;

      case COMMAND:
        if (DEBOUNCE_DONE) { // If rising transition
          switch (active_pin) {
            case BIT1:
              system_state = RANDOM;
              rng_num = TAR;
              break;
            case BIT2:
              system_state = COUNT;
              break;
            default:
              system_state = IDLE;
              break;
          }
        }
        else {
          if (P2IFG & BIT1) {  // If P2.1 triggered
            active_pin = BIT1;
            P2IE &= ~BIT2; // Disable P2.2
          }
          else {  // Do the opposite
            active_pin = BIT2;
            P2IE &= ~BIT1;
          }
          TACCR0 = TAR + DEBOUCE_COUNT; // Schedule debounce interrupt
          TACCTL0 = CCIE; // Enable Timer_A interrupt 0
        }
        P2IE &= ~active_pin;   // Disable interrupt to prevent bounce
        P2IES ^= active_pin;   // Flip edge to look for Release
        P2IFG &= ~active_pin;  // Clear flag so we don't loop forever
        break;

      case COUNT:
        if (DEBOUNCE_DONE) { // If rising transition
          if (active_pin == BIT2) {
            current_count +=1;
            system_state = COUNT;
          }
          else {
          system_state = IDLE;
          }
        }
        else {
          TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
          TACCTL0 = CCIE;  // Enable Timer_A interrupt 0
        }
        P2IE &= ~active_pin;   // Disable interrupt for active pin
        P2IES ^= active_pin;   // Switch relevant edge
        P2IFG &= ~active_pin;  // Clear flag
        break;
    }
    __bic_SR_register_on_exit(LPM3_bits);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {  // For button debounce (highest priority)
  TACCTL0 &= ~CCIE;      // Disable Timer_A interrupt
  P2IFG &= ~active_pin;  // Clear flag for the specific button that was pressed
  P2IE |= active_pin;    // Re-enable interrupt for that specific button
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {  // Timeout
  if (TAIV == TAIV_2) {
    system_state = IDLE;
  }
  TA0CCTL1 &= ~CCIE;
}
