#ifndef SCREEN_HEADER
#define SCREEN_HEADER

#include "debugSettings.h"
#include <Adafruit_ST7789.h>
#include "typeDefs.h"
#include "images/images.h"

#define CAPPED_NEOPIXEL_BRIGHTNESS 90
#define MAX_BACKLIGHT_BRIGHTNESS 127
#define BOOT_FADE_IN_TIME_MS 2000

class TimerManager;

class Screen : public Adafruit_ST7789
{
public:
    Screen(int8_t cs, int8_t dc, int8_t rst, int8_t lite);

    boolean enabled = true,
            dirty = false;
    int8_t LITE_PIN;

    // storage variables for getTextBounds()
    int16_t textBoundX = 0, 
            textBoundY = 0;
    uint16_t textBoundW = 0, 
             textBoundH = 0;

    TimerManager* manager = nullptr;

    void setManager(TimerManager* const m);

    void enable();
    void disable();
    void init();
    void drawSplash();
    void update(TockTimer cT, int (*func)(TockTimer *t));

private:
    int idx;
    //TODO: this is all stuff that should probably be in the Bitmap class
    unsigned int getNextChunk(byte numBytes = 2, const byte* data = splashImage.data);
    void drawPixel(unsigned int color);
    void drawHighPixel(unsigned int colorByte);
    void drawLowPixel(unsigned int colorByte);
    void draw4BitBitmap(Bitmap &bmp);
};
#endif //header guard