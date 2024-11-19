#ifndef SEGMENT_HEADER
#define SEGMENT_HEADER

#include "debugSettings.h"
#include "Arduino.h"
#include "cppQueue.h"
#include <RGBDigitV2.h>
#include "typeDefs.h"

class ProgressBar;

class SegmentDisplay : public RGBDigit
{

public:
  int _numDigits;
  unsigned long updatedAt = 0;
  const unsigned long expireBlinkIntervalInMs = 420;
  TockTimer &currentTimer;
  cppQueue &_queue;
  ProgressBar *progressBar = NULL;

  char digitStringBuffer[6] = {};

  SegmentDisplay(int numDigits, int digitsPin, ProgressBar *bar, TockTimer &timer, cppQueue &queue)
      : RGBDigit(numDigits, digitsPin), _queue(queue), currentTimer(timer)
  {
    _numDigits = numDigits;
    progressBar = bar;
    currentTimer = timer;
    _queue = queue;
  }

  void formatOutputText(unsigned long b)
  {
    // Serial.print("format: ");
    // Serial.print(b);
    // Serial.print(", ");
    // Serial.println(b/1000);
    snprintf(digitStringBuffer, _numDigits + 1, "%05lu", b / 1000);
  }

  void drawBuffertoDigits(unsigned long b)
  {
    formatOutputText(b);
    setText(digitStringBuffer, 0, _numDigits, TimerColor[currentTimer.status]);
  }

  void update(bool forceUpdate = false)
  {
    unsigned long currentMillis = millis();
    // static long lastTime = 0;
    if (currentTimer.remainingTimeInMS > 0 && !_queue.isEmpty())
    {
      if ((currentMillis - updatedAt >= oneSecondInMS) || forceUpdate)
      {
        drawBuffertoDigits(currentTimer.remainingTimeInMS);
        if (!forceUpdate)
        {
          updatedAt = currentMillis;
        }
      };
    }
    else
    {
      expireBlink(currentMillis);
    };
  }

  void forceUpdate()
  {
    update(true);
  };

  void expireBlink(unsigned long currentMillis)
  {
    static unsigned long expireBlinkAt;
    static bool expireLEDBlinkOn = false;
    currentTimer = TockTimer(EXPIRE, 0);
    // NOTE: pretty sure we don't need this unless we want to show some kind
    // of custom display on expiration, but for now I think this is a good default

    // const char expireZeros[6] = "00000";

    if (currentMillis - expireBlinkAt >= expireBlinkIntervalInMs)
    {
      expireLEDBlinkOn = !expireLEDBlinkOn;
      expireBlinkAt = currentMillis;
      if (expireLEDBlinkOn)
      {

        // in case we were doing "10 second hurry up"
        clearDot(2);

        drawBuffertoDigits(0);

        // TODO: this doesn't belong here
        progressBar->fill(TimerColor[EXPIRE], 0, progressBar->_num_leds);
        progressBar->show();
      }
      else
      {
        clearAll();

        // TODO: this doesn't belong here
        progressBar->clear();
        progressBar->show();
      }
    }
  }

// private:
//   long roundUp(long numToRound, long multiple)
//   {
//     if (multiple == 0)
//       return numToRound;

//     long remainder = abs(numToRound) % multiple;
//     if (remainder == 0)
//       return numToRound;

//     if (numToRound < 0)
//       return -(abs(numToRound) - remainder);
//     else
//       return numToRound + multiple - remainder;
//   }
};

#endif