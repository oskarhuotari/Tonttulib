#include <Tonttulib.h>

Tonttulib tLib;

void setup() {
    Serial.begin(115200);

    int status = tLib.init();
    if (status != 1) {
        Serial.print("Initialization failed with code: ");
        Serial.println(status);
        while (true);
    }

    tLib.led.blinkFast();

    Serial.println("All sensors initialized!");
    Serial.println("Motors armed (1000 us).");

    delay(10000);

    // uint32_t lastAddr;
    // uint8_t gen, seq;
    // if (tLib.eeprom.readLatestFlashAddress(lastAddr, gen, seq)) {
    //     Serial.print(F("Recovered last flash address: 0x"));
    //     Serial.println(lastAddr, HEX);
    //     Serial.print(F("generation="));
    //     Serial.print(gen);
    //     Serial.print(F(" sequence="));
    //     Serial.println(seq);
    // } else {
    //     Serial.println(F("No valid flash log found"));
    // }
    float lat0 = 60.1234567f;
    float lon0 = 21.7654321f;
    float p0 = 101325.0f;

    int32_t lat0_e7 = (int32_t)(lat0 * 10000000.0f);
    int32_t lon0_e7 = (int32_t)(lon0 * 10000000.0f);
    int32_t p0_pa   = (int32_t)(p0 + (p0 >= 0 ? 0.5f : -0.5f));

    tLib.eeprom.writeReferenceData(lat0_e7, lon0_e7, p0_pa);
}

void loop() {

    // // Must be called every loop
    tLib.update();

    Serial.println("-----------------------");
    delay(100);
}
