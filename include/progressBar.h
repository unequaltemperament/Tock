#ifndef PROGRESS_HEADER
#define PROGRESS_HEADER

#include "debugSettings.h"
#include <Adafruit_NeoPixel.h>
#include "typeDefs.h"

struct TockTimer;

class ProgressBar : public Adafruit_NeoPixel
{

public:
  ProgressBar(int num_leds, int led_pin, TockTimer timer);

  int _num_leds;
  unsigned long updatedAt = 0;
  unsigned long lightIntervalInMs = 0;
  const int partialSteps = 64;
  TockTimer &currentTimer;

  void update(bool forceUpdate = false);

  void forceUpdate();

private:
  int getMappedLED(int realID);
  uint32_t getDimmedColor(uint32_t color, float dimPercentage);
};

#endif