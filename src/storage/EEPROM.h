#pragma once

#include "Constants.h"
#include <Arduino.h>
#include <EEPROM.h>

// Log entry layout
struct LogEntry
{
  uint8_t valid; // 0x42 = valid
  uint8_t generation;
  uint8_t sequence;
  uint32_t flashAddr;
};

class EEPROMStorage
{
public:
  EEPROMStorage() {}

  // Initialize EEPROM
  void begin()
  {
    EEPROM.begin();
  }

  // Flight state (stored at address 0)
  uint8_t readFlightState();
  void writeFlightState(uint8_t state);

  // Log management
  void writeFlashAddress(uint32_t addr);
  bool readLatestFlashAddress(uint32_t &addr, uint8_t &generation, uint8_t &sequence);

  void dumpLog(); // For debugging

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
