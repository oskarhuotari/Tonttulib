#pragma once

#include "Constants.h"
#include <Arduino.h>
#include <EEPROM.h>

// Log entry layout
struct LogEntry
{
  uint8_t valid;
  uint8_t generation;
  uint8_t sequence;
  uint32_t flashAddr;
};

class EEPROMStorage
{
public:
  EEPROMStorage() {}

  void begin()
  {
    EEPROM.begin();
  }

  // Flight state
  uint8_t readFlightState();
  void writeFlightState(uint8_t state);

  // Flash log
  void writeFlashAddress(uint32_t addr);
  bool readLatestFlashAddress(uint32_t &addr, uint8_t &generation, uint8_t &sequence);

  // Reference values
  void writeLat0E7(int32_t lat0_e7);
  void writeLon0E7(int32_t lon0_e7);
  void writeP0Pa(int32_t p0_pa);

  int32_t readLat0E7();
  int32_t readLon0E7();
  int32_t readP0Pa();

  void writeReferenceData(int32_t lat0_e7, int32_t lon0_e7, int32_t p0_pa);
  void readReferenceData(int32_t &lat0_e7, int32_t &lon0_e7, int32_t &p0_pa);

  void dumpLog();

private:
  uint8_t generation = 0;
  uint8_t sequence = 0;

  uint16_t slotAddress(uint8_t slot)
  {
    return EEPROM_LOG_START + slot * sizeof(LogEntry);
  }

  bool isNewer(uint8_t a, uint8_t b)
  {
    return (uint8_t)(a - b) < 128;
  }
};