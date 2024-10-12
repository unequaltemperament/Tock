#include <Arduino.h> 
#include <assert.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_CAP1188.h>
#include <RGBDigitV2.h>
#include "typeDefs.h"
#include "progressBar.h"
#include "segmentDisplay.h"

#include <cppQueue.h>



#define DEBUG 1
#define QUIET_TFT 0

#if QUIET_TFT
#define quiet_tft_param TestTFT::DISABLE
#else
#define quiet_tft_param TestTFT::DEFAULT
#endif

#if DEBUG
#define debug(...) Serial.print(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#else
#define debug(x)
#define debugln(...)
#endif

#define TFT_CS 7
#define TFT_RST 8  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 9
#define TFT_LITE 10
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

#define LED_PIN 5
#define NUM_LEDS 36
ProgressBar progressBar(NUM_LEDS, LED_PIN);

#define DIGITS_PIN 4
#define NUM_DIGITS 5
SegmentDisplay segmentDisplay(NUM_DIGITS, DIGITS_PIN, &progressBar);

#define CAPPED_NEOPIXEL_BRIGHTNESS 90
#define MAX_BACKLIGHT_BRIGHTNESS 127
#define BOOT_FADE_IN_TIME_MS 2000

#define QUEUE_SIZE_ITEMS 10

Adafruit_CAP1188 cap = Adafruit_CAP1188();

TockTimer currentTimer;

cppQueue timerQueue(sizeof(TockTimer), QUEUE_SIZE_ITEMS);

//storage variables for tft.getTextBounds()
int16_t xTB, yTB;
uint16_t wTB, hTB;

unsigned long currentMillis;
unsigned long previousMillis;
unsigned long startedAt = 0;

const int sensorInterval = 50;
uint32_t previousSensorMillis = 0;


bool isRunning = false;
bool dirty = true;

char timeInputBuffer[5];
int timeInputBufferIndex = 0;

TockTimer generateTockTimer(TimerStatus status = WORK, long initialTimeInSeconds = 3600) {
  TockTimer newTimer{ status, initialTimeInSeconds };
  //TODO: do this in progressBar
  //either as a return value, or update when currentTimer is updated.
  //currently we couple "make any timer" and "set value based on current timer"
  //BAD NEWS BEARZ
  progressBar.lightIntervalInMs = (newTimer.initialTimeInMS / NUM_LEDS) / progressBar.partialSteps;
  return newTimer;
}

bool timeInputBufferIsEmpty() {
  int s = sizeof(timeInputBuffer) / sizeof(timeInputBuffer[0]);
  for (int i = 0; i < s; i++) {
    if (timeInputBuffer[i] > 0) {
      return false;
    }
  }
  return true;
}

uint8_t touchMap(uint8_t i) {
  //byte touchMap[] = { 4, 8, 3, 7, 2, 6, 1, 5 };
  return (i % 2) ? 8 - ((i - 1) / 2) : 4 - (i / 2);
}

void initSensors() {
  debugln("Setting up CAP1188...");
  bool capSensorDetected = cap.begin();
  byte capSensorRetrys = 0;

  while (!capSensorDetected && capSensorRetrys < 3) {
    capSensorRetrys++;
    debug("CAP1188 not found, retrying (attempt ");
    debug(capSensorRetrys);
    debugln("/3)");
    capSensorDetected = cap.begin();
  }
  if (!capSensorDetected) {
    debugln("--------------------");
    debug("CAP1188 not found, aborting program");
    while (1)
      ;
  }
  //Wire.setWireTimeout();
  debugln("CAP1188 found!");

  Serial.print("repeat rate enable: ");
  Serial.println(cap.readRegister(repeat_rate_enable_register));
    Serial.print("sensitivity control: ");
  Serial.println(cap.readRegister(sensitivity_control_register));
    Serial.print("input config: ");
  Serial.println(cap.readRegister(sensor_input_config_register));
    Serial.print("input config 2: ");
  Serial.println(cap.readRegister(sensor_input_config_2_register));
    Serial.print("config 2: ");
  Serial.println(cap.readRegister(config_2_register));
  // cap.writeRegister(repeat_rate_enable_register, 0x00);
  // Serial.println(cap.readRegister(repeat_rate_enable_register));

}

