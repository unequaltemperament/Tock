#ifndef PROGRESS_HEADER
#define PROGRESS_HEADER

#include "debugSettings.h"
#include "HardwareSerial.h"
#include <Adafruit_NeoPixel.h>

struct TockTimer;

class ProgressBar : public Adafruit_NeoPixel
{

public:
  ProgressBar(int num_leds, int led_pin)
      : Adafruit_NeoPixel(num_leds, led_pin, NEO_GRB)
  {
    _num_leds = num_leds;
  };

  int _num_leds;
  unsigned long updatedAt = 0;
  unsigned long lightIntervalInMs = 0;
  const int partialSteps = 64;
  TockTimer *currentTimer = NULL;

  void update(bool forceUpdate = false)
  {
    const unsigned long currentMillis = millis();

    if ((currentMillis - updatedAt >= lightIntervalInMs && currentTimer->remainingTimeInMS > 0) || forceUpdate)
    {
      if (!forceUpdate)
      {
        updatedAt = currentMillis;
      }
      float fullLEDsInt;
      float partial = modff(((float)currentTimer->remainingTimeInMS * _num_leds / currentTimer->initialTimeInMS), &fullLEDsInt);
      fullLEDsInt = _num_leds - fullLEDsInt;
      for (int i = 0; i < (int)fullLEDsInt; i++)
      {
        setPixelColor(getMappedLED(i), TimerColor[currentTimer->status]);
      }
      if ((int)fullLEDsInt < _num_leds && partial > 0)
      {
        setPixelColor(getMappedLED((int)fullLEDsInt), getDimmedColor(TimerColor[currentTimer->status], 1 - partial));
      }
      show();
    }
  }

  void forceUpdate()
  {
    update(true);
  }

private:
  // Progress bar LED strip is split in half and the second half is reversed,
  // and LEDs are interleaved between the two strips, starting with the upper one
  //
  // getMappedLED maps timer progress in "full LED units" to physical LED numbers
  // and it's wired from the right side, so the "first led" is _num_leds / 2
  // assume 10 leds:
  //
  //  5   4   3   2   1
  //    6   7   8   9   10
  //
  //  Lighting order is:
  //    5 6 4 7 3 8 2 9 1 10

  int getMappedLED(int realID)
  {
    realID = _num_leds - realID - 1;
    return (realID % 2 == 0) ? (realID / 2) : (_num_leds - (realID / 2) - 1);
  }

  // TODO: keep working on this....
  // needs a gamma map or something?
  // green overwhelms when all three colors are scaled equally
  uint32_t getDimmedColor(uint32_t color, float dimPercentage)
  {
   
    uint32_t r = (color >> 16 & 0xFF);
    uint32_t g = (color >> 8 & 0xFF);
    uint32_t b = (color & 0xFF);

    r = (uint32_t)r * dimPercentage;
    g = (uint32_t)g * dimPercentage;
    b = (uint32_t)b * dimPercentage;

    return (r << 16) | (g << 8) | b;

  }
};

#endif