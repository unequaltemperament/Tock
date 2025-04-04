#include "progressBar.h"
#include "manager.h"

ProgressBar::ProgressBar(int num_leds, int led_pin)
    : Adafruit_NeoPixel(num_leds, led_pin, NEO_GRB)
{
    _num_leds = num_leds;
    //uint8_t *_pixels = getPixels();
    
}


void ProgressBar::setManager(TimerManager* const m){
    manager = m;
};

void ProgressBar::update(bool forceUpdate = false)
{

    if ((currentMillis - updatedAt >= lightIntervalInMs && manager->getRemainingTime() > 0) || forceUpdate)
    {
        if (!forceUpdate)
        {
            updatedAt = currentMillis;
        }
        // TODO: get rid of this modff
        // TODO: (HANDLED?)
        double elapsedPercentage = manager->getElapsedPercentage() / 100;

        int fullLEDs = elapsedPercentage * _num_leds;
        double partialLEDPercentage = (elapsedPercentage * _num_leds) - fullLEDs;        

        /*float fullLEDsInt;
        float partial = modff(((float)currentTimer.remainingTimeInMS * _num_leds / currentTimer.initialTimeInMS), &fullLEDsInt);
        fullLEDsInt = _num_leds - fullLEDsInt;*/

        /*for (int i = 0; i < fullLEDs; i++)
        {
            setPixelColor(getMappedLED(i), TimerColor[currentTimer.status]);
            //_pixels[getMappedLED(i)] = TimerColor[currentTimer.status];
        }*/

        //NOTE: this sets the last full LED on every pass
        //Probably not a performance issue, but would rather note it now

        //using fullLEDs as an index is off-by-1, -1 is the last actual fullLED and -0 is the current partial LED
        if(fullLEDs > 0){
            setPixelColor(getMappedLED(fullLEDs-1), TimerColor[manager->getStatus()]);
        }

        if (fullLEDs < _num_leds && partialLEDPercentage > 0)
        {

            setPixelColor(getMappedLED(fullLEDs), getDimmedColor(TimerColor[manager->getStatus()], /* 1 - */ partialLEDPercentage));
        }   
        show();
    }
}

void ProgressBar::forceUpdate()
{
    update(true);
}

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

int ProgressBar::getMappedLED(int realID)
{
    if(!lightFromWiredEnd) {
        realID = _num_leds - realID - 1;}

    //return (realID % 2 == 0) ? (realID / 2) : (_num_leds - (realID / 2) - 1);
    return ((realID & 1) == 0) ? (realID >> 1) : _num_leds - (realID >> 1) - 1;
}

// TODO: keep working on this....
// needs a gamma map or something?
// green overwhelms when all three colors are scaled equally
uint32_t ProgressBar::getDimmedColor(uint32_t color, float dimPercentage)
{

    uint32_t r = (color >> 16 & 0xFF);
    uint32_t g = (color >> 8 & 0xFF);
    uint32_t b = (color & 0xFF);

    r = (uint32_t)r * dimPercentage;
    g = (uint32_t)g * dimPercentage;
    b = (uint32_t)b * dimPercentage;

    return (r << 16) | (g << 8) | b;
}