void getSensorInput() {
  //uint8_t touchedNum;

  uint8_t touched;

  touched = cap.touched();
  if (currentMillis - previousSensorMillis >= sensorInterval) {

    previousSensorMillis = currentMillis;

    // No touch bits set, bail
    if (touched == 0) { return; }

    for (uint8_t i = 0; i < 8; i++) {
      if (touched & (1 << i)) {
        debug("C");
        //touchedNum = touchMap(i);
        debug(i+1);
        debug("\t");
      }
    }
    debugln();
  }
}

// //this breaks after a single dequeue, I bet
int iterateNextInQueue(TockTimer* res) {
  static int idx = 0;

  //result is 0 when idx goes off the end of the queue
  int result = timerQueue.peekIdx(res, idx);
  idx++;
  idx *= result;
  if (!result) dirty = false;
  return result;
}

static const byte splashImageData[1492] PROGMEM = {
  0x52, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 
  0x52, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 
  
  0x02, 0x90, 0x1A, 0x00, 0x02, 0x0B, 0x34, 0xFF, 0x00, 0x06, 0xFD,
  0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 0x02, 0x90, 0x1A, 0x00, 0x02,
  0x0B, 0x34, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00,
  0x00, 0x02, 0x90, 0x1A, 0x00, 0x02, 0x0B, 0x34, 0xFF, 0x00, 0x06, 0xFD,
  0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 0x02, 0x90, 0x1A, 0x00, 0x02,
  0x0B, 0x34, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00,
  0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00,
  0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02,
  0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x40, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00,
  0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00,
  0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02,
  0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x40, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00,
  0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00,
  0x06, 0xFD, 0x00, 0x02, 0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x40, 0xFF, 0x00, 0x06, 0xFD, 0x00, 0x02,
  0x00, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x10, 0xFF, 0x00, 0x0C, 0xD8, 0x50, 0x00, 0x00, 0x68, 0xEF, 0x12,
  0xFF, 0x00, 0x18, 0xFB, 0x83, 0x00, 0x00, 0x08, 0x8E, 0xFF, 0xFF, 0xFF,
  0xFD, 0x00, 0x02, 0x12, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x0E, 0xFF, 0x02, 0xC4, 0x0A, 0x00, 0x02, 0x05, 0x02,
  0xDF, 0x0E, 0xFF, 0x02, 0xFA, 0x02, 0x30, 0x0A, 0x00, 0x00, 0x0C, 0x4B,
  0xFF, 0xFF, 0xFD, 0x00, 0x02, 0x0A, 0xFF, 0x00, 0x08, 0x70, 0x00, 0x00,
  0x9F, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0C,
  0xFF, 0x02, 0xF7, 0x0E, 0x00, 0x02, 0x08, 0x0E, 0xFF, 0x02, 0x50, 0x0E,
  0x00, 0x00, 0x0A, 0x6F, 0xFF, 0xFD, 0x00, 0x02, 0x00, 0x08, 0xFF, 0x00,
  0x0A, 0xF6, 0x00, 0x00, 0x1C, 0xFF, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0C, 0xFF, 0x00, 0x14, 0x40, 0x00, 0x01,
  0x8C, 0xFF, 0xFB, 0x80, 0x00, 0x00, 0x6F, 0x0A, 0xFF, 0x00, 0x1C, 0xE3,
  0x00, 0x00, 0x48, 0xFF, 0xFF, 0x84, 0x00, 0x00, 0x3C, 0xFF, 0xFD, 0x00,
  0x02, 0x08, 0xFF, 0x00, 0x0A, 0x60, 0x00, 0x02, 0xCF, 0xFF, 0x00, 0x00,
  0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0A, 0xFF, 0x00,
  0x16, 0xF5, 0x00, 0x00, 0x5F, 0xFF, 0xFF, 0xFF, 0xFE, 0x40, 0x00, 0x07,
  0x00, 0x0A, 0xFF, 0x00, 0x06, 0x30, 0x00, 0x0A, 0x00, 0x08, 0xFF, 0x00,
  0x20, 0x90, 0x04, 0xEF, 0xFF, 0xFD, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0xF4,
  0x00, 0x00, 0x2D, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x0A, 0xFF, 0x00, 0x06, 0x80, 0x00, 0x08, 0x00, 0x0A,
  0xFF, 0x00, 0x16, 0xF6, 0x00, 0x00, 0xAF, 0xFF, 0xFF, 0xFF, 0xF8, 0x00,
  0x00, 0xCF, 0x00, 0x08, 0xFF, 0x00, 0x20, 0xFB, 0x5F, 0xFF, 0xFF, 0xFD,
  0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x40, 0x00, 0x03, 0xEF, 0xFF, 0xFF, 0x00,
  0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00,
  0x26, 0xFE, 0x10, 0x00, 0x5F, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0x30,
  0x00, 0x2F, 0xFF, 0xFF, 0xFF, 0xE1, 0x00, 0x08, 0x00, 0x12, 0xFF, 0x00,
  0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFF, 0xE4, 0x00, 0x00, 0x4E, 0xFF, 0xFF,
  0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08,
  0xFF, 0x00, 0x26, 0xF8, 0x00, 0x00, 0xDF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF,
  0xFF, 0xB0, 0x00, 0x09, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x1E, 0x00, 0x12,
  0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFE, 0x30, 0x00, 0x04, 0xFF,
  0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x08, 0xFF, 0x00, 0x26, 0xF2, 0x00, 0x06, 0xFF, 0xFF, 0xFF, 0xF0,
  0xFF, 0xFF, 0xFF, 0xF4, 0x00, 0x04, 0xFF, 0xFF, 0xFF, 0x20, 0x00, 0x7F,
  0x00, 0x12, 0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xE3, 0x00, 0x00,
  0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x26, 0xC0, 0x00, 0x0A, 0xFF, 0xFF,
  0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0xEF, 0xFF, 0xFC, 0x00,
  0x00, 0xCF, 0x00, 0x12, 0xFF, 0x00, 0x0E, 0xFD, 0x00, 0x02, 0xFD, 0x20,
  0x00, 0x06, 0x00, 0x0A, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x26, 0x90, 0x00, 0x0E, 0xFF, 0xFF,
  0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0xBF, 0xFF, 0xF9, 0x00,
  0x00, 0xEF, 0x00, 0x12, 0xFF, 0x00, 0x0E, 0xFD, 0x00, 0x02, 0xD2, 0x00,
  0x00, 0x8F, 0x00, 0x0A, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x0E, 0x80, 0x00, 0x2F, 0xFF, 0xFF,
  0xFF, 0xF0, 0x00, 0x08, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0x9F, 0xFF, 0xF8,
  0x00, 0x02, 0x00, 0x14, 0xFF, 0x00, 0x0C, 0xFD, 0x00, 0x01, 0x20, 0x00,
  0x08, 0x0C, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x08, 0xFF, 0x00, 0x0E, 0x80, 0x00, 0x2F, 0xFF, 0xFF, 0xFF, 0xF0,
  0x00, 0x08, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0x9F, 0xFF, 0xF8, 0x00, 0x02,
  0x00, 0x14, 0xFF, 0x00, 0x0C, 0xFD, 0x00, 0x01, 0x20, 0x00, 0x08, 0x0C,
  0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08,
  0xFF, 0x00, 0x0E, 0x70, 0x00, 0x2F, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x08,
  0xFF, 0x00, 0x0E, 0x00, 0x00, 0x8F, 0xFF, 0xF7, 0x00, 0x02, 0x00, 0x14,
  0xFF, 0x00, 0x0E, 0xFD, 0x00, 0x02, 0xD1, 0x00, 0x00, 0x8F, 0x00, 0x0A,
  0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08,
  0xFF, 0x00, 0x0E, 0x80, 0x00, 0x2F, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x08,
  0xFF, 0x00, 0x0E, 0x00, 0x00, 0x9F, 0xFF, 0xF8, 0x00, 0x02, 0x00, 0x14,
  0xFF, 0x00, 0x0E, 0xFD, 0x00, 0x02, 0xFC, 0x10, 0x00, 0x08, 0x00, 0x0A,
  0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08,
  0xFF, 0x00, 0x0C, 0x90, 0x00, 0x0E, 0xFF, 0xFF, 0xF0, 0x08, 0xFF, 0x00,
  0x12, 0xFC, 0x00, 0x00, 0xBF, 0xFF, 0xF9, 0x00, 0x00, 0xEF, 0x00, 0x12,
  0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xC1, 0x00, 0x00, 0x8F, 0xFF,
  0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D,
  0x00, 0x08, 0xFF, 0x00, 0x0C, 0xC0, 0x00, 0x0A, 0xFF, 0xFF, 0x0F, 0x08,
  0xFF, 0x00, 0x12, 0xF8, 0x00, 0x00, 0xEF, 0xFF, 0xFC, 0x00, 0x00, 0xAF,
  0x00, 0x12, 0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFC, 0x00, 0x00,
  0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0,
  0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x0A, 0xF2, 0x00, 0x06, 0xFF, 0xF0,
  0x00, 0x0A, 0xFF, 0x00, 0x12, 0xF4, 0x00, 0x04, 0xFF, 0xFF, 0xFF, 0x10,
  0x00, 0x6F, 0x00, 0x12, 0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFF,
  0xB0, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x08, 0xF8, 0x00, 0x00,
  0xDF, 0x0C, 0xFF, 0x00, 0x12, 0xB0, 0x00, 0x09, 0xFF, 0xFF, 0xFF, 0x70,
  0x00, 0x0D, 0x00, 0x12, 0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFF,
  0xFB, 0x00, 0x00, 0x08, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x08, 0xFF, 0x00, 0x08, 0xFE, 0x10, 0x00,
  0x5F, 0x0C, 0xFF, 0x00, 0x12, 0x30, 0x00, 0x2F, 0xFF, 0xFF, 0xFF, 0xD0,
  0x00, 0x05, 0x00, 0x12, 0xFF, 0x00, 0x18, 0xFD, 0x00, 0x02, 0xFF, 0xFF,
  0xFF, 0xA0, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0A, 0xFF, 0x00, 0x06, 0x80, 0x00, 0x08,
  0x00, 0x0A, 0xFF, 0x00, 0x16, 0xF6, 0x00, 0x00, 0xAF, 0xFF, 0xFF, 0xFF,
  0xF8, 0x00, 0x00, 0x8F, 0x00, 0x08, 0xFF, 0x00, 0x20, 0xFC, 0x4F, 0xFF,
  0xFF, 0xFD, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0xF9, 0x00, 0x00, 0x08, 0xFF,
  0xFF, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0A,
  0xFF, 0x00, 0x16, 0xF5, 0x00, 0x00, 0x5F, 0xFF, 0xFF, 0xFF, 0xFE, 0x40,
  0x00, 0x07, 0x00, 0x0A, 0xFF, 0x00, 0x06, 0x30, 0x00, 0x07, 0x00, 0x08,
  0xFF, 0x00, 0x0E, 0xA1, 0x04, 0xEF, 0xFF, 0xFD, 0x00, 0x02, 0x00, 0x08,
  0xFF, 0x00, 0x0A, 0x90, 0x00, 0x00, 0x8F, 0xFF, 0x00, 0x00, 0x00, 0x0C,
  0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0C, 0xFF, 0x00, 0x14, 0x40,
  0x00, 0x01, 0x8C, 0xFF, 0xFB, 0x80, 0x00, 0x00, 0x6F, 0x0A, 0xFF, 0x00,
  0x1C, 0xD2, 0x00, 0x00, 0x28, 0xFF, 0xFF, 0x84, 0x00, 0x00, 0x3E, 0xFF,
  0xFD, 0x00, 0x02, 0x08, 0xFF, 0x00, 0x0A, 0xF8, 0x00, 0x00, 0x08, 0xFF,
  0x00, 0x00, 0x00, 0x0C, 0xFF, 0x00, 0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0C,
  0xFF, 0x02, 0xF7, 0x0E, 0x00, 0x02, 0x08, 0x0C, 0xFF, 0x02, 0xFE, 0x02,
  0x40, 0x0E, 0x00, 0x00, 0x0A, 0x8E, 0xFF, 0xFD, 0x00, 0x02, 0x00, 0x0A,
  0xFF, 0x00, 0x08, 0x80, 0x00, 0x00, 0x8F, 0x00, 0x00, 0x0C, 0xFF, 0x00,
  0x06, 0xB0, 0x00, 0x0D, 0x00, 0x0E, 0xFF, 0x02, 0xC4, 0x0A, 0x00, 0x02,
  0x05, 0x02, 0xDF, 0x0E, 0xFF, 0x02, 0xF9, 0x02, 0x20, 0x0A, 0x00, 0x00,
  0x0C, 0x5C, 0xFF, 0xFF, 0xFD, 0x00, 0x02, 0x0A, 0xFF, 0x00, 0x08, 0xF8,
  0x00, 0x00, 0x06, 0x00, 0x00, 0x22, 0xFF, 0x00, 0x0C, 0xD8, 0x50, 0x00,
  0x00, 0x68, 0xEF, 0x12, 0xFF, 0x00, 0x0C, 0xFA, 0x82, 0x00, 0x00, 0x08,
  0x8E, 0x1E, 0xFF, 0x00, 0x01
};


