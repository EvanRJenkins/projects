#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Definitions
*/
#define DEBOUCE_COUNT 100  // Placeholder value
#define LONG_PRESS_CYCLES 25000  // For activating MENU state
#define TAIV_2 2  // TACCR1 CCIFG
#define DEBOUNCE_DONE ((P2IES & active_pin) == 0)
/*
Menu flag bitsmasks
*/
#define RANDOM_RANGE_20 (1 << 7)
#define RANDOM_RANGE_12 (1 << 6)
#define RANDOM_RANGE_10 (1 << 5)
#define RANDOM_RANGE_8  (1 << 4)
#define RANDOM_RANGE_6  (1 << 3)
#define RANDOM_RANGE_4  (1 << 2)
#define RANDOM_RANGE_2  (1 << 1)
#define COUNTER_RESET   (1 << 0)
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
  RANDOM,
  MENU
} state_t;
/*
Global variable instantiations
*/
volatile state_t system_state = IDLE;
volatile unsigned char rng_num = 0x00;
volatile unsigned char active_pin = 0;
volatile unsigned char current_count = 0;
volatile unsigned char menu_flags = 0x00;
volatile unsigned char menu_count = 0;
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
void schedule_debounce(void){
  TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
  TACCTL0 = CCIE;  // Enable Timer_A interrupt 0
}
void ready_active_pin(unsigned char pin) {
  active_pin = pin;  // Global
  P2IES |= active_pin;  // Set edge select to high-low
  P2IFG &= ~active_pin;  // Clear flag
  P2IE |= active_pin;  // Enable interrupt for P2.1
}
void debounce_high_low_active_pin() {
  P2IE &= ~active_pin;  // Disable button interrupt
  P2IES ^= active_pin;  // Switch relevant edge
  P2IFG &= ~active_pin;  // Clear any accumulated button flags
}
void set_active_pin(unsigned char pin) {
  active_pin = pin;
  if (pin == BIT1) {
    P2IE &= ~BIT2; // Disable P2.2
  }
  else {
    P2IE &= ~BIT1; // Disable P2.1
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
  FSM loop
  */
  while (1) {
    switch (system_state) {
      case IDLE:  // LPM and wait to turn on
        ready_active_pin(BIT1);
        SIPO_shift(SHIFT_BUFFER(0xF8));
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
        __delay_cycles(10000);
        system_state = IDLE;
        break;
      
      case MENU:  // Switch random range or clear counter
        if (menu_count == 0) {
        SIPO_shift(SHIFT_BUFFER(0x99));
        }
        else {
          SIPO_shift(menu_count);
        }
        while ((P2IN & active_pin) == 0) {  // Wait through rising edge to skip debounce
          ;
        }
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        __bis_SR_register(LPM3_bits);
        break;
   }
  }
}
/*
Interrupts
*/
#pragma vector = PORT2_VECTOR
__interrupt void Port_2_ISR(void) {
    switch (system_state) {
      case IDLE:
        if (DEBOUNCE_DONE) {  // If rising transition (Button Release)
          // Stop the long-press timer immediately
          TA0CCTL1 &= ~CCIE;  // Stop the long-press timer
          if (system_state == IDLE) { 
             system_state = COMMAND;
          }
        }
        else { // If falling transition (Button Press)
          schedule_debounce();
          TACCR1 = TAR + LONG_PRESS_CYCLES;  // Start the Long Press Timer
          TA0CCTL1 = CCIE;     // Enable Timer A1 interrupt
        }
        debounce_high_low_active_pin();
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
            set_active_pin(BIT1);
          }
          else {  // Do the opposite
            set_active_pin(BIT2);
          }
          schedule_debounce();
        }
        debounce_high_low_active_pin();
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
          if (P2IFG & BIT1) {  // If P2.1 triggered
            set_active_pin(BIT1);
          }
          else {  // Do the opposite
            set_active_pin(BIT2);
          }
          schedule_debounce();
        }
        debounce_high_low_active_pin();
        break;

        case MENU:
        if (DEBOUNCE_DONE) { // If rising transition
          if (active_pin == BIT2) {
            menu_count +=1;
            system_state = MENU;
          }
          else {
            menu_flags = (1 << menu_count);
            menu_count = 0;
            system_state = IDLE;
          }
        }
        else {
          if (P2IFG & BIT1) {  // If P2.1 triggered
            set_active_pin(BIT1);
          }
          else {  // Do the opposite
            set_active_pin(BIT2);
          }
          schedule_debounce();
        }
        debounce_high_low_active_pin();
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
__interrupt void Timer_A1_ISR(void) {
  if (TAIV == TAIV_2) {
    TA0CCTL1 &= ~CCIE; // Disable this interrupt
    if (system_state == IDLE) {  // If in IDLE and holding the button go to MENU
      system_state = MENU;
      __bic_SR_register_on_exit(LPM3_bits);
    }
    else {  // Else in COMMAND or COUNT and timed out, go to IDLE
      system_state = IDLE;
    }
  }
}