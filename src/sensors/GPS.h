#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>

class GPS
{
public:
  GPS();

  bool init(HardwareSerial &serial, uint32_t baud = 9600);
  void update();

  bool hasNewData();
  void clearNewData();

  float latitude();
  float longitude();
  float altitude();
  uint8_t fixType(); // 0 = no fix, 1 = fix
  uint8_t satellites();

private:
  HardwareSerial *_serial;
  TinyGPSPlus _gps;
  bool _newData;

  static const uint8_t SET_5HZ_RATE[];
};