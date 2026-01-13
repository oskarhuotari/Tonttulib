#include <Tonttulib.h>

Tonttulib tLib;

void setup() {
    Serial.begin(115200);

    int status = tLib.init();
    if (status != 1) {
        Serial.print("Initialization failed with code: ");
        Serial.println(status);
        while(true);
    }
    tLib.led.begin();
    tLib.led.blinkFast();
    Serial.println("All sensors initialized!");
}

void loop() {
    float temp = tLib.readTemperature();
    float ldrV = tLib.readLDRVoltage();

    Serial.print("Temperature: "); Serial.print(temp, 2); Serial.println(" °C");
    Serial.print("LDR Voltage: "); Serial.print(ldrV, 2); Serial.println(" V");

    // --- BMP388 barometer ---
    float pressure = tLib.baro.readPressure();
    Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" Pa");

    // --- IMU ---
    float ax, ay, az;
    float gx, gy, gz;
    tLib.imu.readAccel(ax, ay, az);
    tLib.imu.readGyro(gx, gy, gz);

    Serial.print("Accel X: "); Serial.print(ax); Serial.print(" g, ");
    Serial.print("Y: "); Serial.print(ay); Serial.print(" g, ");
    Serial.print("Z: "); Serial.println(az);

    Serial.print("Gyro X: "); Serial.print(gx); Serial.print(" deg/s, ");
    Serial.print("Y: "); Serial.print(gy); Serial.print(" deg/s, ");
    Serial.print("Z: "); Serial.println(gz);

    // --- Flash ---
    uint8_t buffer[256];
    if (tLib.flash.readPage(0, buffer)) {
        Serial.print("First byte of flash page 0: 0x");
        Serial.println(buffer[0], HEX);
    }

    // must be called every loop
    tLib.update();

    Serial.println("-----------------------");
    delay(1000);
}