//TODO: RENAME THIS!!!!!
unsigned int getNextChunk(byte numBytes = 2)
{
  static int idx = 0;
  unsigned int ret;
  if (numBytes == 1)
  {
    ret = pgm_read_byte(splashImageData + idx);
    idx++;
  }
  else
  {
    ret = pgm_read_word(splashImageData + idx);
    // Little-endian, swap the bytes around
    // so our consumer gets the individual bytes in order
    ret = (ret >> 8) | (ret << 8);
    idx += 2;
  }

  return ret;
}

void drawHighPixel(unsigned int colorByte, Bitmap& bmp) {

  //get just the high 4 bits & move them to the low 4 bits for 565'ing
  unsigned int highPixel = (colorByte & 0x00FF) >> 4;

  if (highPixel != 0x0F) {
    //RGB565
    highPixel = (highPixel << 1 | highPixel >> 3) << 11 | (highPixel << 2 | highPixel >> 2) << 5 | (highPixel << 1 | highPixel >> 3);

    // debug("highPixel: ");
    // debugln(highPixel, HEX);

    tft.drawPixel(bmp.x, bmp.y, highPixel);
  }

  bmp.advanceCursor();
}

void drawLowPixel(unsigned int colorByte, Bitmap& bmp) {

  //get just the low 4 bits
  unsigned int lowPixel = colorByte & 0x000F;

  if (lowPixel != 0x0F) {
    //RGB565
    lowPixel = (lowPixel << 1 | lowPixel >> 3) << 11 | (lowPixel << 2 | lowPixel >> 2) << 5 | (lowPixel << 1 | lowPixel >> 3);

    // debug("lowPixel: ");
    // debugln(lowPixel, HEX);

    tft.drawPixel(bmp.x, bmp.y, lowPixel);
  }

  bmp.advanceCursor();
}

