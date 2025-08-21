#ifndef PROGRESS_HEADER
#define PROGRESS_HEADER

#include "debugSettings.h"
#include <Adafruit_NeoPixel.h>
#include "typeDefs.h"

struct TockTimer;
class TimerManager;
extern unsigned long currentMillis;

class ProgressBar : public Adafruit_NeoPixel
{

public:
  ProgressBar(int num_leds, int led_pin);

  int _num_leds;
  bool enabled = false;
  unsigned long updatedAt = 0;
  unsigned long lightIntervalInMs = 0;
  const int partialSteps = 128;
  bool expireLEDBlinkOn = true;
  TimerManager *manager = nullptr;

  void update(bool forceUpdate = false);

  void init();
  void forceUpdate();
  void expireBlink();
  void setManager(TimerManager *const manager);

private:
  bool lightFromWiredEnd = 0;
  const unsigned long expireBlinkIntervalInMS = 420;
  int getMappedLED(int realID);
  uint32_t getDimmedColor(uint32_t color, float dimPercentage);
};

#endif // header guard