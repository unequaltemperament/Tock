#include "typeDefs.h"
#include "menu.h"
#include <Arduino.h>
#include <EEPROM.h>

struct menuOptions menuOptions{
    {false, 127},
    {{"default", {0, 0xFF0000, 0xD9FF00, 0x2AE600, 0x101010}},
     {"fire",    {0, 0xE64A4A, 0xE6B34A, 0xEEF208, 0x2B0C0C}},
     {"water",   {0, 0x265EE0, 0x26DBE0, 0xBCFFFC, 0x0A1E40}},
     {"forest",  {0, 0x147521, 0x50CE61, 0x15FF33, 0x0A1C0E}},
     {"clouds",  {0, 0xD8EBEA, 0xA2FFFC, 0x8C8C8C, 0x2E2E2E}},
     {"sunset",  {0, 0x9A6ABE, 0xC49057, 0xEB60D2, 0x2B1A29}},
     {"pastel",  {0, 0x00CAFF, 0x9CEF95, 0xFF7D8D, 0x1F1F2A}},
     {"retro",   {0, 0xC78F00, 0x00B6C6, 0xE260FF, 0x231B0F}}}
};

struct userPrefs uPrefs{
    {false, 127},
    0,
    false
};

void setPallete(const userPrefs &p)
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

bool initPrefs(userPrefs& prefs) {

    prefs.brightness.autoSet = EEPROM.read(0);                     
    prefs.brightness.value = word(EEPROM.read(1),EEPROM.read(2));  
    prefs.selectedPalette = EEPROM.read(3);                        
    prefs.init = EEPROM.read(4);                                  

    return prefs.init;
};

bool savePrefs(const userPrefs& prefs){
    bool success = 1;

    EEPROM.update(0, prefs.brightness.autoSet);
    EEPROM.update(1, prefs.brightness.value >> 8);
    EEPROM.update(2, prefs.brightness.value & 0x0F);
    EEPROM.update(3, prefs.selectedPalette);
    
   success =  (prefs.brightness.autoSet == EEPROM.read(0))
             &(prefs.brightness.value == word(EEPROM.read(1),EEPROM.read(2)))
             &(prefs.selectedPalette == EEPROM.read(3));

    EEPROM.update(4, success);

    return success;


}