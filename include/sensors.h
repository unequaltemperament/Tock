#ifndef SENSOR_HEADER
#define SENSOR_HEADER

#include "debugSettings.h"
#include "typeDefs.h"
#include <Adafruit_CAP1188.h>
#include <cppQueue.h>

extern uint8_t touched;
extern unsigned long currentMillis;
extern Adafruit_CAP1188 cap;
extern cppQueue buttonQueue;
TockTimer generateTockTimer(TimerStatus status = WORK, long initialTimeInSeconds = 3600);
inline char buttonMap[] = {1, 0, 4, 3, 2, 7, 6, 5};

void initSensors();

void getSensorInput();

bool processButtonQueue(cppQueue& pushTo);


#endif //header guard