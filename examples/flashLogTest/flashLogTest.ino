// flash_logger_1hz_drop.ino
//
// Logs one TelemetryRecord per second into flash.
//
// Design:
// - Records buffered into 256B page (4 records/page)
// - When page fills:
//      if flash ready -> start write
//      if flash busy  -> DROP PAGE (continue logging)
//
// Guarantees:
// - No blocking
// - No busy waits
// - Deterministic timing
// - Reset-safe (EEPROM updated only after successful write)

#include <Arduino.h>
#include <Tonttulib.h>

Tonttulib tLib;

// ================= Flash Layout =================

static constexpr uint32_t PAGE_SIZE = 256;
static constexpr uint32_t LOG_START_PAGE = 0;

// ================= Logging Rate =================

static constexpr uint32_t LOG_PERIOD_MS = 1000;

// ================= Telemetry Struct =================
//
// Exactly 64 bytes.
// Exactly 4 records fit in one page.
// No record ever crosses a page boundary.

#pragma pack(push,1)
struct TelemetryRecord
{
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
    // pid arvot
    // kulmat
    //state
};
#pragma pack(pop)

static_assert(sizeof(TelemetryRecord) == 64);

static constexpr uint32_t RECORD_SIZE = sizeof(TelemetryRecord);
static constexpr uint32_t RECORDS_PER_PAGE = PAGE_SIZE / RECORD_SIZE; // =4

// ================= Page Buffer =================

static uint8_t pageBuf[PAGE_SIZE];

static uint32_t currentPage = LOG_START_PAGE;
static uint8_t recordIndex = 0;

// ================= Flash State =================

static bool writeInProgress = false;
static uint32_t writingPage = 0;

// ================= Helpers =================

static int16_t clamp_i16(long v)
{
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

static uint16_t clamp_u16(long v)
{
    if (v < 0) return 0;
    if (v > 65535) return 65535;
    return (uint16_t)v;
}

// ================= Read Sensors =================

TelemetryRecord readTelemetry()
{
    TelemetryRecord r{};
    r.t_ms = millis();

    float ax,ay,az;
    float gx,gy,gz;

    tLib.imu.readAccel(ax,ay,az);
    tLib.imu.readGyro(gx,gy,gz);

    r.ax_mg = clamp_i16(ax*1000);
    r.ay_mg = clamp_i16(ay*1000);
    r.az_mg = clamp_i16(az*1000);

    r.gx_cdps = clamp_i16(gx*100);
    r.gy_cdps = clamp_i16(gy*100);
    r.gz_cdps = clamp_i16(gz*100);

    r.pressure_pa = tLib.baro.readPressure();
    r.temp_cC = clamp_i16(tLib.thermistor.readCelsius()*100);

    if(tLib.gps.fixType()>0)
    {
        r.lat_e7 = tLib.gps.latitude()*1e7;
        r.lon_e7 = tLib.gps.longitude()*1e7;
        r.alt_cm = tLib.gps.altitude()*100;
    }

    r.vbat_mV = clamp_u16(tLib.vlipo.readVoltage()*1000);

    return r;
}

// ================= EEPROM Recovery =================

void recoverPointer()
{
    uint32_t addr;
    uint8_t g, s;

    if (tLib.eeprom.readLatestFlashAddress(addr, g, s))
    {
        // addr is NEXT write address
        currentPage = addr / PAGE_SIZE;

        Serial.print("Recovered page ");
        Serial.println(currentPage);
    }
    else
    {
        currentPage = LOG_START_PAGE;
        Serial.println("Starting fresh log");
    }
}

// ================= Flash Service =================
//
// Nonblocking state machine

void serviceFlash()
{
    if (writeInProgress)
    {
        if (!tLib.flash.readBusyBit())
        {
            // Commit NEXT write address
            uint32_t nextAddr = (writingPage + 1) * PAGE_SIZE;
            tLib.eeprom.writeFlashAddress(nextAddr);

            writeInProgress = false;

            Serial.print("Page written ");
            Serial.println(writingPage);
        }
    }
}

// ================= Append Record =================

void appendRecord(const TelemetryRecord &r)
{
    uint32_t offset = recordIndex * RECORD_SIZE;

    memcpy(pageBuf+offset,&r,RECORD_SIZE);

    recordIndex++;

    if(recordIndex==RECORDS_PER_PAGE)
    {
        if(!tLib.flash.readBusyBit())
        {
            tLib.flash.writeEnable();

            tLib.flash.writePage(currentPage,pageBuf);

            writingPage=currentPage;
            writeInProgress=true;

            Serial.print("Writing page ");
            Serial.println(currentPage);

            currentPage++;
        }
        else
        {
            Serial.println("FLASH BUSY -> PAGE DROPPED");
        }

        recordIndex=0;
    }
}

// ================= Setup =================

void setup()
{
    Serial.begin(115200);

    int status=tLib.init();

    if(status!=1)
    {
        Serial.println("Init failed");
        while(1);
    }

    memset(pageBuf,0xFF,PAGE_SIZE);

    recoverPointer();

    Serial.println("Logger ready");
}

// ================= Loop =================

void loop()
{
    tLib.update();

    serviceFlash();

    static uint32_t lastLog=0;

    if(millis()-lastLog>LOG_PERIOD_MS)
    {
        lastLog=millis();

        TelemetryRecord r=readTelemetry();

        appendRecord(r);
    }
}