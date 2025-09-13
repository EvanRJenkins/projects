#include <msp430.h>

/*
1,000,000 Hz / note frequency = period cycles
factor = half period cycles
factors are rounded to their nearest integer value
Octave 4 by default
*/
#define A_NOTE 1136       // ~440.00 Hz
#define A_SHARP_NOTE 1073 // ~466.16 Hz
#define B_NOTE 1013       // ~493.88 Hz
#define C_NOTE 1911       // ~523.25 Hz
#define C_SHARP_NOTE 1804 // ~554.37 Hz
#define D_NOTE 1703       // ~587.33 Hz
#define D_SHARP_NOTE 1607 // ~622.25 Hz
#define E_NOTE 1517       // ~659.25 Hz
#define F_NOTE 1432       // ~698.46 Hz
#define F_SHARP_NOTE 1351 // ~739.99 Hz
#define G_NOTE 1276       // ~783.99 Hz
#define G_SHARP_NOTE 1204 // ~830.61 Hz
#define REST_NOTE 0

const unsigned int championMelody[] = {
    D_SHARP_NOTE, F_NOTE,       F_SHARP_NOTE, F_NOTE,       C_SHARP_NOTE,
    REST_NOTE,    D_SHARP_NOTE, F_NOTE,       F_SHARP_NOTE, F_NOTE,
    A_SHARP_NOTE, A_NOTE,       F_NOTE,       REST_NOTE,    F_NOTE,
    D_SHARP_NOTE, C_SHARP_NOTE};

const unsigned int championDurations[] = {100, 100, 200, 100, 500, 1200,
                                          100, 100, 200, 100, 100, 500,
                                          100, 50,  200, 100, 200};

// Song struct definition
struct Song {
  const unsigned int *melody;    // Note pattern array
  const unsigned int *durations; // Note durations array
  unsigned int length;           // Num notes
};

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

  const struct Song championSong = {championMelody, championDurations,
                                    sizeof(championMelody) /
                                        sizeof(championMelody[0])};
  unsigned int i = 0;
  playNote(&P1OUT, BIT4, C_NOTE);
  __delay_cycles(500000);
  for (i = 0; i < championSong.length; i++) {
    playNoteForDuration(&P1OUT, BIT4, championSong.melody[i],
                        championSong.durations[i]);
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
  unsigned long numCycles;
  unsigned int j;
  unsigned long i;

  if (note == REST_NOTE) { // Handle rest note case
    __delay_cycles(50000);
    return;
  }

  numCycles = 50000UL / note; // Scale duration for each note to ~50 (ms)

  *port &= ~pin; // start LOW

  for (i = numCycles; i > 0; --i) {
    *port ^= pin; // Toggle
    for (j = note / 2; j > 0; --j) {
      __delay_cycles(1);
    }
    *port ^= pin; // Toggle
    for (j = note / 2; j > 0; --j) {
      __delay_cycles(1);
    }
  }

  *port &= ~pin; // End LOW
}

void playNoteForDuration(volatile unsigned char *port, unsigned char pin,
                         const unsigned int note, unsigned char duration) {
  unsigned long numCycles;
  unsigned int j;
  unsigned long i;

  if (note == REST_NOTE) { // Handle rest note case
    unsigned int k;
    for (k = 0; k < duration; k++) {
      __delay_cycles(1000); // 1 ms per loop at 1 MHz
    }
    return;
  }

  numCycles = (duration * 1000) /
              note; // Scale duration for each note to ~duration (ms)

  *port &= ~pin; // start LOW

  for (i = numCycles; i > 0; --i) {
    *port ^= pin; // Toggle
    for (j = note / 2; j > 0; --j) {
      __delay_cycles(1);
    }
    *port ^= pin; // Toggle
    for (j = note / 2; j > 0; --j) {
      __delay_cycles(1);
    }
  }

  *port &= ~pin; // End LOW
}
