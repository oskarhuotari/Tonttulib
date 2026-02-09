// huge thanks to @iforce2d on Youtube:
// https://www.youtube.com/watch?v=ylxwOg2pXrc

#pragma once
#include <Arduino.h>

class GPS
{
public:
  GPS();

  bool init(HardwareSerial &serial);
  void update();

  bool hasNewData();
  void clearNewData();

  // Accessors
  float latitude();
  float longitude();
  float altitude();
  uint8_t fixType();
  uint8_t satellites();
  float heading();

private:
  HardwareSerial *_serial;

  static constexpr uint32_t BYTE_TIMEOUT_MS = 250;
  static constexpr uint8_t UBX_HEADER[2] = {0xB5, 0x62};

  struct NAV_PVT
  {
    uint8_t cls;
    uint8_t id;
    uint16_t len;
    uint32_t iTOW;
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    int8_t valid;
    uint32_t tAcc;
    int32_t nano;
    uint8_t fixType;
    int8_t flags;
    uint8_t reserved1;
    uint8_t numSV;
    int32_t lon;
    int32_t lat;
    int32_t height;
    int32_t hMSL;
    uint32_t hAcc;
    uint32_t vAcc;
    int32_t velN;
    int32_t velE;
    int32_t velD;
    int32_t gSpeed;
    int32_t heading;
    uint32_t sAcc;
    uint32_t headingAcc;
    uint16_t pDOP;
    int16_t reserved2;
    uint32_t reserved3;
  };

  NAV_PVT _pvt;
  bool _newData;

  uint8_t _fpos;
  uint8_t _checksum[2];
  uint32_t _lastByteTime;

  void calcChecksum(uint8_t *ck);
  bool processByte(uint8_t c);
};
