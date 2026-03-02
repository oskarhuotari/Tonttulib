// flash_dump_menu_csv_nextptr.ino
//
// EEPROM pointer semantics (IMPORTANT):
// - EEPROM stores the **NEXT WRITE ADDRESS** (byte address).
//   Example: after page 10 is fully written, EEPROM stores (page 11)*256.
// - Reset pointer means writing 0x00000000 (next write = page 0).
//
// Startup behavior:
//
// Flash contains data?
//   Ask: ERASE? (y/n)
//      y -> erase flash log region + reset pointer + preflight
//      n -> Ask: dump? (y/n)
//            y -> dump mode (CSV, waits ACK/NACK per record)
//            n -> idle
//
// Notes:
// - This is a bench tool: erasing and dumping are blocking (OK here).
// - Flash operations require Write Enable before EACH erase/program.
// - Dump prints a CSV header and then one record per line.
//   After each line, type "ACK" to continue or "NACK" to resend that same record.

#include <Arduino.h>
#include <Tonttulib.h>

Tonttulib tLib;

// ================= Flash layout =================
static constexpr uint32_t PAGE_SIZE      = 256;
static constexpr uint32_t LOG_START_PAGE = 0;

// 4k sectors in your Flash class
static constexpr uint32_t SECTOR_SIZE_BYTES = 4096;
static constexpr uint32_t PAGES_PER_SECTOR  = SECTOR_SIZE_BYTES / PAGE_SIZE; // 16

// ================= Telemetry record (MUST match logger) =================
#pragma pack(push, 1)
struct TelemetryRecord {
  uint32_t t_ms;

  int16_t ax_mg;
  int16_t ay_mg;
  int16_t az_mg;

  int16_t gx_cdps;
  int16_t gy_cdps;
  int16_t gz_cdps;

  int32_t pressure_pa;
  int16_t temp_cC;

  int32_t lat_e7;
  int32_t lon_e7;
  int32_t alt_cm;

  uint16_t vbat_mV;

  uint8_t reserved[28];
};
#pragma pack(pop)

static_assert(sizeof(TelemetryRecord) == 64, "TelemetryRecord must be 64 bytes.");
static constexpr uint32_t RECORD_SIZE      = sizeof(TelemetryRecord);
static constexpr uint32_t RECORDS_PER_PAGE = PAGE_SIZE / RECORD_SIZE; // 4

static uint8_t pageBuf[PAGE_SIZE];

// ================= Serial helpers =================

static String readLineBlocking() {
  // Blocking read of a non-empty line. Trims whitespace.
  while (true) {
    if (Serial.available()) {
      String s = Serial.readStringUntil('\n');
      s.trim();
      if (s.length() > 0) return s;
    }
    delay(1);
  }
}

static bool askYesNo(const __FlashStringHelper* prompt) {
  Serial.print(prompt);
  Serial.println(F(" (y/n)"));
  while (true) {
    const String s = readLineBlocking();
    if (s.equalsIgnoreCase("y") || s.equalsIgnoreCase("yes")) return true;
    if (s.equalsIgnoreCase("n") || s.equalsIgnoreCase("no")) return false;
    Serial.println(F("Please type y or n."));
  }
}

static bool waitAckOrNack() {
  // Returns true for ACK, false for NACK.
  while (true) {
    const String s = readLineBlocking();
    if (s.equalsIgnoreCase("ACK"))  return true;
    if (s.equalsIgnoreCase("NACK")) return false;
    Serial.println(F("Type ACK or NACK."));
  }
}

// ================= Flash / data helpers =================

static bool isAllFF(const uint8_t* buf, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    if (buf[i] != 0xFF) return false;
  }
  return true;
}

static bool isErasedRecord(const TelemetryRecord& r) {
  return isAllFF(reinterpret_cast<const uint8_t*>(&r), sizeof(TelemetryRecord));
}

