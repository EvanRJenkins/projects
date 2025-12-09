#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Definitions
*/
#define DEBOUCE_COUNT 35  // Placeholder value
#define LONG_PRESS_CYCLES 25000  // For activating MENU state
#define TAIV_2 2  // TACCR1 CCIFG
#define DEBOUNCE_DONE ((P2IES & active_pin) == 0)
/*
Menu flag bitsmasks
*/
#define COUNTER_RESET   (1 << 0)
#define RANDOM_RANGE_2  (1 << 1)
#define RANDOM_RANGE_6  (1 << 3)
#define RANDOM_RANGE_8  (1 << 4)
#define RANDOM_RANGE_10 (1 << 5)
#define RANDOM_RANGE_12 (1 << 6)
#define SINGLE_DIGIT_RANGE (menu_flags & (RANDOM_RANGE_2 | RANDOM_RANGE_4 | RANDOM_RANGE_8))
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
volatile unsigned char menu_count = 8;
/*
Helper functions
*/
unsigned char hex_MOD_10(unsigned char input) {  // Return %10 on 2-digit hex digits in char
  unsigned char upper = (input & 0xF0) >> 4;
  unsigned char lower = (input & 0x0F);
  upper = upper % 10;
  lower = lower % 10;
  return (upper << 4) | lower;
}
unsigned char hex_to_BCD(unsigned char input) {
  if (input > 99) {  // limit input to 99 (remove letters)
    input = 99; 
  }
  unsigned char tens = input / 10;
  unsigned char ones = input % 10;
  return (tens << 4) | ones;
}
unsigned char BCD_mod(unsigned char input) {
  unsigned char upper = (input & 0xF0) >> 4;
  unsigned char lower = (input & 0x0F);
  unsigned char total_val = (upper * 10) + lower;
  unsigned char result_int = 0;
  switch (menu_flags) {  // Get random number in flag range
    case RANDOM_RANGE_2:
      result_int = (total_val % 2) + 1;
      break;
    case RANDOM_RANGE_4:
      result_int = (total_val % 4) + 1;
      break;
    case RANDOM_RANGE_6:
      result_int = (total_val % 6) + 1;
      break;
    case RANDOM_RANGE_8:
      result_int = (total_val % 8) + 1;
      break;
    case RANDOM_RANGE_10:
      result_int = (total_val % 10) + 1;
      break;
    case RANDOM_RANGE_12:
      result_int = (total_val % 12) + 1;
      break;
    case RANDOM_RANGE_20:
      result_int = (total_val % 20) + 1;
      break;
    default:
      result_int = total_val; // Pass if no flag
      break;
  }
  unsigned char tens = result_int / 10;  // 12 / 10 = 1
  unsigned char ones = result_int % 10;  // 12 % 10 = 2
  return (tens << 4) | ones;  // Shift tens to upper (0x10), OR with ones (0x02)
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
        SIPO_shift(SHIFT_BUFFER(0xF8));  // Both displays off
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        SIPO_shift(SHIFT_BUFFER(0x00));
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COUNT:  // Make "random" number in set range and display for some time
        if ((menu_flags & 0x01) == 1) {  // If COUNTER_RESET flag 
          current_count = 0;  // Reset count
          menu_flags &= 0xFE;  // Lower COUNTER_RESET flag
        }
        SIPO_shift(SHIFT_BUFFER(hex_to_BCD(current_count)));
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case RANDOM:  // Increment counter and display for some time
        rng_num = BCD_mod(hex_MOD_10(rng_num));
        if (SINGLE_DIGIT_RANGE) {
        display_scramble_1_digit();
        }
        else {
          display_scramble_2_digits();
        }
        SIPO_shift(SHIFT_BUFFER(rng_num));
        __delay_cycles(30000);  // Limit display time
        system_state = IDLE;
        break;
      
      case MENU:  // Switch random range or clear counter
        if (menu_count == 8) {  // Flash alternating displays to indicate MENU state
        SIPO_shift(SHIFT_BUFFER(0x88));
        __delay_cycles(2000);
        SIPO_shift(SHIFT_BUFFER(0xFF));
        __delay_cycles(2000);
        SIPO_shift(SHIFT_BUFFER(0x88));
        __delay_cycles(2000);
        SIPO_shift(SHIFT_BUFFER(0xFF));
        __delay_cycles(2000);
        }
        else {
          SIPO_shift((menu_count));  // Display on left to indicate menu mode
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
            current_count += 1;
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
          if (active_pin == BIT2) {  // If count button
            if (menu_count < 7) {  // Max settings count
              ++menu_count;
            }
            else {
              menu_count = 0;  // Cycle to beginning of count range
            }
            system_state = MENU;  // Stay in MENU
          }
          else {  // If return to IDLE button
            menu_flags &= (0x01);  // Ensure all count settings are off
            menu_flags |= (1 << menu_count);  // Set new count range if selected
            menu_count = 8;  // Reset menu_count to default
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
      menu_count = 8;  // Ensure menu_count starts at reset condition
      debounce_high_low_active_pin(); // Override rising edge interrupt
      __bic_SR_register_on_exit(LPM3_bits);
    }
    else {  // Else in COMMAND or COUNT and timed out, go to IDLE
      system_state = IDLE;
    }
  }
}