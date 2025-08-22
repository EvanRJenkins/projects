#include "plant_config.h"
#include "hal.h"
#include <string.h>

/* Global variable definitions for this module */
Plant plantList[PLANTS_IN_GARDEN];  // Array of all plants

/* Plants */
void definePlants() 
{
  strcpy(plantList[0].species, "hibiscus");     // Define plantList[0] as "hibiscus"
  plantList[0].sensorPin = AI0;                 // Moisture sensor input pin
  
  strcpy(plantList[1].species, "marigold");     // Define plantList[1] as "marigold"
  plantList[1].sensorPin = AI1;                 // Moisture sensor input pin
  
  // Add any additional plants here
}
