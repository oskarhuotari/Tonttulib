/*
  Tonttulib.h - Library for interfacing with Tonttuboard:
    a custom PCB designed for the Finnish 2026 Cansat competition.
  Created by Oskar Huotari, 2026
*/
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "Constants.h"

// Sensors
#include "sensors/BMP388.h"
#include "sensors/IMU.h"
#include "sensors/Thermistor.h"
#include "sensors/LDR.h"
#include "sensors/GPS.h"
#include "sensors/VLiPo.h"

// Actuators
#include "actuators/LED.h"
#include "actuators/Motors.h"

// Storage
#include "storage/Flash.h"
#include "storage/EEPROM.h"

class Tonttulib
{
public:
  Tonttulib();

  int init(TwoWire &wire = Wire, SPIClass &spi = SPI);
  void update();

  enum FlightState : uint8_t
  {
    STATE_PRELAUNCH = 0,
    STATE_INSIDE_ROCKET,
    STATE_DEPLOYMENT,
    STATE_FREEFALL,
    STATE_POWERED,
    STATE_LANDED
  };

  // Convert enum to string
  static const char *flightStateToString(FlightState state)
  {
    switch (state)
    {
    case STATE_PRELAUNCH:
      return "PRELAUNCH";
    case STATE_INSIDE_ROCKET:
      return "INSIDE_ROCKET";
    case STATE_DEPLOYMENT:
      return "DEPLOYMENT";
    case STATE_FREEFALL:
      return "FREEFALL";
    case STATE_POWERED:
      return "POWERED";
    case STATE_LANDED:
      return "LANDED";
    default:
      return "UNKNOWN";
    }
  }

  // Modules
  BMP388 baro;
  IMU imu;
  Thermistor thermistor;
  LDR ldr;
  Motors motors;
  Led led;
  Flash flash;
  EEPROMStorage eeprom;
  GPS gps;
  VLiPo vlipo;

private:
  TwoWire *_i2c;
  SPIClass *_spi;
};
