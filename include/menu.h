#ifndef MENU_HEADER
#define MENU_HEADER

#include "typeDefs.h"

struct menuOptions
{

    struct brightness
    {
        bool autoSet;
        int value;
    } brightness;

    palleteOption palletes[8];
};

struct userPrefs
{
    struct brightness
    {
        bool autoSet;
        int value;
    } brightness;

    char selectedPalette;

    bool init;
};

void setPallete(const userPrefs &p);
void setPallete(int palleteIndex);
void initPrefs();

#endif // header guard