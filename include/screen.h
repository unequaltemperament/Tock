#ifndef SCREEN_HEADER
#define SCREEN_HEADER

#include "typeDefs.h"
#include <Adafruit_ST7789.h>
#include "images/images.h"

class TimerManager;

class Screen;
typedef void (Screen::*FunctionPointer)();

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

    enum Mode
    {
        SPLASH,
        QUEUE,
        EXPIRED,
        MENU
    };

    

    void setManager(TimerManager *const m);

    void enable();
    void disable();
    void init();
    void update();
    void setBrightness(int brightness);
    void setMode(Mode m);
    Mode getMode();


    
private:

    void drawSplash();
    void displayQueue();
    void displayExpired();
    void displayMenu();

    Mode mode = SPLASH;

    int idx = 0;

    FunctionPointer fps[4] = {
        &Screen::drawSplash,
        &Screen::displayQueue,
        &Screen::displayExpired,
        &Screen::displayMenu
    };

    // TODO: this is all stuff that should probably be in the Bitmap class
    uint16_t getNextChunk(byte numBytes = 2, const byte *data = splashImage.data);
    void drawPixel(uint16_t color);
    void drawHighPixel(uint16_t colorByte);
    void drawLowPixel(uint16_t colorByte);
    void draw4BitBitmap(const Bitmap &bmp);

    long getBGColor();    

    uint16_t RGB888toRGB565(long color);
};
#endif // header guard