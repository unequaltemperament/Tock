#ifndef SCREEN_HEADER
#define SCREEN_HEADER

#include "typeDefs.h"
#include <Adafruit_ST7789.h>
#include "images/images.h"

class TimerManager;

class Screen;
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
    void update();
    void setBrightness(int brightness);
    void setMode(TimerStatus t);
    
private:

    void drawSplash();
    void displayQueue();
    void displayElapsed();

    enum Mode
    {
        SPLASH,
        QUEUE,
        ELAPSED,
        MENU
    };

    Mode mode = SPLASH;

    int idx = 0;

    FunctionPointer fps[3] = {
        &Screen::drawSplash,
        &Screen::displayQueue,
        &Screen::displayElapsed
    };

    // TODO: this is all stuff that should probably be in the Bitmap class
    uint16_t getNextChunk(byte numBytes = 2, const byte *data = splashImage.data);
    void drawPixel(uint16_t color);
    void drawHighPixel(uint16_t colorByte);
    void drawLowPixel(uint16_t colorByte);
    void draw4BitBitmap(const Bitmap &bmp);

    long getBG();    

    uint16_t RGB888toRGB565(long color);
};
#endif // header guard