#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

/* Type Definitions */
typedef enum State  // Enum for state machine
{
  IDLE,                             // Power down, only exit via interrupt
  LOG_PRE,                          // Record moisture of currentPlantIndex Plant
  WAIT_FOR_ADC,                     // Wait for new data, increment currentPlantIndex, go to LOG_PRE
  START_WATERING,                   // Send water to all plants
  WAIT_FOR_FLOW,                    // Delay until flow checkpoint is reached, ERROR after timeout
  WATERING,                         // Water flowing to plants
  STOP_WATERING,                    // Stop watering, transition to LOG_POST
  LOG_POST,                         // Log after watering
  ERROR                             // System fault, block all tasks until error ack
} state_t;

typedef struct  // Holds key characteristics of an individual plant
{
  unsigned char species[10];      // String holding species name
  uint8_t sensorPin;              // Analog Input Pin for Plant's moisture sensor
  uint16_t moistureLevel;         // Most recently logged moisture level
} Plant;

#endif // MAIN_H