void drawSplash()
{

  // TODO: hardcoded is sad
  // but current BMP compressed encoding
  // doesn't include image size
  // width = 106, height = 40
  Bitmap cursor = {106, 40, 0, 0};
  unsigned int scanPad;
  bool done = false;

  cursor.advanceCursorToImageStart(tft.width(), tft.height());
  long now = millis();

  while (!done)
  {
    scanPad = getNextChunk();
    if ((scanPad >> 8) == 0)
    {
      const int chunkLowByte = scanPad & 0x00FF;
      if (chunkLowByte <= 2)
      {
        // escape values
        switch (chunkLowByte)
        {
        // EOL, go to start of next line
        case 0:
          cursor.x = tft.width() / 2 - cursor.width / 2;
          cursor.y++;
          break;
        // EOF
        case 1:
          done = true;
          break;
        // Delta, following two bytes are horizontal & vertical
        // offsets to next pixel relative to current position
        case 2:
          scanPad = getNextChunk();
          cursor.x += scanPad >> 8;
          cursor.y += chunkLowByte;
          break;
        }
      }
      else
      {
        // absolute mode, scanPad is # of indexes
        // sorry i didn't say indices
        int numIndexes = chunkLowByte;
        for (int j = 0; j < numIndexes; j += 2)
        {
          scanPad = getNextChunk(1);
          drawHighPixel(scanPad, cursor);
          drawLowPixel(scanPad, cursor);
        }
        // adjust for word boundary alignment by chewing up padding byte
        if (numIndexes % 4 > 1)
        {
          getNextChunk(1);
        }
      }
    }

    else
    {
      // definitely encoded
      int repeatLength = scanPad >> 8;
      scanPad &= 0x00FF;

      int drawIndex = 0;

      while (drawIndex < repeatLength)
      {
        drawHighPixel(scanPad, cursor);
        drawIndex++;
        // odd number of pixels, bail early
        if (drawIndex == repeatLength)
        {
          continue;
        }
        drawLowPixel(scanPad, cursor);
        drawIndex++;
      }
    }
  }

  now = millis() - now;
  char doneText[16] = "done";
  int mspad = sprintf(doneText, "done: %li", now);
  sprintf(&doneText[mspad], "ms");

  tft.setTextSize(2);
  tft.getTextBounds(doneText, 0, 0, &xTB, &yTB, &wTB, &hTB);
  tft.setCursor(tft.width() / 2 - (wTB / 2), tft.height() - hTB - 10);
  tft.setTextColor(0x00);
  tft.print(doneText);
  while(1);
  debugln("splash done.");
}

