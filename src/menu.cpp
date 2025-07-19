#include "menu.h"
#include <Arduino.h>

void setPallete(prefs &p, int palleteIndex)
{

#if DEBUG
    if (palleteIndex > (sizeof(p.palletes) / sizeof(palleteOption)) || palleteIndex < 0)
    {
        debug("invalid palleteIndex passed: ");
        debug(palleteIndex);
        debug(", max index: ");
        debugln(sizeof(p.palletes) / sizeof(palleteOption));
    };
#endif

    int x = sizeof(TimerColor) / sizeof(TimerColor[0]);

    for (int i = 0; i < x; i++)
    {
        TimerColor[i] = p.palletes[palleteIndex].pallete[i];
    };
};