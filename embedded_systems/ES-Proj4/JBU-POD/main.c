#include <msp430.h>

/*
1,000,000 Hz / note frequency = period cycles
factor = half period cycles
Factors are rounded to their nearest integer value
*/
#define A_NOTE 2273 // ~440 Hz
#define B_NOTE 2025 // ~494 Hz
#define C_NOTE 3823 // ~262 Hz
#define D_NOTE 3405 // ~294 Hz
#define E_NOTE 3034 // ~330 Hz
#define F_NOTE 2864 // ~349 Hz
#define G_NOTE 2551 // ~392 Hz

// Function prototypes
char debounceGoingLow(volatile unsigned char *port, unsigned char pin);
char debounceGoingHigh(volatile unsigned char *port, unsigned char pin);
void playNote(volatile unsigned char *port, unsigned char pin,
              const unsigned int note);
void playNoteForDuration(volatile unsigned char *port, unsigned char pin,
                         const unsigned int note, unsigned char duration);
void myDelay(const unsigned int i);

int main(void) {
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

  P1DIR |= BIT4;  // Set P1.4 as output
  P1OUT &= ~BIT4; // Set P1.4 output low initially
  P1REN |= BIT3;  // Enable input resistor
  P1OUT |= BIT3;  // Set pull-up resistor

  const unsigned int notes[7] = {A_NOTE / 2, B_NOTE / 2, C_NOTE / 2, D_NOTE / 2,
                                 E_NOTE / 2, F_NOTE / 2, G_NOTE / 2};
  unsigned char noteIndex = 0;
  while (1) {
    if (debounceGoingLow(&P1IN, BIT3)) {
      if (noteIndex < 6) {
        ++noteIndex;
      } else {
        noteIndex = 0;
      }
      while (!debounceGoingHigh(&P1IN, BIT3)) {
        ; // Wait
      }
    }
    playNote(&P1OUT, BIT4, notes[noteIndex]); // Play note
  }

  return 0;
}

// Function definitions
char debounceGoingLow(volatile unsigned char *port, unsigned char pin) {
  if (!(*port & pin)) {
    __delay_cycles(50000);
    if (!(*port & pin)) {
      return 1;
    }
  }
  return 0;
}

char debounceGoingHigh(volatile unsigned char *port, unsigned char pin) {
  if ((*port & pin)) {
    __delay_cycles(50000);
    if ((*port & pin)) {
      return 1;
    }
  }
  return 0;
}

void playNote(volatile unsigned char *port, unsigned char pin,
              const unsigned int note) {
  unsigned int i;
  for (i = 0; i < 1000; ++i) {
    *port ^= pin;
    for (i = 0; i < note; ++i) {
      __delay_cycles(1);
    }
  }
}

void playNoteForDuration(volatile unsigned char *port, unsigned char pin,
                         const unsigned int note, unsigned char duration) {
  // Empty rn
}
