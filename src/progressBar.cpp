#include "debugSettings.h"
#include "progressBar.h"
#include "manager.h"

ProgressBar::ProgressBar(int num_leds, int led_pin)
    : Adafruit_NeoPixel(num_leds, led_pin, NEO_GRB)
{
    _num_leds = num_leds;
}

void ProgressBar::setManager(TimerManager *const m)
{
    manager = m;
};

void ProgressBar::init()
{
  begin(); // INITIALIZE NeoPixel progressBar.updatedAt object (REQUIRED)
  show();  // Turn OFF all pixels ASAP
  setBrightness(CAPPED_BAR_BRIGHTNESS);
  enabled = true;
}

void ProgressBar::update(bool forceUpdate = false)
{
    if (manager->getStatus() != TimerStatus::EXPIRE)
    {
        if ((currentMillis - updatedAt >= lightIntervalInMs && manager->getRemainingTime() > 0) || forceUpdate)
        {
            if (!forceUpdate)
            {
                updatedAt = currentMillis;
            }
  
            double elapsedPercentage = manager->getElapsedPercentageNormalized();

            int fullLEDs = elapsedPercentage * _num_leds;
            double partialLEDPercentage = (elapsedPercentage * _num_leds) - fullLEDs;

            long c = TimerColor[manager->getStatus()];
            //NOTE: cannot use fill() because of the way the LEDs are wired
            if (fullLEDs > 0)
            {
                for(int i = 0; i < fullLEDs; i++)
                {
                    setPixelColor(getMappedLED(i), c);
                }
            }

            if (fullLEDs < _num_leds && partialLEDPercentage > 0)
            {
                setPixelColor(getMappedLED(fullLEDs), getDimmedColor(c, partialLEDPercentage));
            }
            show();
        }
    }
}

void ProgressBar::forceUpdate()
{
    update(true);
}

void ProgressBar::expireBlink()
{
    static unsigned long expireBlinkAt = currentMillis;

    if (currentMillis - expireBlinkAt >= expireBlinkIntervalInMS)
    {
        expireLEDBlinkOn = !expireLEDBlinkOn;
        expireBlinkAt = currentMillis;

        fill(TimerColor[manager->getStatus()] * expireLEDBlinkOn, 0, _num_leds);
        show();
    }
}

// Once installed, LEDS are ordered like this from the data/power pins.
// lightFromWiredEnd is set false during "normal" operation
// but is possibly set to true during testing
//
//  5   4   3   2   1
//    6   7   8   9   10
//
//  Assuming lightFromWiredEnd is false (default), then lighting order is:
//  Reverse it when lightFromWiredEnd is true
//
//    5 6 4 7 3 8 2 9 1 10

int ProgressBar::getMappedLED(int realID)
{
    if (!lightFromWiredEnd)
    {
        realID = _num_leds - realID - 1;
    }

    return ((realID & 1) == 0) ? (realID >> 1) : _num_leds - (realID >> 1) - 1;
}

// TODO: keep working on this....
// needs a gamma map or something?
// green overwhelms when all three colors are scaled equally
uint32_t ProgressBar::getDimmedColor(uint32_t color, float dimPercentage)
{

    auto dim = [dimPercentage](uint32_t c){
        return static_cast<uint32_t>((c & 0xFF) * dimPercentage);
    };

    return (dim(color >> 16) << 16 | dim(color >> 8) << 8 | dim(color));

    // This saves 8 byte over the above
    // uint32_t r = (color >> 16 & 0xFF);
    // uint32_t g = (color >> 8 & 0xFF);
    // uint32_t b = (color & 0xFF);

    
    // r = static_cast<uint32_t>(r * dimPercentage);
    // g = static_cast<uint32_t>(g * dimPercentage);
    // b = static_cast<uint32_t>(b * dimPercentage);

    // return (r << 16) | (g << 8) | b;
}