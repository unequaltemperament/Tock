#ifndef SCREEN_HEADER
#define SCREEN_HEADER

#include "debugSettings.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "typeDefs.h"
#include "images/images.h"

#define CAPPED_NEOPIXEL_BRIGHTNESS 90
#define MAX_BACKLIGHT_BRIGHTNESS 127
#define BOOT_FADE_IN_TIME_MS 2000

class Screen : public Adafruit_ST7789
{
public:
    Screen(int8_t cs, int8_t dc, int8_t rst, int8_t lite);

    boolean enabled = true,
            dirty = false;
    int8_t LITE_PIN;

    // storage variables for getTextBounds()
    int16_t xTB = 0, 
            yTB = 0;
    uint16_t wTB = 0, 
             hTB = 0;

    // TODO: hardcoded is sad
    // but current BMP compressed encoding
    // doesn't include image size
    // width = 106, height = 40, cursor inital position x=0,y=0

    void enable();
    void disable();
    void init();
    void drawSplash();
    void update(TockTimer cT, int (*func)(TockTimer *t));

private:
    
    unsigned int getNextChunk(byte numBytes = 2, const byte data[] = splashImageData);
    void advanceCursor(int numPixelsDrawn = 1);
    void setCursorForCenteredImageDraw(Bitmap &bmp);
    void drawPixel(unsigned int color);
    void drawHighPixel(unsigned int colorByte);
    void drawLowPixel(unsigned int colorByte);
};
#endif