static bool flashWaitReady(uint32_t timeoutMs = 60000) {
  // Bench tool: waiting is acceptable, but we still protect with a timeout.
  const uint32_t start = millis();
  while (tLib.flash.readBusyBit()) {
    if ((uint32_t)(millis() - start) > timeoutMs) return false;
    delay(1);
  }
  return true;
}

// ================= EEPROM pointer handling (NEXT WRITE ADDRESS) =================

static bool getNextWriteAddr(uint32_t& nextAddrOut) {
  uint8_t gen = 0, seq = 0;
  uint32_t addr = 0;
  if (!tLib.eeprom.readLatestFlashAddress(addr, gen, seq)) return false;
  nextAddrOut = addr; // NEXT write byte address
  return true;
}

static bool getLastWrittenPage(uint32_t& lastPageOut) {
  // If nextAddr == 0, nothing has been written yet.
  uint32_t nextAddr = 0;
  if (!getNextWriteAddr(nextAddr)) return false;
  if (nextAddr == 0) return false;

  const uint32_t nextPage = nextAddr / PAGE_SIZE;
  if (nextPage == 0) return false;

  lastPageOut = nextPage - 1;
  return true;
}

static void resetLogPointerToStart() {
  // Reset pointer means "next write address is 0"
  const uint32_t startAddr = LOG_START_PAGE * PAGE_SIZE; // 0
  tLib.eeprom.writeFlashAddress(startAddr);
  Serial.print(F("EEPROM pointer reset to addr 0x"));
  Serial.println(startAddr, HEX);
}

static bool flashContainsData() {
  // check if page 0 contains data
  if (!tLib.flash.readPage(LOG_START_PAGE, pageBuf)) return false;
  return !isAllFF(pageBuf, PAGE_SIZE);
}

// ================= Actions =================

static void enterPreflight() {
  tLib.eeprom.writeFlightState((uint8_t)Tonttulib::FlightState::PRELAUNCH);
  Serial.println(F("Entered PREFLIGHT."));
}

static bool eraseLogUpToPage(uint32_t lastPage) {
  // Erase sector-by-sector from sector 0 up to the sector containing lastPage.
  const uint32_t lastSector = lastPage / PAGES_PER_SECTOR;

  Serial.print(F("Erasing sectors 0.."));
  Serial.println(lastSector);

  for (uint32_t sector = 0; sector <= lastSector; sector++) {
    if (!flashWaitReady()) {
      Serial.println(F("ERROR: timeout waiting for flash ready (before erase)."));
      return false;
    }
    if (!tLib.flash.writeEnable()) {
      Serial.println(F("ERROR: writeEnable failed (before sector erase)."));
      return false;
    }
    if (!tLib.flash.sectorErase(sector)) {
      Serial.print(F("ERROR: sectorErase failed at sector "));
      Serial.println(sector);
      return false;
    }
    if (!flashWaitReady()) {
      Serial.println(F("ERROR: timeout waiting for flash ready (during erase)."));
      return false;
    }

    Serial.print(F("Erased sector "));
    Serial.println(sector);
  }

  return true;
}

static void printRecordCSV(const TelemetryRecord& r) {
  // Output order:
  // t_ms,temp_C,pressure_Pa,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,lat,lon,alt_m,vbat_V

  const float tempC  = r.temp_cC / 100.0f;

  const float ax_g   = r.ax_mg / 1000.0f;
  const float ay_g   = r.ay_mg / 1000.0f;
  const float az_g   = r.az_mg / 1000.0f;

  const float gx_dps = r.gx_cdps / 100.0f;
  const float gy_dps = r.gy_cdps / 100.0f;
  const float gz_dps = r.gz_cdps / 100.0f;

  const double lat   = r.lat_e7 / 1e7;
  const double lon   = r.lon_e7 / 1e7;
  const double alt_m = r.alt_cm / 100.0;

  const float vbat_V = r.vbat_mV / 1000.0f;

  Serial.print(r.t_ms);
  Serial.print(',');
  Serial.print(tempC, 2);
  Serial.print(',');
  Serial.print(r.pressure_pa);
  Serial.print(',');
  Serial.print(ax_g, 4);
  Serial.print(',');
  Serial.print(ay_g, 4);
  Serial.print(',');
  Serial.print(az_g, 4);
  Serial.print(',');
  Serial.print(gx_dps, 3);
  Serial.print(',');
  Serial.print(gy_dps, 3);
  Serial.print(',');
  Serial.print(gz_dps, 3);
  Serial.print(',');
  Serial.print(lat, 7);
  Serial.print(',');
  Serial.print(lon, 7);
  Serial.print(',');
  Serial.print(alt_m, 2);
  Serial.print(',');
  Serial.println(vbat_V, 3);
}

