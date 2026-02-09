// huge thanks to @iforce2d on Youtube:
// https://www.youtube.com/watch?v=ylxwOg2pXrc

#include "GPS.h"

GPS::GPS()
    : _serial(nullptr),
      _newData(false),
      _fpos(0),
      _lastByteTime(0)
{
}

bool GPS::init(HardwareSerial &serial)
{
  _serial = &serial;
  return true;
}

void GPS::update()
{
  if (!_serial)
    return;

  while (_serial->available())
  {
    uint8_t c = _serial->read();
    if (processByte(c))
    {
      _newData = true;
    }
  }
}

bool GPS::hasNewData()
{
  return _newData;
}

void GPS::clearNewData()
{
  _newData = false;
}

uint8_t GPS::fixType()
{
  return _pvt.fixType;
}

uint8_t GPS::satellites()
{
  return _pvt.numSV;
}

float GPS::latitude()
{
  return _pvt.lat / 1e7f;
}

float GPS::longitude()
{
  return _pvt.lon / 1e7f;
}

float GPS::altitude()
{
  return _pvt.hMSL / 1000.0f;
}

float GPS::heading()
{
  return _pvt.heading / 1e5f;
}

void GPS::calcChecksum(uint8_t *ck)
{
  ck[0] = ck[1] = 0;
  for (uint16_t i = 0; i < sizeof(NAV_PVT); i++)
  {
    ck[0] += ((uint8_t *)(&_pvt))[i];
    ck[1] += ck[0];
  }
}

bool GPS::processByte(uint8_t c)
{
  uint32_t now = millis();

  if (_lastByteTime && (now - _lastByteTime) > BYTE_TIMEOUT_MS)
  {
    _fpos = 0;
  }
  _lastByteTime = now;

  const uint16_t payloadSize = sizeof(NAV_PVT);

  if (_fpos < 2)
  {
    if (c == UBX_HEADER[_fpos])
    {
      _fpos++;
    }
    else
    {
      _fpos = 0;
    }
    return false;
  }

  if ((_fpos - 2) < payloadSize)
  {
    ((uint8_t *)(&_pvt))[_fpos - 2] = c;
  }

  _fpos++;

  if (_fpos == payloadSize + 2)
  {
    calcChecksum(_checksum);
  }
  else if (_fpos == payloadSize + 3)
  {
    if (c != _checksum[0])
      _fpos = 0;
  }
  else if (_fpos == payloadSize + 4)
  {
    _fpos = 0;
    if (c == _checksum[1])
    {
      return true;
    }
  }
  else if (_fpos > payloadSize + 4)
  {
    _fpos = 0;
  }

  return false;
}
