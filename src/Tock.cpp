#include "debugSettings.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_CAP1188.h>
#include <cppQueue.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"

// #define TOCK_UNO
#define TOCK_EVERY

// nano every
#ifdef TOCK_EVERY

#define TFT_CS 14
#define TFT_DC 15
#define TFT_RST -1
#define TFT_LITE 10

#define CAP_RST 2

#define LED_PIN 7
#define DIGITS_PIN 5

#endif

// uno
#ifdef TOCK_UNO

#define TFT_CS 7
#define TFT_DC 9
#define TFT_RST 8
#define TFT_LITE 10

#define CAP_RST -1

#define LED_PIN 5
#define DIGITS_PIN 4

#endif

unsigned long currentMillis,
              startedAt = 0;

bool isRunning = false;

TockTimer currentTimer;

Screen tft(TFT_CS, TFT_DC, TFT_RST, TFT_LITE);

#define NUM_LEDS 36
ProgressBar progressBar(NUM_LEDS, LED_PIN, currentTimer);

#define QUEUE_SIZE_ITEMS 10

cppQueue timerQueue(sizeof(TockTimer), QUEUE_SIZE_ITEMS);

#define NUM_DIGITS 5
SegmentDisplay segmentDisplay(NUM_DIGITS, DIGITS_PIN, &progressBar, currentTimer, timerQueue);

Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP_RST);

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

void initSensors()
{

  // If MCU was soft reset, SDA line might be held low
  // so we just reset the cap1188 on boot no matter what
  //(technique commented out below doesn't work, not sure why the data line gets released
  // sometimes and then we get accidentally create/send a start condition
  // and then we're a different kind of stuck)

  pinMode(CAP_RST, OUTPUT);
  digitalWrite(CAP_RST, HIGH);
  delay(15);
  digitalWrite(CAP_RST, LOW);
  pinMode(CAP_RST, INPUT);
  delay(15);
  debugln("Setting up CAP1188...");
  bool capSensorDetected = cap.begin();


  while (!capSensorDetected)
  {
    static byte capSensorRetrys = 0;
    if (capSensorRetrys < 3)
    {
      capSensorRetrys++;
      debug("CAP1188 not found, retrying (attempt ");
      debug(capSensorRetrys);
      debugln("/3)");
      capSensorDetected = cap.begin();
      continue;
    }
    debugln("--------------------");
    debug("CAP1188 not found, aborting program");
    while (1);
  }

  debugln("CAP1188 found!");

  debug("repeat rate enable: ");
  debugln(cap.readRegister(repeat_rate_enable_register));
  debug("sensitivity control: ");
  debugln(cap.readRegister(sensitivity_control_register));
  debug("input config: ");
  debugln(cap.readRegister(sensor_input_config_register));
  debug("input config 2: ");
  debugln(cap.readRegister(sensor_input_config_2_register));
  debug("config 2: ");
  debugln(cap.readRegister(config_2_register));
}

int8_t sensorDeltas[9] = {};

void getSensorInput()
{
  static unsigned long previousSensorMillis = 0;

  uint8_t touched = cap.touched();

  if (currentMillis - previousSensorMillis >= sensorIntervalInMS)
  {

    previousSensorMillis = currentMillis;

    // No touch bits set, bail
    if (touched == 0)
    {
      return;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
      sensorDeltas[i] = cap.readRegister(0x10 + i);
      /*       if (touched & (1 << i)) */

      debug(sensorDeltas[i]);
      if (i < 7)
      {
        debug(" | ");
      }
    }
    debugln();
  }
}

// this breaks after a single dequeue, I bet
int iterateNextInQueue(TockTimer *res)
{
  static int idx = 0;

  // result is 0 when idx goes off the end of the queue
  int result = timerQueue.peekIdx(res, idx);
  idx++;
  idx *= result;
  dirty = static_cast<bool>(result);
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

  tft.init();
  initSensors();

  // dummy testing data
  for (int i = 0; i < QUEUE_SIZE_ITEMS; i++)
  {
    TockTimer t = generateTockTimer(static_cast<TimerStatus>((i % 2) + 1), random(300, 1000));
    timerQueue.push(&t);
  }

  // TODO: updating current timer needs to update/notify segmentDisplay and progressBar
  timerQueue.pop(&currentTimer);

  isRunning = true;
  startedAt = millis();
  currentMillis = millis();

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

  tft.updateDisplay(currentTimer, &iterateNextInQueue);

  if (isRunning)
  {
    currentMillis = millis();
    currentTimer.remainingTimeInMS = currentTimer.initialTimeInMS - (currentMillis - startedAt);
    getSensorInput();
    segmentDisplay.update();
    if (currentTimer.status != EXPIRE)
    {
      progressBar.update();
    }
  }
}