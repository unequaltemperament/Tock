#include "menu.h"
#include "typeDefs.h"

struct menuOptions menuOptions{
    {false, 127},
    {{"default", {0, 0xFF0000, 0xD9FF00, 0x2AE600}},
     {"fire", {0, 0xE64A4A, 0xE6B34A, 0xEEF208}},
     {"water", {0, 0x265EE0, 0x26DBE0, 0xBCFFFC}},
     {"forest", {0, 0x147521, 0x50CE61, 0x15FF33}},
     {"clouds", {0, 0xD8EBEA, 0xA2FFFC, 0x8C8C8C}},
     {"sunset", {0, 0x9A6ABE, 0xC49057, 0xEB60D2}},
     {"pastel", {0, 0x00caff, 0x9cef95, 0xff7d8d}},
     {"retro", {0, 0xc78f00, 0x00b6c6, 0xe260ff}}}
};

struct userPrefs uPrefs{
    {false, 127},
    0,
    false
};

void setPallete(userPrefs &p)
{
    int x = sizeof(TimerColor) / sizeof(TimerColor[0]);

    for (int i = 0; i < x; i++)
    {
        TimerColor[i] = menuOptions.palletes[p.selectedPalette].pallete[i];
    };
};

void setPallete(int palleteIndex)
{
    int x = sizeof(TimerColor) / sizeof(TimerColor[0]);

    uPrefs.selectedPalette = palleteIndex;

    for (int i = 0; i < x; i++)
    {
        TimerColor[i] = menuOptions.palletes[palleteIndex].pallete[i];
    };
};

void initPrefs() {};