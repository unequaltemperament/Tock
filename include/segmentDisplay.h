#ifndef SEGMENT_HEADER
#define SEGMENT_HEADER

#include "debugSettings.h"
#include "Arduino.h"
#include <RGBDigitV2.h>
#include "typeDefs.h"
#include "progressBar.h"

class TimerManager;

class SegmentDisplay : public RGBDigit
{

public:

  SegmentDisplay(int numDigits, int digitsPin);
  
  int _numDigits;
  unsigned long updatedAt = 0;
  const unsigned long normalUpdateIntervalInMS = 1000;
  const unsigned long expireBlinkIntervalInMS = 420;
  TimerManager* manager = nullptr;

  void setManager(TimerManager* const m);

  char digitStringBuffer[6] = {};

  void formatOutputText(unsigned long b);

  void drawBuffertoDigits(unsigned long b);

  void update(bool forceUpdate = false);

  void forceUpdate();

  void expireBlink(unsigned long currentMillis);

// private:
//  long roundUp(long numToRound, long multiple);
};

#endif //header guard