#include "GPS.h"

const uint8_t GPS::SET_5HZ_RATE[] = {
    0xB5, 0x62, 0x06, 0x08, 0x06, 0x00,
    0xC8, 0x00, 0x01, 0x00, 0x01, 0x00,
    0xDE, 0x6A};

GPS::GPS() : _serial(nullptr), _newData(false) {}

bool GPS::init(HardwareSerial &serial, uint32_t baud)
{
  _serial = &serial;
  _serial->begin(baud);

  for (int i = 0; i < 3; i++)
  {
    _serial->write(SET_5HZ_RATE, sizeof(SET_5HZ_RATE));
    delay(100);
  }
  return true;
}

void GPS::update()
{
  if (!_serial)
    return;

  while (_serial->available())
  {
    if (_gps.encode(_serial->read()))
    {
      if (_gps.location.isValid())
      {
        _newData = true;
      }
    }
  }
}

bool GPS::hasNewData() { return _newData; }
void GPS::clearNewData() { _newData = false; }

float GPS::latitude() { return _gps.location.lat(); }
float GPS::longitude() { return _gps.location.lng(); }
float GPS::altitude() { return _gps.altitude.isValid() ? _gps.altitude.meters() : 0.0f; }
uint8_t GPS::fixType() { return _gps.location.isValid() ? 1 : 0; }
uint8_t GPS::satellites() { return _gps.satellites.isValid() ? _gps.satellites.value() : 0; }