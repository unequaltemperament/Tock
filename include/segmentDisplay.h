#ifndef SEGMENT_HEADER
#define SEGMENT_HEADER

#include "debugSettings.h"
#include "Arduino.h"
#include "cppQueue.h"
#include <RGBDigitV2.h>
#include "typeDefs.h"
#include "progressBar.h"

class SegmentDisplay : public RGBDigit
{

public:

  SegmentDisplay(int numDigits, int digitsPin, ProgressBar *bar, TockTimer &timer, cppQueue &queue);
  
  int _numDigits;
  unsigned long updatedAt = 0;
  const unsigned long expireBlinkIntervalInMs = 420;
  TockTimer &currentTimer;
  cppQueue &_queue;
  ProgressBar *progressBar = NULL;

  char digitStringBuffer[6] = {};

  void formatOutputText(unsigned long b);

  void drawBuffertoDigits(unsigned long b);

  void update(bool forceUpdate = false);

  void forceUpdate();

  void expireBlink(unsigned long currentMillis);

// private:
//  long roundUp(long numToRound, long multiple);
};

#endif