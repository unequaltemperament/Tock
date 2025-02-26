#include "screen.h"

Screen::Screen(int8_t cs, int8_t dc, int8_t rst, int8_t lite)
    : Adafruit_ST7789(cs, dc, rst)
{
    LITE_PIN = lite;
    textBoundW = Adafruit_GFX::width();
    textBoundH = Adafruit_GFX::height();
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
    fillScreen(0xFFFF);

    for (int i = 0; i < MAX_BACKLIGHT_BRIGHTNESS; i++)
    {
        analogWrite(LITE_PIN, i);
        if (i == MAX_BACKLIGHT_BRIGHTNESS)
        {
            break;
        }
        delay(BOOT_FADE_IN_TIME_MS/ MAX_BACKLIGHT_BRIGHTNESS);
    }

    drawSplash();
    delay(1200);

    // TODO: the rest of this should go in the update loop
    fillScreen(0x00);
    setTextColor(0xFFFF);
    setTextSize(2);

    char title[] = {"Current:"};

    getTextBounds(title, 0, 0, &textBoundX, &textBoundY, &textBoundW, &textBoundH);
    setCursor(120 - (textBoundW / 2), 5);
    print(title);
}

void Screen::drawSplash()
{

#if DEBUG
    long now = millis();
#endif

    //TODO: why does copying this into another variable reduce program size?
    Bitmap bmp = splashImage;
    //draw at screen center
    setCursor((width() - bmp.width) / 2, (height() - bmp.height ) / 2);
    draw4BitBitmap(bmp);


    
#if DEBUG
    now = millis() - now;
    char doneText[16] = "done";
    int mspad = sprintf(doneText, "done: %li", now);
    sprintf(&doneText[mspad], "ms");

    setTextSize(2);
    getTextBounds(doneText, 0, 0, &textBoundX, &textBoundY, &textBoundW, &textBoundH);
    setCursor(width() / 2 - (textBoundW / 2), height() - textBoundH - 10);
    setTextColor(0x00);
    print(doneText);
    debugln("splash done.");
#endif
}

void Screen::update(TockTimer cT, int (*func)(TockTimer *t))
{
    TockTimer storage;
    const char queued[] = {"Coming Up:"};
    if (dirty)
    {
        int cursorStart = 30;
        setTextColor(TimerColor[cT.status], 0x0000);
        setCursor(15, cursorStart);
        print(statusType[cT.status]);
        print(" for ");
        print(cT.initialTimeInMS / 1000);

        setTextColor(0xFFFF);
        getTextBounds(queued, 0, 0, &textBoundX, &textBoundY, &textBoundW, &textBoundH);
        setCursor(120 - (textBoundW / 2), 70);
        print(queued);

        cursorStart = 95;

        int result = func(&storage);

        // TODO: better handling of flagging screen needing redraw
        dirty = static_cast<bool>(result);
        while (result && storage.status != 0 && dirty)
        {
            long curColor = TimerColor[storage.status];
            curColor = (curColor >> 8 & 0xf800) | (curColor >> 5 & 0x07e0) | (curColor >> 3 & 0x001f);
            setTextColor(curColor, 0x0000);
            setCursor(15, cursorStart);
            print(statusType[storage.status]);
            print(" for ");
            print(storage.initialTimeInMS / 1000);
            cursorStart += 25;
            // debug(storage.status);
            // debug(" for ");
            // debugln(storage.initialTimeInMS/1000);
            // debugln(result);
            result = func(&storage);
        }
    }
}

// TODO: should this be attached to the bitmap instead?
// Right now we can only draw one image at a time (which is fine, since we should just
// be drawing one start-to-finish, but also our graphics handling is primitive as hell),
// and technically should specify the image-to-draw in every call, which seems like the perfect
// reason to move it and remove a parameter.
// Attaching to the bitmap lets each image handle its own indexing
// TODO: also do we need out-of-bounds checking
unsigned int Screen::getNextChunk(byte numBytes, const byte *data)
{
    unsigned int ret;
    if (numBytes == 1)
    {
        ret = pgm_read_byte(data + idx);
        idx++;
    }
    else
    {
        ret = pgm_read_word(data + idx);
        // Little-endian, swap the bytes around
        // so our consumer gets the individual bytes in order
        ret = (ret >> 8) | (ret << 8);
        idx += 2;
    }

    return ret;
}

// TODO: Can probably accelerate this with some of the drawLine functions
void Screen::drawPixel(unsigned int color)
{

    // TODO: bg color checking (if color != bgcolor)
    if (color != 0x0F)
    {
        // RGB565
        color = (color << 1 | color >> 3) << 11 | (color << 2 | color >> 2) << 5 | (color << 1 | color >> 3);

        Adafruit_ST7789::drawPixel(getCursorX(), getCursorY(), (uint16_t)color);
    }

    // Compressed 4-bit bmp doesn't wrap pixels around boundaries, so we can skip that check
    // all line advancement should be handled when we hit a 0x00 0x00 in the scan pad
    // so I thiiiiiiink this really only ever needs to advance x
    setCursor(getCursorX() + 1, getCursorY());
}

void Screen::drawHighPixel(unsigned int colorByte)
{
    // get just the high bits & move them to the low 4 bits for 565'ing
    // always sits in the high 4 bits of the low byte
    unsigned int color = (colorByte & 0x00FF) >> 4;
    drawPixel(color);
}

void Screen::drawLowPixel(unsigned int colorByte)
{
    // get just the low 4 bits of the low byte
    unsigned int color = colorByte & 0x000F;
    drawPixel(color);
}

void Screen::draw4BitBitmap(Bitmap &bmp)
{
    
    unsigned int scanPad;
    bool done = false;

    idx = 0;
    
    while (!done)
    {
        scanPad = getNextChunk();
        if ((scanPad >> 8) == 0)
        {
            const int chunkLowByte = scanPad & 0x00FF;
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
                    scanPad = getNextChunk();
                    setCursor(getCursorX() + (scanPad >> 8), getCursorY() + chunkLowByte);
                    break;
                default:
                {
                    // absolute mode, scanPad is # of indexes
                    // sorry i didn't say indices
                    const int numIndexes = chunkLowByte;
                    for (int j = 0; j < numIndexes; j += 2)
                    {
                        scanPad = getNextChunk(1);
                        drawHighPixel(scanPad);
                        drawLowPixel(scanPad);
                    }
                    // adjust for word boundary alignment by chewing up padding byte
                    if (numIndexes % 4 > 1)
                    {
                        getNextChunk(1);
                    }
                }
            }
        }

        else  // definitely encoded
        {

            int drawIndex = 0;
            const int repeatLength = scanPad >> 8;
            scanPad &= 0x00FF;

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

