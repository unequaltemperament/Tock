#include "debugSettings.h"
#include "boardConfigs/config.h"
#include <Arduino.h>
#include <Wire.h>
#include <cppQueue.h>
#include "manager.h"
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"

constexpr char NUM_LEDS= 36;
constexpr char NUM_DIGITS= 5;
constexpr char QUEUE_MAX_SIZE= 10;

unsigned long currentMillis;

TockTimer currentTimer;

cppQueue timerQueue(sizeof(TockTimer), QUEUE_MAX_SIZE);
ProgressBar progressBar(NUM_LEDS, LED_PIN);
SegmentDisplay segmentDisplay(NUM_DIGITS, DIGITS_PIN);
Screen screen(TFT_CS, TFT_DC, TFT_RST, TFT_LITE);
Adafruit_CAP1188 cap(CAP_RST);
TimerManager manager(segmentDisplay,progressBar,screen,timerQueue);

TockTimer generateTockTimer(TimerStatus status = WORK, long initialTimeInSeconds = 3600)
{
  TockTimer newTimer = TockTimer{status, initialTimeInSeconds};
  // TODO: do this in progressBar
  // either as a return value, or update when currentTimer is updated.
  // currently we couple "make any timer" and "set value based on current timer"
  // BAD NEWS BEARZ
  progressBar.lightIntervalInMs = (newTimer.initialTimeInMS / NUM_LEDS) / progressBar.partialSteps;
  return newTimer;
}

// this breaks after a single dequeue, I bet
int iterateNextInQueue(TockTimer *res)
{
  static int idx = 0;

  // result is 0 when idx goes off the end of the queue
  int result = timerQueue.peekIdx(res, idx);
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

  segmentDisplay.begin();
  segmentDisplay.clearAll();
  segmentDisplay.setBrightness(CAPPED_NEOPIXEL_BRIGHTNESS);

  progressBar.begin(); // INITIALIZE NeoPixel progressBar.updatedAt object (REQUIRED)
  progressBar.show();  // Turn OFF all pixels ASAP
  progressBar.setBrightness(CAPPED_NEOPIXEL_BRIGHTNESS * .25);

  screen.init();
  initSensors();

  // dummy testing data
  timerQueue.flush();
  for (int i = 0; i < QUEUE_MAX_SIZE; i++)
  {
    TockTimer t = generateTockTimer(static_cast<TimerStatus>((i % 2) + 1), random(300, 1000));
    timerQueue.push(&t);
  }

  // TODO: updating current timer needs to update/notify  segmentDisplay and progressBar
  timerQueue.pop(&currentTimer);

  currentMillis = millis();
  manager.startedAt = currentMillis;

  segmentDisplay.updatedAt = currentMillis;
  progressBar.updatedAt = currentMillis;
  segmentDisplay.forceUpdate();
  progressBar.forceUpdate();

  debug(millis());
  debugln(": End of setup, entering loop");
}

void loop()
{

  currentMillis = millis();

  getSensorInput();
  manager.update();
  screen.update(currentTimer, &iterateNextInQueue);
  //currentTimer.update();
  //segmentDisplay.update();
  //progressBar.update();
  //screen.update();
}