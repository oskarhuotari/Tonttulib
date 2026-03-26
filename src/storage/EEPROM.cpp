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

// Write flash address
void EEPROMStorage::writeFlashAddress(uint32_t addr)
{
  sequence++;
  if (sequence == 0)
  {
    generation++;
  }

  uint8_t slot = sequence % EEPROM_MAX_ENTRIES;

  LogEntry e;
  e.valid = EEPROM_VALID_MARKER;
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

    if (e.valid != EEPROM_VALID_MARKER)
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

  if (found)
  {
    generation = gen;
    sequence = seq;
  }

  return found;
}

// Reference values
void EEPROMStorage::writeLat0E7(int32_t lat0_e7)
{
  EEPROM.put(EEPROM_LAT0_ADDR, lat0_e7);
}

void EEPROMStorage::writeLon0E7(int32_t lon0_e7)
{
  EEPROM.put(EEPROM_LON0_ADDR, lon0_e7);
}

void EEPROMStorage::writeP0Pa(int32_t p0_pa)
{
  EEPROM.put(EEPROM_P0_ADDR, p0_pa);
}

int32_t EEPROMStorage::readLat0E7()
{
  int32_t v;
  EEPROM.get(EEPROM_LAT0_ADDR, v);
  return v;
}

int32_t EEPROMStorage::readLon0E7()
{
  int32_t v;
  EEPROM.get(EEPROM_LON0_ADDR, v);
  return v;
}

int32_t EEPROMStorage::readP0Pa()
{
  int32_t v;
  EEPROM.get(EEPROM_P0_ADDR, v);
  return v;
}

void EEPROMStorage::writeReferenceData(int32_t lat0_e7, int32_t lon0_e7, int32_t p0_pa)
{
  EEPROM.put(EEPROM_LAT0_ADDR, lat0_e7);
  EEPROM.put(EEPROM_LON0_ADDR, lon0_e7);
  EEPROM.put(EEPROM_P0_ADDR, p0_pa);
}

void EEPROMStorage::readReferenceData(int32_t &lat0_e7, int32_t &lon0_e7, int32_t &p0_pa)
{
  EEPROM.get(EEPROM_LAT0_ADDR, lat0_e7);
  EEPROM.get(EEPROM_LON0_ADDR, lon0_e7);
  EEPROM.get(EEPROM_P0_ADDR, p0_pa);
}

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

    if (e.valid != EEPROM_VALID_MARKER)
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

  Serial.print(F("lat0_e7 = "));
  Serial.println(readLat0E7());
  Serial.print(F("lon0_e7 = "));
  Serial.println(readLon0E7());
  Serial.print(F("p0_pa = "));
  Serial.println(readP0Pa());

  Serial.println(F("-------------------------"));
}