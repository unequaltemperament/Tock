#include "segmentDisplay.h"
#include "manager.h"

void SegmentDisplay::setManager(TimerManager* const m){
  manager = m;
}

  SegmentDisplay::SegmentDisplay(int numDigits, int digitsPin)
      : RGBDigit(numDigits, digitsPin)
  {
    _numDigits = numDigits;
  }

  void SegmentDisplay::formatOutputText(unsigned long b)
  {
    // Serial.print("format: ");
    // Serial.print(b);
    // Serial.print(", ");
    // Serial.println(b/1000);
    snprintf(digitStringBuffer, _numDigits + 1, "%05lu", b / 1000);
  }

  void SegmentDisplay::drawBuffertoDigits(unsigned long b)
  {
    formatOutputText(b);
    setText(digitStringBuffer, 0, _numDigits, TimerColor[manager->getStatus()]);
  }

  void SegmentDisplay::update(bool forceUpdate = false)
  {

    if (!manager->isExpired())
    {
      if ((currentMillis - updatedAt >= oneSecondInMS) || forceUpdate)
      {
        drawBuffertoDigits(manager->getRemainingTime());
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

  void SegmentDisplay::forceUpdate()
  {
    update(true);
  };

  void SegmentDisplay::expireBlink(unsigned long currentMillis)
  {
    // static unsigned long expireBlinkAt;
    // static bool expireLEDBlinkOn = false;
    // manager.currentTimer = TockTimer(EXPIRE, 0);
    // // NOTE: pretty sure we don't need this unless we want to show some kind
    // // of custom display on expiration, but for now I think this is a good default

    // // const char expireZeros[6] = "00000";

    // if (currentMillis - expireBlinkAt >= expireBlinkIntervalInMS)
    // {
    //   expireLEDBlinkOn = !expireLEDBlinkOn;
    //   expireBlinkAt = currentMillis;
    //   if (expireLEDBlinkOn)
    //   {

    //     // in case we were doing "10 second hurry up"
    //     clearDot(2);

    //     drawBuffertoDigits(0);

    //     // TODO: this doesn't belong here
    //     progressBar->fill(TimerColor[EXPIRE], 0, progressBar->_num_leds);
    //     progressBar->show();
    //   }
    //   else
    //   {
    //     clearAll();

    //     // TODO: this doesn't belong here
    //     progressBar->clear();
    //     progressBar->show();
    //   }
    // }
  }

// private:
//   long SegmentDisplay::roundUp(long numToRound, long multiple)
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