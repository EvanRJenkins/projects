#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Constant definitions
*/
#define DEBOUCE_COUNT 1000  // Placeholder value
#define COMMAND_TIMEOUT 10000  // Placeholder value
#define TAIV_2 2  // TACCR1 CCIFG
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
  MENU,
  COUNT,
  RANDOM
} state_t;
/*
Global variable instantiations
*/
volatile state_t system_state = IDLE;
volatile unsigned int temp_timer = 0x00;
volatile unsigned char rng_num = 0x00;
volatile unsigned char active_pin = 0;
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
  TACTL = TASSEL_2 + MC_2;  // SMCLK, continuous mode
  /*
  Enable P2.1 interrupt
  */
  P2IES |= BIT1;  // Set edge select to high-low
  P2IFG &= ~BIT1;  // Clear flag
  P2IE |= BIT1;  // Enable interrupt for P2.1
  __bis_SR_register(GIE);
  /*
  FSM loop
  */
  int test_flag = 1;
  while (1) {
    switch (system_state) {
      case IDLE:  // LPM and wait
        SIPO_shift(0x01);
        break;
      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        rng_num = 0x0F;  // Display 0 on left and blank on right
        SIPO_shift(SHIFT_BUFFER(rng_num));
        P2IES |= (BIT1 | BIT2);  // Set edge select to high-low
        P2IFG &= ~(BIT1 | BIT2);  // Clear flags
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        TA0CTL = TACLR;  // Clear Timer_A counter
        TACCR1 = COMMAND_TIMEOUT;  // Set timout countdown
        TACCTL1 = CCIE;  // Enable Timer_A interrupt 1 (Timeout)
        while (1)  // Select command
        {
          ;  // Wait until an interrupt happens
        }
        break;
      case MENU:  // Configure RANDOM number range, mode (COUNT or RANDOM, etc.)
       break;
      case COUNT:  // Increment counter and display for some time
        break;
      case RANDOM:  // Make "random" number in set range and display for some time
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
        if ((P2IES & BIT1) == 0) {  // If rising transition
          system_state = COMMAND;
        }
        else {
          TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
          TACCTL0 = CCIE;  // Enable Timer_A interrupt 0
        }
        P2IE &= ~BIT1;  // Disable button interrupt
        P2IES ^= BIT1;  // Switch relevant edge
        P2IFG &= ~BIT1;  // Clear any accumulated button flags
        break;
      case COMMAND:
        if (P2IFG & BIT1) {  // If P2.1 triggered
            active_pin = BIT1;
            P2IE &= ~BIT2; // Disable P2.2
        }
        else if (P2IFG & BIT2) {  // Do the opposite
            active_pin = BIT2;
            P2IE &= ~BIT1;
        }
        if ((P2IES & active_pin) == 0) { // If rising transition
            // Perform COMMAND actions here if needed
            // e.g., if (active_pin == BIT2) shift_something();
        }
        else {
            TACCR0 = TAR + DEBOUCE_COUNT; // Schedule debounce interrupt
            TACCTL0 = CCIE; // Enable Timer_A interrupt 0
        }
        P2IE &= ~active_pin;   // Disable interrupt for active pin
        P2IES ^= active_pin;   // Switch relevant edge
        P2IFG &= ~active_pin;  // Clear flag for active pin
        break;
    }
    
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {  // For button debounce (highest priority)
  TACCTL0 &= ~CCIE;      // Disable Timer_A interrupt
  switch (system_state) {
    case IDLE:
      TACCTL0 &= ~CCIE;  // Disable Timer_A interrupt
      P2IFG &= ~BIT1;  // Clear button interrupt flag
      P2IE |= BIT1;  // Enable button interrupt
      break;
    case COMMAND:
      P2IFG &= ~active_pin;  // Clear flag for the specific button that was pressed
      P2IE |= active_pin;    // Re-enable interrupt for that specific button
    break;
  }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {  // Timeout
  if (TAIV == TAIV_2) {
    system_state = IDLE;
  }
  TA0CCTL1 &= ~CCIE;
}
