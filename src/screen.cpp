#include "debugSettings.h"
#include "screen.h"
#include "manager.h"
#include "images/images.h"
#include "strings.h"

#define BOOT_FADE_IN_TIME_MS 2000

Screen::Screen(int8_t cs, int8_t dc, int8_t rst, int8_t lite)
    : Adafruit_ST7789(cs, dc, rst)
{
    LITE_PIN = lite;
}

void Screen::setManager(TimerManager *const m)
{
    manager = m;
}

void Screen::enable()
{
    enableDisplay(true);
    enableSleep(false);
    enabled = true;
}

void Screen::disable()
{
    enableDisplay(false);
    enableSleep(true);
    enabled = false;
}

void Screen::init()
{
    pinMode(LITE_PIN, OUTPUT);
    analogWrite(LITE_PIN, 0);
    Adafruit_ST7789::init(240, 320);

    if (!enabled)
    {
        fillScreen(0x00);
        disable();
        return;
    }

    setRotation(2);
    setBGColor(0xFFFFFF);
    fillScreen(getBGColor());

    // for (int i = 0; i < CAPPED_BACKLIGHT_BRIGHTNESS; i++)
    // {
    //     analogWrite(LITE_PIN, i);
    //     if (i == CAPPED_BACKLIGHT_BRIGHTNESS)
    //     {
    //         break;
    //     }
    //     delay(BOOT_FADE_IN_TIME_MS / CAPPED_BACKLIGHT_BRIGHTNESS);
    // }
    analogWrite(LITE_PIN, CAPPED_BACKLIGHT_BRIGHTNESS);

    //  drawSplash();
    //  delay(1200);
    mode = QUEUE;
}

void Screen::drawSplash()
{

#if DEBUG
    long now = millis();
#endif

    // draw at screen center
    setCursor((width() - splashImage.width) / 2, (height() - splashImage.height) / 2);
    draw4BitBitmap(splashImage);

#if DEBUG
    now = millis() - now;
    char doneText[16] = "done";
    int mspad = sprintf(doneText, "done: %li", now);
    sprintf(&doneText[mspad], "ms");

    setTextSize(2);
    getTextBounds(doneText);
    setCursor(width() / 2 - (textBoundW / 2), height() - textBoundH - 10);
    setTextColor(0x00);
    print(doneText);
    debugln("splash done.");
#endif
}

void Screen::update()
{
    if (!enabled || !dirty)
    {
        return;
    }

    dirty = false;
    (this->*fps[mode])();
}

void Screen::setBrightness(int brightness)
{
    int b = constrain(brightness, 0, CAPPED_BACKLIGHT_BRIGHTNESS);
    analogWrite(LITE_PIN, b);
}

// TODO: should this be attached to the bitmap instead?
// Right now we can only draw one image at a time (which is fine, since we should just
// be drawing one start-to-finish, but also our graphics handling is primitive as hell),
// and technically should specify the image-to-draw in every call, which seems like the perfect
// reason to move it and remove a parameter.
// Attaching to the bitmap lets each image handle its own indexing
// TODO: also do we need out-of-bounds checking
// TODO: also also should validate numBytes
uint16_t Screen::getChunk(const byte *data, uint16_t& idx, byte numBytes)
{
    uint16_t ret;
    switch (numBytes)
    {
    case 1:
        ret = pgm_read_byte(data + idx);
        break;
    case 2:
        ret = pgm_read_word(data + idx);
        // Little-endian, swap the bytes around
        // so our consumer always gets the individual bytes in order
        ret = (ret >> 8) | (ret << 8);
        break;
    }

    idx += numBytes;
    return ret;
}

// TODO: Can probably accelerate this with some of the drawLine functions
void Screen::drawPixel(uint8_t color)
{
    uint16_t bg565 = rgb888ToRgb565(getBGColor());
    uint16_t color565 = gray4ToRgb565(color);
    if (color565 != bg565)
    {
        Adafruit_ST7789::drawPixel(getCursorX(), getCursorY(), gray4ToRgb565(color565));
    }

    // Compressed 4-bit bmp doesn't wrap pixels around boundaries, so we can skip that check
    // all line advancement should be handled when we hit a 0x00 0x00 in the scan pad
    // so I thiiiiiiink this really only ever needs to advance x
    setCursor(getCursorX() + 1, getCursorY());
}

void Screen::drawHighPixel(uint16_t colorByte)
{
    // get just the high bits & move them to the low 4 bits for 565'ing
    // always sits in the high 4 bits of the low byte
    uint8_t color = (colorByte & 0xF0) >> 4;
    drawPixel(color);
}

void Screen::drawLowPixel(uint16_t colorByte)
{
    // get just the low 4 bits of the low byte
    uint8_t color = colorByte & 0x0F;
    drawPixel(color);
}

