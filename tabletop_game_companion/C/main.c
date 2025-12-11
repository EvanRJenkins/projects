#include <msp430g2452.h>
#include "_74AHC164.h"
#include "init.h"
/*
Definitions
*/
#define DEBOUCE_COUNT 50  // Debounce timer cycles
#define LONG_PRESS_CYCLES 5000  // For activating MENU state
#define TIMEOUT_CYCLES 20000  // For timeout condition
#define TAIV_2 2  // TACCR1 CCIFG
#define DEBOUNCE_DONE ((P2IES & active_pin) == 0)
/*
Bitmasks for flags1
*/
#define COUNTER_RESET_FLAG   (1 << 0)
#define RANDOM_RANGE_2_FLAG  (1 << 1)
#define RANDOM_RANGE_4_FLAG  (1 << 2)
#define RANDOM_RANGE_6_FLAG  (1 << 3)
#define RANDOM_RANGE_8_FLAG  (1 << 4)
#define RANDOM_RANGE_10_FLAG (1 << 5)
#define RANDOM_RANGE_12_FLAG (1 << 6)
#define RANDOM_RANGE_20_FLAG (1 << 7)
#define SINGLE_DIGIT_RANGE (flags1 & (RANDOM_RANGE_2_FLAG | RANDOM_RANGE_4_FLAG | RANDOM_RANGE_8_FLAG))
/*
Bitmasks for flags2
*/
#define DELAY_FLAG (1 << 0)
#define MENU_INDICATOR_FLAG (1 << 1)
/*
MACRO to switch digit order
*/
#define SHIFT_BUFFER(num) ((((num) & 0x0F) << 4) | (((num) & 0xF0) >> 4))
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
volatile unsigned char flags1 = 0x00;
volatile unsigned char flags2 = (0x00 | MENU_INDICATOR_FLAG);
volatile signed char menu_count = -1;
/*
Helper functions
*/
void set_active_pin(unsigned char pin) {
  active_pin = pin;
  if (pin == BIT1) {
    P2IE &= ~BIT2; // Disable P2.2
  }
  else {
    P2IE &= ~BIT1; // Disable P2.1
  }
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
void start_timer_debounce(void){
  TACCR0 = TAR + DEBOUCE_COUNT;  // Schedule debounce interrupt
  TACCTL0 = CCIE;  // Enable Timer_A interrupt 0
}
void start_timer_longpress(void) { 
  TACCR1 = TAR + LONG_PRESS_CYCLES;  // Start the Long Press Timer
  TA0CCTL1 = CCIE;     // Enable Timer A1 interrupt
}
void start_timer_timeout(void) {
  TACCR1 = TAR + TIMEOUT_CYCLES;  // Start the Timeout Counter
  TA0CCTL1 = CCIE;     // Enable Timer A1 interrupt
}
void timer_delay(unsigned int cycles) {
  __disable_interrupt();  // Prevent interrupt interference
  flags2 |= DELAY_FLAG;
  TACCR1 = TAR + cycles;  // Start the Long Press Timer
  TA0CCTL1 = CCIE;  // Enable Timer A1 interrupt
  __bis_SR_register(LPM3_bits + GIE);  // LPM until delay interrupt occurs
}
void SIPO_shift_safe(unsigned char shift_byte) {  // Just SIPO_shift but disables and re-enables interrupts
  __disable_interrupt();
  SIPO_shift(shift_byte);
  __bis_SR_register(GIE);
}
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
  switch (flags1) {  // Get random number in flag range
    case RANDOM_RANGE_2_FLAG:
      result_int = (total_val % 2) + 1;
      break;
    case RANDOM_RANGE_4_FLAG:
      result_int = (total_val % 4) + 1;
      break;
    case RANDOM_RANGE_6_FLAG:
      result_int = (total_val % 6) + 1;
      break;
    case RANDOM_RANGE_8_FLAG:
      result_int = (total_val % 8) + 1;
      break;
    case RANDOM_RANGE_10_FLAG:
      result_int = (total_val % 10) + 1;
      break;
    case RANDOM_RANGE_12_FLAG:
      result_int = (total_val % 12) + 1;
      break;
    case RANDOM_RANGE_20_FLAG:
      result_int = (total_val % 20) + 1;
      break;
    default:
      result_int = total_val;  // Pass if no flag
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
    SIPO_shift_safe(val);
    timer_delay(100);
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
    SIPO_shift_safe(SHIFT_BUFFER(val));
    timer_delay(100);
    val++;
    if ((val & 0x0F) > 0x09) {
      val += 0x06; 
    }
  }
}
void MENU_indicator(void) {  // Flash alternating 8 on displays
  __disable_interrupt();
  SIPO_shift(SHIFT_BUFFER(0xF8));
  timer_delay(1000);
  SIPO_shift(SHIFT_BUFFER(0x8F));
  timer_delay(1000);
  SIPO_shift(SHIFT_BUFFER(0xF8));
  timer_delay(1000);
  SIPO_shift(SHIFT_BUFFER(0x8F));
  timer_delay(1000);
  __bis_SR_register(GIE);
}
void set_timing_pin(void) {
  P2OUT |= BIT3;
}
void reset_timing_pin(void) {
  P2OUT &= ~BIT3;
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
  /*
  FSM loop
  */
  while (1) {
    switch (system_state) {
      case IDLE:  // LPM and wait to turn on
        SIPO_shift_safe(SHIFT_BUFFER(0xFF));  // Both displays off
        ready_active_pin(BIT1);
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COMMAND:  // Jump to MENU, COUNT, or RANDOM depending on user input
        SIPO_shift_safe(SHIFT_BUFFER(0x00));
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        start_timer_timeout();  // Start timeout countdown 
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case COUNT:  // Make "random" number in set range and display for some time
        TA0CCTL1 &= ~CCIE;     // Disable timeout counter
        if ((flags1 & 0x01) == 1) {  // If COUNTER_RESET_FLAG flag 
          current_count = 0;  // Reset count
          flags1 ^= COUNTER_RESET_FLAG;  // Lower COUNTER_RESET_FLAG flag
        }
        SIPO_shift_safe(SHIFT_BUFFER(hex_to_BCD(current_count)));
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        start_timer_timeout();  // Start timeout countdown 
        __bis_SR_register(LPM3_bits + GIE);
        break;

      case RANDOM:  // Increment counter and display for some time
        TA0CCTL1 &= ~CCIE;  // Disable timeout counter
        rng_num = BCD_mod(hex_MOD_10(rng_num));
        if (SINGLE_DIGIT_RANGE) {
          display_scramble_1_digit();
        }
        else {
          display_scramble_2_digits();
        }
        SIPO_shift_safe(SHIFT_BUFFER(rng_num));
        start_timer_timeout();  // Start timeout countdown
        __bis_SR_register(LPM3_bits + GIE);
        break;
      
      case MENU:  // Switch random range or clear counter
        TA0CCTL1 &= ~CCIE;   // Disable timeout counter
        if (flags2 & MENU_INDICATOR_FLAG) {  // Flash alternating displays to indicate MENU state
          P2IFG &= ~(BIT1 | BIT2);
          flags2 ^= MENU_INDICATOR_FLAG;
          MENU_indicator();
        }
        else {
          SIPO_shift_safe((menu_count));  // Display on left to indicate menu mode
        }
        P2IE |= (BIT1 | BIT2);  // Enable interrupt for P2.1 and P2.2
        P2IES |= (BIT1 | BIT2);  // Set falling edge for P2.1 and P2.2
        start_timer_timeout();  // Start timeout countdown 
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
          TA0CCTL1 &= ~CCIE;  // Stop the long-press timer
          if (system_state == IDLE) { 
             system_state = COMMAND;
          }
        }
        else {  // If falling transition (Button Press)
          start_timer_longpress();  // Start the long press timer
          start_timer_debounce();  // Start the debounce timer
        }
        debounce_high_low_active_pin();
        break;
      case COMMAND:
        TA0CCTL1 &= ~CCIE;  // Disable timeout counter
        if (DEBOUNCE_DONE) {  // If rising transition
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
          start_timer_debounce();
        }
        debounce_high_low_active_pin();
        break;
      case COUNT:
        if (DEBOUNCE_DONE) {  // If rising transition
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
          start_timer_debounce();
        }
        debounce_high_low_active_pin();
        break;
        case MENU:
        if (DEBOUNCE_DONE) {  // If rising transition
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
            flags1 &= (0x01);  // Ensure all count settings are off
            flags1 |= (1 << menu_count);  // Set new count range if selected
            menu_count = 8;  // Reset menu_count to default
            flags2 |= MENU_INDICATOR_FLAG;
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
          start_timer_debounce();
        }
        debounce_high_low_active_pin();
        break;
    }
    __bic_SR_register_on_exit(LPM3_bits);
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {  // For button debounce (highest priority)
  TACCTL0 &= ~CCIE;   // Disable Timer_A interrupt
  P2IFG &= ~active_pin;  // Clear flag for the specific button that was pressed
  P2IE |= active_pin;  // Re-enable interrupt for that specific button
}
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {  // For long-press MENU activation
  if (TAIV == TAIV_2) {
    TA0CCTL1 &= ~CCIE;  // Disable this interrupt
    if (system_state == IDLE) {  // If in IDLE and holding the button go to MENU
      system_state = MENU;
      menu_count = -1;  // Ensure menu_count starts at reset condition
      debounce_high_low_active_pin();  // Override rising edge interrupt
    }
    else if (flags2 & DELAY_FLAG) {
      flags2 ^= DELAY_FLAG;  // Lower delay_flag
    }
    else {  // End timeout
      TACCTL0 &= ~CCIE;  // Disable Timer_A interrupt
      system_state = IDLE;
    }
    _bic_SR_register_on_exit(LPM3_bits);
  }
}
