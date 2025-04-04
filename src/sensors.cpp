#include <Arduino.h>
#include "sensors.h"

int8_t sensorDeltas[9] = {};

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

void getSensorInput()
{
  static unsigned long previousSensorMillis = 0;

  uint8_t touched = cap.touched();
  uint8_t touchedPrev = 0;

  if (currentMillis - previousSensorMillis >= sensorIntervalInMS)
  {

    previousSensorMillis = currentMillis;

    // No touch bits set, bail
    if (!touched || touched == touchedPrev){
      return;
    }

    touchedPrev = touched;
    debug(touched);
    
    // for (uint8_t i = 0; i < 8; i++)
    // {
    //   sensorDeltas[i] = cap.readRegister(0x10 + i);
    //   /*       if (touched & (1 << i)) */

    //   debug(sensorDeltas[i]);
    //   if (i < 7)
    //   {
    //     debug(" | ");
    //   }
    // }
    // debugln();
  }
}
