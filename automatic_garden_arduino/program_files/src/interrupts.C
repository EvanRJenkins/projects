#include "interrupts.h"
#include <avr/interrupt.h>

/* Global variables */
volatile uint8_t g_isrFlags = 0;          // Bitfield holding all ISR flags
volatile uint32_t g_msCounter = 0;        // ms counter
volatile uint32_t g_pulseDuration;        // Difference between most recent two pulses

/* ISR Handler Macros */
ISR (PCINT0_vect) 
{               // Handler for pin change at PB0 (D8)
  g_isrFlags |= PCINT0;
}

ISR (ADC_vect)  // Handler for ADC conversion complete
{
  g_isrFlags |= ADC;  // Flag new data availiable
}

ISR (TIMER0_OVF_vect)  // Handler for Timer/Counter0 Overflow
{
  g_isrFlags |= WTRDONE;
}

ISR(TIMER1_COMPA_vect)  // Handler for Timer/Counter1 compare (ms)
{
  g_msCounter++;
}

ISR(INT0_vect)  // Handler for flow sensor pulse
{
  static bool waitForSecondPulse = false;
  static uint32_t firstPulseTime;  // msCounter value at first pulse
  if (!waitForSecondPulse)  // First pulse
  {
    firstPulseTime = g_msCounter;  // Record start time
    waitForSecondPulse = true;
  } 
  
  else  // Second Pulse
  {
    g_pulseDuration = g_msCounter - firstPulseTime; // Store duration
    g_isrFlags |= NEW_PULSE_DATA;  // Set flag that data is ready
    waitForSecondPulse = false;  // Reset for the next pair of pulses
  }
}
