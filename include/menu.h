#ifndef MENU_HEADER
#define MENU_HEADER


#include "debugSettings.h"
#include "typeDefs.h"


extern long TimerColor[4];

struct prefs {

    struct brightness {
        bool autoSet = false;
        int value = 127;
    };
    
    palleteOption palletes[8] = {
        {"default",{0,0xFF0000, 0xD9FF00, 0x2AE600}},
        {"fire",{0xff0000,0xdd3333,0x881111,0xff5353}}
    };
    
};

void setPallete(prefs& p, int palleteIndex);

#endif