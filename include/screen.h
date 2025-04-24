#ifndef SCREEN_HEADER
#define SCREEN_HEADER

#include "debugSettings.h"
#include <Adafruit_ST7789.h>
#include "typeDefs.h"
#include "images/images.h"

#define CAPPED_NEOPIXEL_BRIGHTNESS 90
#define MAX_BACKLIGHT_BRIGHTNESS 127
#define BOOT_FADE_IN_TIME_MS 2000

class Screen;
class TimerManager;
typedef void (Screen::*FunctionPointer)();
int iterateNextInQueue(TockTimer *buf);

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

    TimerManager *manager = nullptr;

    void setManager(TimerManager *const m);

    void enable();
    void disable();
    void init();
    void drawSplash();
    void update();

private:

    enum Mode
    {
        SPLASH,
        QUEUE,
        MENU
    };

    Mode mode = SPLASH;

    int idx = 0;

    FunctionPointer fps[2] = {
        &drawSplash,
        &displayQueue
    };

    // TODO: this is all stuff that should probably be in the Bitmap class
    uint16_t getNextChunk(byte numBytes = 2, const byte *data = splashImage.data);
    void drawPixel(uint16_t color);
    void drawHighPixel(uint16_t colorByte);
    void drawLowPixel(uint16_t colorByte);
    void draw4BitBitmap(const Bitmap &bmp);
    void displayQueue();

    

    uint16_t RGB888toRGB565(long color);
};
#endif // header guard