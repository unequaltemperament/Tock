#ifndef SENSOR_HEADER
#define SENSOR_HEADER

#include "debugSettings.h"
#include "boardConfigs/config.h"
#include "typeDefs.h"
#include <Adafruit_CAP1188.h>

extern uint8_t touched;
extern unsigned long currentMillis;
extern Adafruit_CAP1188 cap;

void initSensors();

void getSensorInput();


#endif //header guard