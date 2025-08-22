#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"
#include "plant_config.h"
#include "hal.h"
#include "interrupts.h"

/* Global variables */
state_t currentState;                 // Current system state
uint8_t currentPlantIndex;            // Index value of current plant during log process
uint32_t g_timeoutStart;

/* Startup functions */
state_t sysInit() 
{
  regConfig();
  definePlants();
  //if something goes wrong, return ERROR;
  sei();
  return IDLE;
}

/* Main */
int main(void) 
{
  currentState = sysInit();  // sysInit() returns a the startup state
  while (1)  // State machine loop
  {
    switch (currentState) {   // State machine switch
      case IDLE:  
        if (g_isrFlags & PCINT0)  // Start State sequence if PCINT0 flag
        {
          currentState = LOG_PRE;
          g_isrFlags &= ~PCINT0;
        }
        else 
        {
          // sleep();                   // Enter power down mode until PCINT0
        }
        break;

      case LOG_PRE:
        if (currentPlantIndex < PLANTS_IN_GARDEN)  // Two-state loop until all plants logged
        {
          currentState = adcRecord(plantList[currentPlantIndex].sensorPin);  // Returns WAIT_FOR_ADC on success
        }
        else  // Logging complete, start watering
        {
          currentState = START_WATERING;
          currentPlantIndex = 0;  // For reuse in LOG_POST)
        }
        break;

      case WAIT_FOR_ADC:
        if (g_isrFlags & ADC)
        {
          plantList[currentPlantIndex].moistureLevel = ADCW;  // Log moistureLevel
          ++currentPlantIndex;
          g_isrFlags &= ~ADC;
          if (!(g_isrFlags & WTRDONE))  // Go back to current log state
          {
            currentState = LOG_PRE;
          }
          else {
            currentState = LOG_POST;
          }
        }
        break;  // Stay here until ADC complete interrupt

      case START_WATERING:
        g_timeoutStart = g_msCounter;  // Record start time for timeout comp
        currentState = startPump();  // Returns WAIT_FOR_FLOW on success
        break;

      case WAIT_FOR_FLOW:
        if (g_isrFlags & NEW_PULSE_DATA) 
        {
          g_isrFlags &= ~NEW_PULSE_DATA;  // Clear flag
          if (g_pulseDuration < FLOW_CHECK_SETPOINT)  // Is water flowing?
          {
            currentState = WATERING;  // Start counting watering time
          }                                       
        }
        else if ((g_msCounter - g_timeoutStart) > FLOW_TIMEOUT_MS)  // Timeout check
        {
          stopPump();  // For safety
          currentState = ERROR;
        }
        break;

      case WATERING:
        if (!(TCCR0B & ((1 << CS02) | (1 << CS01) | (1 << CS00))))  // If watering timer off
        {
          TCCR0B |= (1 << CS00);  // Select I/Oclk for Timer/Counter0 (Start it)
        }
        if (g_isrFlags & WTRDONE)  // Timer overflow sets this flag
        {
          currentState = STOP_WATERING;
        }
        break;

      case STOP_WATERING:
        currentState = stopPump();  // Returns LOG_POST on success
        TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));  // Watering timer off
        TCNT0 = 0;  // Clear watering timer
        break;
      
      case LOG_POST:
        if (currentPlantIndex < PLANTS_IN_GARDEN)  // Same two-state loop as LOG_PRE
        {
          currentState = adcRecord(plantList[currentPlantIndex].sensorPin);
        }
        else  // Logging done, housekeeping and go to IDLE
        {
          g_isrFlags = 0x0;  // Clear flags
          currentPlantIndex = 0;
          currentState = IDLE;
        }
        break;

      case ERROR:
        // perform any fail safety actions
        // wait for error to clear
        // update state  
        break;
    }
  }
}
