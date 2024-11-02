#ifndef TYPES_HEADER
#define TYPES_HEADER

#include "debugSettings.h"

const unsigned long oneSecondInMS = 1000,
                    sensorIntervalInMS = 50;

// capacitive sensor configuration info

int sensitivity_control_register = 0x1F;
int sensitivity_control_default = 0x2F; // 0b00101111
// Bits 6-4 represent a 3-bit sensitivity value, where 0b000 is the most sensitive and 0b111 is the least (default 010)
// Bits 3-0 control the scaling and presentation of the Base Count registers, where higher values correspond to a higher
// range and lower resolution in the data. Does not affect touch sensitivity, and typically does not need to be adjusted.
// Scales by powers of 2 from 1x to 256x. 0b1000 is a max value representing 256x multiplier, all others will be truncated
//(default 1111)

// Bits:
//    7: -unused-
//    6: [DELTA_SENSE bit 2]
//    5: [DELTA_SENSE bit 1]
//    4: [DELTA_SENSE bit 0]
//    3: [BASE_SHIFT  bit 3]
//    2: [BASE_SHIFT  bit 2]
//    1: [BASE_SHIFT  bit 1]
//    0: [BASE_SHIFT  bit 0]

int repeat_rate_enable_register = 0x28;
int repeat_rate_enable_default = 0xFF; // 0b11111111
// Enables repeat rate for each sensor

// Bits:
//    7: Sensor 8 repeat enable (default 1)
//    6: Sensor 7 repeat enable (default 1)
//    5: Sensor 6 repeat enable (default 1)
//    4: Sensor 5 repeat enable (default 1)
//    3: Sensor 4 repeat enable (default 1)
//    2: Sensor 3 repeat enable (default 1)
//    1: Sensor 2 repeat enable (default 1)
//    0: Sensor 1 repeat enable (default 1)

int sensor_input_config_register = 0x22;
int sensor_input_config_default = 0xA4; // 0b10100100
// Bits 7-4 determine maximum time a sensor can be touch before automatic re-calibration occurs, increments of 280ms per
// increment in the register value, range from 560ms (0b0000) to 11200ms (0b1111), default 5600ms (0b1010)
// Bits 3-0 determine duration between interrupts assertions when auto-repeat is enabled. 35 ms resolution
// range from 35ms (0b0000) to 560ms(0b1111), default 175ms (0b0100)

// Bits:
//    7: [MAX_DUR bit 3]
//    6: [MAX_DUR bit 2]
//    5: [MAX_DUR bit 1]
//    4: [MAX_DUR bit 0]
//    3: [RPT_RATE  bit 3]
//    2: [RPT_RATE  bit 2]
//    1: [RPT_RATE  bit 1]
//    0: [RPT_RATE  bit 0]

int sensor_input_config_2_register = 0x23;
int sensor_input_config_2_default = 0x07; // 0b00000111
// Sets minimum time to detect a "press and hold" event before triggering auto repeat output at the set repeat rate
// Increments by 35ms per increment in the register value, range from 35ms (0b0000) to 560ms (0b1111), default 280ms (0b0111)

// Bits:
//    7: -unused-
//    6: -unused-
//    5: -unused-
//    4: -unused-
//    3: [M_PRESS bit 3]
//    2: [M_PRESS bit 2]
//    1: [M_PRESS bit 1]
//    0: [M_PRESS bit 0]

int config_2_register = 0x44;
int config_2_default = 0x40; // 0b01000000

// Bits:
//    7: Invert linked LED transition control (default 0)
//    6: Alert# pin polarity inversion (default 1)
//    5: Block power control (default 0)
//    4: Block LED polarity mirror control (default 0)
//    3: Show RF noise only on Noise Status bits (default 0)
//    2: Disable RF Noise filter (default 0)
//    1: -unused-
//    0: Disable release interrupts (default 0)

enum TimerStatus
{
  EXPIRE,
  WORK,
  BREAK
};

char statusType[][8] = {"expired", " work", "break"};

long TimerColor[] = {0xFF0000, 0xD9FF00, 0x2AE600};

struct TockTimer
{

  TimerStatus status;
  long remainingTimeInMS;
  long initialTimeInMS;

  TockTimer(TimerStatus initStatus = WORK, long initTimeInS = 3600)
  {
    status = initStatus;
    initialTimeInMS = initTimeInS * 1000;
    remainingTimeInMS = initialTimeInMS;
  }
};

struct Bitmap
{
  int width;
  int height;
  int cursorX;
  int cursorY;
};
#endif