#include "boardConfigs/config.h"
#include "debugSettings.h"
#include "sensors.h"
#include "manager.h"
#include <Arduino.h>

int8_t sensorDeltas[8] = {};
uint8_t touched = 0;
cppQueue buttonQueue(sizeof(uint8_t), 6);
bool sensorsEnabled = false;
extern TimerManager manager;
extern Adafruit_CAP1188 cap;
extern unsigned long currentMillis;
const char sensorIntervalInMS = 50;

void initSensors()
{

  // If MCU was soft reset, SDA line might be held low
  // so we just reset the cap1188 on boot no matter what

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
    while (1)
      ;
  }

  debugln("CAP1188 found!");
  sensorsEnabled = true;

  // debug("repeat rate enable: ");
  // debugln(cap.readRegister(repeat_rate_enable_register));
  // debug("sensitivity control: ");
  // debugln(cap.readRegister(sensitivity_control_register));
  // debug("input config: ");
  // debugln(cap.readRegister(sensor_input_config_register));
  // debug("input config 2: ");
  // debugln(cap.readRegister(sensor_input_config_2_register));
  // debug("config 2: ");
  // debugln(cap.readRegister(config_2_register));


  
}


//range 0-1023
int getAmbientBrightness()
{
  
    //TODO: process raw value into something useful

    return analogRead(LDR_PIN);
}

void getSensorInput()
{
  static unsigned long previousSensorMillis = 0;
  static uint8_t touchedPrev = 0;

  if (currentMillis - previousSensorMillis >= sensorIntervalInMS)
  {
    touched = cap.touched();
    previousSensorMillis = currentMillis;

    // No touch bits set, bail
    if (!touched || touched == touchedPrev)
    {
      return;
    }

    for (int i = 0; i < 8; i++)
    {
      if ((touched >> i & 1) ^ (touchedPrev >> i & 1))
      {
        buttonQueue.push(&i);
      }
    }
  }
  touchedPrev = touched;
};

bool processButtonQueue(cppQueue &pushTo)
{
  if (buttonQueue.isEmpty())
  {
    return false;
  }

  while (!buttonQueue.isEmpty())
  {
    uint8_t button;
    buttonQueue.pop(&button);
    switch (button)
    {

    case 4: // its pomodoro time!
      pushTo.flush();
      manager.queueTimer(TimerStatus::WORK, 25 * 60);
      manager.queueTimer(TimerStatus::BREAK, 5 * 60);
      manager.queueTimer(TimerStatus::WORK, 25 * 60);
      manager.queueTimer(TimerStatus::BREAK, 5 * 60);
      manager.queueTimer(TimerStatus::WORK, 25 * 60);
      manager.queueTimer(TimerStatus::BREAK, 5 * 60);
      manager.queueTimer(TimerStatus::WORK, 25 * 60);
      manager.queueTimer(TimerStatus::BREAK, 5 * 60);
      manager.queueTimer(TimerStatus::BREAK, 25 * 60);
      manager.loadNextTimer();
      manager.start();
      break;
    case 7:
      manager.updatePalette();
      break;

    default:
      if (pushTo.isFull())
      {
        debugln("Queue is full, no timer added");
        return false;
      }
      randomSeed(millis());
      manager.queueTimer((TimerStatus)random(2, 4), (buttonMap[button] + 1) * 60);
      break;
    }

    return true;
  };
};
