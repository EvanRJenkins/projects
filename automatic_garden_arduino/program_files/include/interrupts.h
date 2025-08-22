#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "main.h"

/* Interrupt bitfield bits */
#define PCINT0 (1 << 0)                   // Bit 0 is PCINT0
#define ADC (1 << 1)                      // Bit 1 is ADC complete
#define WTRDONE (1 << 2)                  // Bit 2 is watering done
#define NEW_PULSE_DATA (1 << 3)           // Bit 3 is new pulse data ready

/* Extern declarations for global variables */
extern volatile uint8_t g_isrFlags;
extern volatile uint32_t g_msCounter;
extern volatile uint32_t g_pulseDuration;

#endif // INTERRUPTS_H