void initDisplay(TestTFT mode = TestTFT::DEFAULT) {

  pinMode(TFT_LITE, OUTPUT);
  analogWrite(TFT_LITE, 0);
  tft.init(240, 320);
  if (mode == TestTFT::DISABLE) {
    tft.fillScreen(0x00);
    tft.enableSleep(true);
    tft.enableDisplay(false);
    return;
  }


  tft.setRotation(2);
  tft.fillScreen(0xFFFF);

  for (int i = 0; i < MAX_BACKLIGHT_BRIGHTNESS; i++) {
    analogWrite(TFT_LITE, i);
    if (i == MAX_BACKLIGHT_BRIGHTNESS) { break; }
    delay(BOOT_FADE_IN_TIME_MS / MAX_BACKLIGHT_BRIGHTNESS);
  }

   drawSplash();
  // delay(1200);

  tft.fillScreen(0x00);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  char title[] = { "Current:" };

  tft.getTextBounds(title, 0, 0, &xTB, &yTB, &wTB, &hTB);
  tft.setCursor(120 - (wTB / 2), 5);
  tft.print(title);
}

void updateDisplay() {
  TockTimer storage;
  char queued[] = { "Coming Up:" };
  if(dirty){
  int cursorStart = 30;
  tft.setTextColor(TimerColor[currentTimer.status], 0x0000);
  tft.setCursor(15, cursorStart);
  tft.print(statusType[currentTimer.status]);
  tft.print(" for ");
  tft.print(currentTimer.initialTimeInMS / 1000);

  tft.setTextColor(0xFFFF);
  tft.getTextBounds(queued, 0, 0, &xTB, &yTB, &wTB, &hTB);
  tft.setCursor(120 - (wTB / 2), 70);
  tft.print(queued);

  cursorStart = 95;

  int result = iterateNextInQueue(&storage);
  while (result && storage.status != 0 && dirty) {
    long curColor = TimerColor[storage.status];
    curColor = (curColor>>8&0xf800)|(curColor>>5&0x07e0)|(curColor>>3&0x001f);
    tft.setTextColor(curColor, 0x0000);
    tft.setCursor(15, cursorStart);
    tft.print(statusType[storage.status]);
    tft.print(" for ");
    tft.print(storage.initialTimeInMS / 1000);
    cursorStart += 25;
    // debug(storage.status);
    // debug(" for ");
    // debugln(storage.initialTimeInMS/1000);
    // debugln(result);
    result = iterateNextInQueue(&storage);
  }}
}