static void runDumpMode(uint32_t lastPage) {
  Serial.println();
  Serial.println(F("CSV dump mode."));
  Serial.println(F("After each CSV line, type ACK to continue or NACK to resend that same line."));
  Serial.println();

  Serial.println(F("t_ms,temp_C,pressure_Pa,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,lat,lon,alt_m,vbat_V"));
  Serial.println();

  Serial.print(F("Dumping pages "));
  Serial.print(LOG_START_PAGE);
  Serial.print(F(".."));
  Serial.println(lastPage);

  for (uint32_t page = LOG_START_PAGE; page <= lastPage; page++) {
    if (!tLib.flash.readPage(page, pageBuf)) {
      Serial.print(F("ERROR: Failed to read page "));
      Serial.println(page);
      return;
    }

    for (uint32_t i = 0; i < RECORDS_PER_PAGE; i++) {
      TelemetryRecord r;
      memcpy(&r, &pageBuf[i * RECORD_SIZE], RECORD_SIZE);

      // End-of-data convention: erased record (all 0xFF)
      if (isErasedRecord(r)) {
        Serial.println(F("# End of data (erased record)"));
        Serial.println(F("# Done"));
        return;
      }

      while (true) {
        printRecordCSV(r);
        if (waitAckOrNack()) break;
        Serial.println(F("# NACK received -> resending same record"));
      }
    }
  }

  Serial.println(F("# Done (reached end page)"));
}

static void enterIdle() {
  Serial.println(F("Idle"));
}

// ================= setup / loop =================

void setup() {
  Serial.begin(115200);

  const int status = tLib.init();
  if (status != 1) {
    Serial.print(F("Initialization failed with code: "));
    Serial.println(status);
    while (true) {}
  }

  tLib.led.blinkFast();

  Serial.println(F("\n--- Flash log menu ---"));

  if (!flashContainsData()) {
    Serial.println(F("Flash contains no data -> preflight."));
    resetLogPointerToStart();
    enterPreflight();
    return;
  }

  Serial.println(F("Flash contains data."));

  const bool doErase = askYesNo(F("ERASE?"));
  if (doErase) {
    uint32_t lastPage = 0;
    if (!getLastWrittenPage(lastPage)) {
      Serial.println(F("WARNING: No EEPROM last page; erasing sector 0 only."));
      lastPage = 0;
    }

    Serial.print(F("Last written page: "));
    Serial.println(lastPage);

    if (!eraseLogUpToPage(lastPage)) {
      Serial.println(F("Erase failed -> idle."));
      enterIdle();
      return;
    }

    resetLogPointerToStart();
    enterPreflight();
    return;
  }

  const bool doDump = askYesNo(F("dump?"));
  if (doDump) {
    uint32_t lastPage = 0;
    if (!getLastWrittenPage(lastPage)) {
      Serial.println(F("No last written page found -> nothing to dump."));
      enterIdle();
      return;
    }

    runDumpMode(lastPage);
    enterIdle();
    return;
  }

  enterIdle();
}

void loop() {
  // Bench tool; nothing to do continuously.
}