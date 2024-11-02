#ifndef DEBUGSETTINGS_HEADER
#define DEBUGSETTINGS_HEADER

#include "HardwareSerial.h"

#define DEBUG 1

#if DEBUG
#define debug(...) Serial.print(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#else
#define debug(x)
#define debugln(...)
#endif

#endif