#ifndef SENSOR_HEADER
#define SENSOR_HEADER

#include "typeDefs.h"
#include <Adafruit_CAP1188.h>

class cppQueue;

bool queueTimer(cppQueue &q, TimerStatus status, long initialTimeInSeconds);

inline char buttonMap[] = {1, 0, 4, 3, 2, 7, 6, 5};

void initSensors();

int getAmbientBrightness();
void getSensorInput();

bool processButtonQueue(cppQueue& pushTo);


#endif //header guard