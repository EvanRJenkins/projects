#include <msp430.h>

// SONG: CHAMPION BY KANYE WEST

/*
1,000,000 Hz / note frequency = period cycles
factor = half period cycles
factors are rounded to their nearest integer value
Octave 4 by default
*/
#define C_NOTE        1911
#define C_SHARP_NOTE  1804
#define D_NOTE        1703
#define D_SHARP_NOTE  1607
#define E_NOTE        1517
#define F_NOTE        1432
#define F_SHARP_NOTE  1352
#define G_NOTE        1276
#define G_SHARP_NOTE  1204
#define A_NOTE        1136
#define A_SHARP_NOTE  1075
#define B_NOTE        1013
#define A_SHARP_3_NOTE 2145


// Song definition
const unsigned int championMelody[] = {
    A_SHARP_3_NOTE, F_NOTE, C_SHARP_NOTE, F_SHARP_NOTE, D_SHARP_NOTE,
    F_NOTE, F_SHARP_NOTE, F_NOTE, C_SHARP_NOTE, D_SHARP_NOTE,
    F_NOTE, F_SHARP_NOTE, F_NOTE, A_SHARP_NOTE, G_SHARP_NOTE,
    F_NOTE, F_NOTE, D_SHARP_NOTE, E_NOTE, G_NOTE
};

const unsigned int championDurations[] = {
    180, 80, 200, 200, 40, 40, 60,
40, 160, 40, 40, 60, 40, 60,
100, 40, 60, 200, 100, 100
};

// State enum
enum E_state
{
    IDLE,
    PLAY,
    PAUSE
};

// Function prototypes
char debounceGoingLow( volatile unsigned char *port, unsigned char pin );
char debounceGoingHigh( volatile unsigned char *port, unsigned char pin );
void playNote( volatile unsigned char *port, unsigned char pin,
               const unsigned int note );
void playNoteForDuration( volatile unsigned char *port, unsigned char pin,
                          const unsigned int note, unsigned int duration );
void myDelay( const unsigned int i );

int main( void )
{
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

    P1DIR |= BIT4;  // Set P1.4 as output
    P1OUT &= ~BIT4; // Set P1.4 output low initially
    P1REN |= BIT3;  // Enable input resistor
    P1OUT |= BIT3;  // Set pull-up resistor

    enum E_state state = IDLE;       // Holds current state
    unsigned char i = 0;             // Generic loop counter
    unsigned char currentNote = 0;   // For saving not paused at
    unsigned int resetCount = 30000; // ~3s Delay
    // FSM loop
    while ( 1 )
    {
        switch ( state )
        {

        case IDLE: // Song not playing and not paused
            while ( debounceGoingLow( &P1IN, BIT3 ) )
            {
                ;
            }
            while ( !debounceGoingLow( &P1IN, BIT3 ) )
            {
                ;
            }
            while ( !debounceGoingHigh( &P1IN, BIT3 ) )
            {
                ;
            }
            state = PLAY;
            break;

        case PLAY: // Song playing unless paused or done

            for ( i = currentNote; i < 20; i++ )
            {
                playNoteForDuration( &P1OUT, BIT4, championMelody[i],
                                     championDurations[i] );
                if ( debounceGoingLow( &P1IN, BIT3 ) )
                {
                    currentNote = i + 1;
                    break;
                }
            }

            while ( !debounceGoingHigh( &P1IN, BIT3 ) )
            {
                if ( resetCount == 0 )
                {
                    i = 0;
                    currentNote = 0;
                    resetCount = 0;
                    state = IDLE;
                }
                ++resetCount;
            }
            state = PAUSE;
            break;

        case PAUSE: // Song paused until resumed or reset
            while ( !debounceGoingLow( &P1IN, BIT3 ) )
            {
                ;
            }
            while ( !debounceGoingHigh( &P1IN, BIT3 ) )
            {
                if ( resetCount == 0 )
                {
                    i = 0;
                    currentNote = 0;
                    resetCount = 0;
                    state = IDLE;
                }
                ++resetCount;
            }
            state = PLAY;
            break;
            
        }
    }

    return 0;
}

// Function definitions
char debounceGoingLow( volatile unsigned char *port, unsigned char pin )
{
    if ( !( *port & pin ) )
    {
        __delay_cycles( 10000 );
        if ( !( *port & pin ) )
        {
            return 1;
        }
    }
    return 0;
}

char debounceGoingHigh( volatile unsigned char *port, unsigned char pin )
{
    if ( ( *port & pin ) )
    {
        __delay_cycles( 10000 );
        if ( ( *port & pin ) )
        {
            return 1;
        }
    }
    return 0;
}

void playNote( volatile unsigned char *port, unsigned char pin,
               const unsigned int note )
{
    unsigned long numCycles;
    unsigned int j;
    unsigned long i;

    numCycles = 50000UL / note; // Scale duration for each note to ~50 (ms)

    *port &= ~pin; // Start LOW

    for ( i = numCycles; i > 0; --i )
    {
        *port ^= pin; // Toggle
        for ( j = note / 2; j > 0; --j )
        {
            __delay_cycles( 1 );
        }
        *port ^= pin; // Toggle
        for ( j = note / 2; j > 0; --j )
        {
            __delay_cycles( 1 );
        }
    }

    *port &= ~pin; // End LOW
}

void playNoteForDuration( volatile unsigned char *port, unsigned char pin,
                          const unsigned int note, unsigned int duration )
{
    unsigned long totalCycles;
    unsigned long numCycles;
    unsigned int j;
    unsigned long i;

    totalCycles = ( unsigned long )duration * 1000UL; // 1 MHz

    numCycles = totalCycles / note;

    *port &= ~pin; // Start LOW

    for ( i = numCycles; i > 0; --i )
    {
        *port ^= pin; // Toggle
        for ( j = note / 4; j > 0; --j )
        {
            __delay_cycles( 1 );
        }
        *port ^= pin; // Toggle
        for ( j = note / 4; j > 0; --j )
        {
            __delay_cycles( 1 );
        }
    }

    *port &= ~pin; // End LOW
}
