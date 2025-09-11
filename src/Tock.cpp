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

cppQueue timerQueue(sizeof(TockTimer), QUEUE_MAX_SIZE);
ProgressBar progressBar(NUM_LEDS, LED_PIN);
SegmentDisplay segmentDisplay(NUM_DIGITS, DIGITS_PIN);
Screen screen(TFT_CS, TFT_DC, TFT_RST, TFT_LITE);
Adafruit_CAP1188 cap(CAP_RST);
TimerManager manager(segmentDisplay, progressBar, screen, timerQueue);

bool queueTimer(cppQueue &q, TimerStatus status, long initialTimeInSeconds)
{
  bool result;
  TockTimer timer(status, initialTimeInSeconds);
  result = q.push(&timer);
  return result;
}

int iterateNextInQueue(TockTimer *buf)
{
  static int idx = 0;

  // result is 0 when idx goes off the end of the queue
  int result = timerQueue.peekIdx(buf, idx);
  idx++;
  idx *= result;
  return result;
}

void setup()
{
  Wire.setTimeout(300);
#if DEBUG
  Serial.begin(115200);
  debugln("");
  debugln("--------Everything above this line is garbage on reset--------");
#endif

  segmentDisplay.init();
  progressBar.init();
  // screen.enabled = false;
  screen.init();
  initSensors();

  setPallete(5);

  timerQueue.flush(); // here now in case we wrap this in some kind of reset function later

  // Force first timer to be a reasonable value
  // queueTimer(timerQueue,TimerStatus::WORK, 30);
  // queueTimer(timerQueue,TimerStatus::BREAK, 30);
  queueTimer(timerQueue, TimerStatus::WORK, 6);
  queueTimer(timerQueue, TimerStatus::BREAK, 6);
  // queueTimer(timerQueue,TimerStatus::WORK, 4);
  // queueTimer(timerQueue,TimerStatus::BREAK, 4);
  // queueTimer(timerQueue,TimerStatus::WORK, 4);
  // queueTimer(timerQueue,TimerStatus::BREAK, 4);
  // queueTimer(timerQueue,TimerStatus::WORK, 4);
  // queueTimer(timerQueue,TimerStatus::BREAK, 4);

  manager.loadNextTimer();
  manager.start();

  // TODO: Screen update is slow.
  //  on screen redraw, we lag about 250ms updating the progressbar, causing a jerky first LED
  screen.update();

  debugln("--------");
  char initBuffer[20];
  sprintf(initBuffer, "%-13s%s", "Digits", segmentDisplay.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "ProgressBar", progressBar.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "Screen", screen.enabled ? "enabled" : "DISABLED");
  debugln(initBuffer);
  sprintf(initBuffer, "%-13s%s", "Sensors", sensorsEnabled ? "enabled" : "DISABLED");
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