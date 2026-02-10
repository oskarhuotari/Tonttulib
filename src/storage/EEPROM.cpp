#include "EEPROM.h"

// Flight state
uint8_t EEPROMStorage::readFlightState()
{
  return EEPROM.read(EEPROM_STATE_ADDR);
}

void EEPROMStorage::writeFlightState(uint8_t state)
{
  EEPROM.update(EEPROM_STATE_ADDR, state);
}

// Write flash address (updates internal gen/seq)
void EEPROMStorage::writeFlashAddress(uint32_t addr)
{
  sequence++;
  if (sequence == 0)
  {
    generation++;
  }

  uint8_t slot = sequence % EEPROM_MAX_ENTRIES;

  LogEntry e;
  e.valid = 0x42;
  e.generation = generation;
  e.sequence = sequence;
  e.flashAddr = addr;

  EEPROM.put(slotAddress(slot), e);
}

// Read latest flash address
bool EEPROMStorage::readLatestFlashAddress(uint32_t &addr, uint8_t &gen, uint8_t &seq)
{
  bool found = false;
  for (uint8_t i = 0; i < EEPROM_MAX_ENTRIES; i++)
  {
    LogEntry e;
    EEPROM.get(slotAddress(i), e);
    if (e.valid != 0x42)
      continue;

    if (!found || isNewer(e.generation, gen) ||
        (e.generation == gen && isNewer(e.sequence, seq)))
    {
      gen = e.generation;
      seq = e.sequence;
      addr = e.flashAddr;
      found = true;
    }
  }

  // Update internal counters if found
  if (found)
  {
    generation = gen;
    sequence = seq;
  }

  return found;
}

// Debug helper
void EEPROMStorage::dumpLog()
{
  Serial.println(F("---- EEPROM LOG DUMP ----"));
  for (uint8_t i = 0; i < EEPROM_MAX_ENTRIES; i++)
  {
    LogEntry e;
    EEPROM.get(slotAddress(i), e);
    Serial.print(F("Slot "));
    Serial.print(i);
    Serial.print(F(": "));
    if (e.valid != 0x42)
    {
      Serial.println(F("INVALID"));
      continue;
    }
    Serial.print(F("VALID gen="));
    Serial.print(e.generation);
    Serial.print(F(" seq="));
    Serial.print(e.sequence);
    Serial.print(F(" addr=0x"));
    Serial.println(e.flashAddr, HEX);
  }
  Serial.println(F("-------------------------"));
}
