#include "segmentDisplay.h"
#include "manager.h"

SegmentDisplay::SegmentDisplay(int numDigits, int digitsPin)
    : RGBDigit(numDigits, digitsPin)
{
  _numDigits = numDigits;
}

void SegmentDisplay::setManager(TimerManager *const m)
{
  manager = m;
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

  if ((currentMillis - updatedAt >= oneSecondInMS) || forceUpdate)
  {
    // millis() occasionally skips a number due to prescaler reasons
    // but has a built-in adjustment to keep it in sync
    // This should keep us with the proper display so long as we don't manage to be 500ms off
    // which would mean much larger problems anway.
    long drawTime = lround((double)manager->getRemainingTime() / oneSecondInMS) * oneSecondInMS;
    drawBuffertoDigits(drawTime);
    if (manager->getRemainingTime() < 0)
    {
      debugln(manager->getElapsedPercentageNormalized());
      while (1)
        ;
    }
    if (!forceUpdate)
    {
      updatedAt = currentMillis;
    }
  };
};

void SegmentDisplay::forceUpdate()
{
  update(true);
};

void SegmentDisplay::expireBlink()
{

  static unsigned long expireBlinkAt = currentMillis;

  //TODO: Yeah but what about when we hit a second expiration?
  static bool expireLEDBlinkOn = [this]()
  {
    clearDot(2);
    drawBuffertoDigits(0);

    return true;
  }();
  // NOTE: pretty sure we don't need this unless we want to show some kind
  // of custom display on expiration, but for now I think this is a good default

  // const char expireZeros[6] = "00000";

  if (currentMillis - expireBlinkAt >= expireBlinkIntervalInMS)
  {
    expireLEDBlinkOn = !expireLEDBlinkOn;
    expireBlinkAt = currentMillis;
    if (expireLEDBlinkOn)
    {

      // in case we were doing "10 second hurry up"
      clearDot(2);
      drawBuffertoDigits(0);
    }
    else
    {
      clearAll();
    }
  }
}