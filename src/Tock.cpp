#include "debugSettings.h"
#include "boardConfigs/config.h"
#include "typeDefs.h" 
#include <Arduino.h>
#include <Wire.h>
#include <cppQueue.h>
#include "manager.h"
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"
#include "menu.h"

constexpr char NUM_LEDS = 36;
constexpr char NUM_DIGITS = 5;
constexpr char QUEUE_MAX_SIZE = 10;

unsigned long currentMillis;
extern bool sensorsEnabled;
extern struct userPrefs uPrefs;
extern struct menuOptions menuOptions;

cppQueue timerQueue(sizeof(TockTimer), QUEUE_MAX_SIZE);
ProgressBar progressBar(NUM_LEDS, LED_PIN);
SegmentDisplay segmentDisplay(NUM_DIGITS, DIGITS_PIN);
Screen screen(TFT_CS, TFT_DC, TFT_RST, TFT_LITE);
Adafruit_CAP1188 cap(CAP_RST);
TimerManager manager(segmentDisplay, progressBar, screen, timerQueue);

void setup()
{
  Wire.setTimeout(300);
#if DEBUG
  Serial.begin(115200);
  debugln("");
  debugln("--------Everything above this line is garbage on reset--------");
#endif

  initPrefs(uPrefs);
  segmentDisplay.init();
  progressBar.init();
  // screen.enabled = false;
  screen.init();
  initSensors();

  timerQueue.flush(); // here now in case we wrap this in some kind of reset function later

  // Force first timer to be a reasonable value
  // manager.queueTimer(TimerStatus::WORK, 30);
  // manager.queueTimer(TimerStatus::BREAK, 30);
  manager.queueTimer(TimerStatus::WORK, 6);
  manager.queueTimer(TimerStatus::BREAK, 6);
  // manager.queueTimer(TimerStatus::WORK, 4);
  // manager.queueTimer(TimerStatus::BREAK, 4);
  // manager.queueTimer(TimerStatus::WORK, 4);
  // manager.queueTimer(TimerStatus::BREAK, 4);
  // manager.queueTimer(TimerStatus::WORK, 4);
  // manager.queueTimer(TimerStatus::BREAK, 4);

  manager.loadNextTimer();
  manager.start();

  // TODO: Screen update is slow.
  //  on screen redraw, we lag about 250ms updating the progressbar, causing a jerky first LED
  screen.update();

  debugln("--------");
  char initBuffer[32];
  sprintf(initBuffer, "%-13s%s", "Digits", segmentDisplay.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "ProgressBar", progressBar.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "Screen", screen.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "Sensors", sensorsEnabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s%s", "UserPrefs", uPrefs.init ? "loaded" : "NOT LOADED");
  debugln(initBuffer);
  if(!uPrefs.init){
  sprintf(initBuffer, "%-13s", "falling back to defaults");
  debugln(initBuffer);
  }
  sprintf(initBuffer, "%-13s%s", "Autobright", uPrefs.brightness.autoSet ? "on" : "off");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%i", "Saved brightness", uPrefs.brightness.value);
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", menuOptions.palletes[uPrefs.selectedPalette].palleteName, "palette selected");
  debugln(initBuffer);
  debugln("--------");

  debug(millis());
  debugln(": End of setup, entering loop");
}

void loop()
{
  currentMillis = millis();
  getSensorInput();
  screen.dirty = processButtonQueue(timerQueue);
  manager.update();
  screen.update();
}