#ifndef PLANT_CONFIG_H
#define PLANT_CONFIG_H

#include "main.h"

/* Adjust these constants for your physical setup */
#define PLANTS_IN_GARDEN 2                    // Adjust depending on # of plants currently in garden
#define FLOW_RATE 562                         // Volumetric flow rate of pump in mL/s
#define FLOW_CHECK_SETPOINT (FLOW_RATE * 2)   // Minimum time (ms) between pulses for Flow Check to pass
#define FLOW_TIMEOUT_MS 5000                  // 5000ms or 5 second timeout

/* Extern declarations for global variables */
extern Plant plantList[PLANTS_IN_GARDEN];

/* Function Prototypes */
void definePlants();

#endif // PLANT_CONFIG_H
