#ifndef SEGMENT_HEADER
#define SEGMENT_HEADER

#include "debugSettings.h"
#include <RGBDigitV2.h>
#include "typeDefs.h"

class TimerManager;
extern unsigned long currentMillis;

class SegmentDisplay : public RGBDigit
{

public:
  int _numDigits;
  bool enabled = false;
  unsigned long updatedAt = 0;
  const unsigned long normalUpdateIntervalInMS = 1000;
  const unsigned long expireBlinkIntervalInMS = 420;
  bool expireLEDBlinkOn = true;

  TimerManager *manager = nullptr;

  SegmentDisplay(int numDigits, int digitsPin);

  void setManager(TimerManager *const m);

  void init();
  char digitStringBuffer[6] = {};

  void formatOutputText(unsigned long b);

  void drawBuffertoDigits(unsigned long b);
  void drawBuffertoDigits(const char *b);

  void update(bool forceUpdate = false);

  void forceUpdate();

  void expireBlink();
};

#endif // header guard