void setup() {



#if DEBUG
  Serial.begin(115200);
  debugln("");
  debugln("--------Everything above this line is garbage on reset--------");
  Serial.setTimeout(100);
#endif

  segmentDisplay.begin();
  segmentDisplay.clearAll();
  segmentDisplay.setBrightness(CAPPED_NEOPIXEL_BRIGHTNESS);

  progressBar.begin();  // INITIALIZE NeoPixel progressBar.updatedAt object (REQUIRED)
  progressBar.show();   // Turn OFF all pixels ASAP
  progressBar.setBrightness(CAPPED_NEOPIXEL_BRIGHTNESS * .25);

  initDisplay(quiet_tft_param);
  initSensors();

  //dummy testing data
  for (int i = 0;i<QUEUE_SIZE_ITEMS;i++) {
    TockTimer t = generateTockTimer(static_cast<TimerStatus>((i % 2)+1), random(300,1000));
    timerQueue.push(&t);
  }

  //TODO: updating current timer needs to update/notify segmentDisplay and progressBar
  timerQueue.pop(&currentTimer);

  segmentDisplay.queue = &timerQueue;
  segmentDisplay.currentTimer = &currentTimer;
  progressBar.currentTimer = &currentTimer;


  isRunning = true;
  currentMillis = millis();
  previousMillis = currentMillis;

  segmentDisplay.updatedAt = currentMillis;
  progressBar.updatedAt = currentMillis;
  segmentDisplay.forceUpdate();
  progressBar.forceUpdate();

  startedAt = millis();
  debug(millis());
  debugln(": End of setup, entering loop");
}

void loop() {

  currentMillis = millis();

  #if QUIET_TFT
  #else
  updateDisplay();
  #endif

  if (isRunning) {
    currentMillis = millis();
    //Serial.println(currentMillis - previousMillis);
    currentTimer.remainingTimeInMS = currentTimer.initialTimeInMS - (currentMillis - startedAt);
    //getSensorInput();
    segmentDisplay.update();
    if (currentTimer.status != EXPIRE) {
      progressBar.update();
    }
    previousMillis = currentMillis;
  }
}