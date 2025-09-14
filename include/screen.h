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

    FunctionPointer fps[4] = {
        &Screen::drawSplash,
        &Screen::displayQueue,
        &Screen::displayExpired,
        &Screen::displayMenu
    };

    // TODO: this is all stuff that should probably be in the Bitmap class
    uint16_t getChunk(const byte *data, uint16_t& idx, byte numBytes);
    void drawPixel(uint8_t color);
    void drawHighPixel(uint16_t colorByte);
    void drawLowPixel(uint16_t colorByte);
    void draw4BitBitmap(const Bitmap &bmp);

    long getBGColor();
    void setBGColor(uint32_t bgColor);
    inline void getTextBounds(const char* str);

    uint16_t rgb888ToRgb565(unsigned long color);
    uint16_t gray4ToRgb565(uint8_t g4);  
};
#endif // header guard