void Screen::draw4BitBitmap(const Bitmap &bmp)
{

    bool done = false;

    uint16_t idx = 0;

    while (!done)
    {
        uint16_t scanPad = getChunk(bmp.data, idx, 2);

        if ((scanPad >> 8) == 0)
        {
            const int chunkLowByte = scanPad & 0xFF;
            // 0,1,2 are escape values for encoded mode
            // values >= 3 are absolute mode
            switch (chunkLowByte)
            {
            // EOL, go to start of next line
            case 0:
                setCursor(width() / 2 - bmp.width / 2, getCursorY() + 1);
                break;
            // EOF
            case 1:
                done = true;
                break;
            // Delta, following two bytes are horizontal & vertical
            // offsets to next pixel relative to current position
            case 2:
                scanPad = getChunk(bmp.data, idx, 2);
                setCursor(getCursorX() + (scanPad >> 8), getCursorY() + chunkLowByte);
                break;
            default:
            {
                // absolute mode, scanPad is # of indexes
                // sorry i didn't say indices
                const int numIndexes = chunkLowByte;
                for (int j = 0; j < numIndexes; j += 2)
                {
                    scanPad = getChunk(bmp.data, idx, 1);
                    drawHighPixel(scanPad);
                    drawLowPixel(scanPad);
                }
                // adjust for word boundary alignment by chewing up padding byte
                if (numIndexes % 4 > 1)
                {
                    getChunk(bmp.data, idx, 1);
                }
            }
            break;
            }
        }

        else // definitely encoded
        {

            int drawIndex = 0;
            const int repeatLength = scanPad >> 8;
            scanPad &= 0xFF;

            while (drawIndex < repeatLength)
            {
                drawHighPixel(scanPad);
                drawIndex++;
                // odd number of pixels, bail early
                if (drawIndex == repeatLength)
                {
                    continue;
                }
                drawLowPixel(scanPad);
                drawIndex++;
            }
        }
    }
};

void Screen::displayQueue()
{

    fillScreen(0x00);
    setTextColor(0xFFFF);
    setTextSize(2);

    setCursor(width() - plugImage.width - 10, height() - plugImage.height - 10);
    draw4BitBitmap(plugImage);

    char title[] = {"Current:"};

    getTextBounds(title);
    setCursor(120 - (textBoundW / 2), 5);
    print(title);

    int cursorY = 30;
    setTextColor(rgb888ToRgb565(TimerColor[manager->getStatus()]), getBGColor());
    setCursor(15, cursorY);

    // TODO: faster to combine these to one call?
    print(statusType[manager->getStatus()]);
    print(" for ");
    print(manager->getCurrentTimer()->initialTimeInMS / 1000);
    fillRect(getCursorX(), getCursorY(), width() - getCursorX(), textBoundH, getBGColor());

    TockTimer buffer;
    int result = manager->iterateNextInQueue(&buffer);
    if (result)
    {
        getTextBounds(strings::queued);
        setTextColor(0xFFFF);
        setCursor(120 - (textBoundW / 2), 70);
        print(strings::queued);
        cursorY = 95;

        while (result)
        {
            long curColor = rgb888ToRgb565(TimerColor[buffer.status]);
            setTextColor(curColor, getBGColor());
            setCursor(15, cursorY);
            print(statusType[buffer.status]);
            print(" for ");
            print(buffer.initialTimeInMS / 1000);

            // blank out the rest of the line
            fillRect(getCursorX(), getCursorY(), width() - getCursorX(), textBoundH, getBGColor());

            cursorY += 25 - textsize_y;
            result = manager->iterateNextInQueue(&buffer);
        }
    }
    // blank out the rest of the screen, clipping handled automatically
    fillRect(0, cursor_y + textBoundH, width(), height() - textBoundH, getBGColor());
}

void Screen::displayExpired()
{
    getTextBounds(strings::timeup);
    setCursor((width() - textBoundW) / 2, (height() / 2 - textBoundH) / 2);
    setTextColor(rgb888ToRgb565(TimerColor[manager->getStatus()]), getBGColor());
    fillScreen(getBGColor());
    print(strings::timeup);
    return;
}

void Screen::setMode(Mode m)
{
    mode = m;
    dirty = true;
}

Screen::Mode Screen::getMode()
{
    return mode;
}

long Screen::getBGColor()
{

    int size = sizeof(TimerColor) / sizeof(TimerColor[0]);
    return TimerColor[size];
}

void Screen::setBGColor(uint32_t bgColor){
    int size = sizeof(TimerColor) / sizeof(TimerColor[0]);
    TimerColor[size] = rgb888ToRgb565(bgColor);
}

uint16_t Screen::rgb888ToRgb565(unsigned long color)
{
    return ((color >> 8) & 0xf800) | // Red   → bits 11–15
           ((color >> 5) & 0x07e0) | // Green → bits 5–10
           ((color >> 3) & 0x001f);  // Blue  → bits 0–4
}

uint16_t Screen::gray4ToRgb565(uint8_t g4) {
    // expand 4-bit to 8-bit: v = g4 * 17  (same as (g4<<4)|g4)
    uint8_t v = g4 * 17u;
    uint16_t r5 = (v >> 3) & 0x1F;
    uint16_t g6 = (v >> 2) & 0x3F;
    uint16_t b5 = (v >> 3) & 0x1F;
    return (r5 << 11) | (g6 << 5) | b5;
    };

void Screen::getTextBounds(const char* str){
    Adafruit_GFX::getTextBounds(str, 0, 0, &textBoundX, &textBoundY, &textBoundW, &textBoundH);
};

void Screen::displayMenu() {};