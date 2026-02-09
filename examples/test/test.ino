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
}

void loop() {
    float temp = tLib.thermistor.readCelsius();
    float ldrV = tLib.ldr.readVoltage();

    Serial.print("Temperature: ");
    Serial.print(temp, 2);
    Serial.println(" °C");

    Serial.print("LDR Voltage: ");
    Serial.print(ldrV, 2);
    Serial.println(" V");

    // --- BMP388 barometer ---
    float pressure = tLib.baro.readPressure();
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pa");

    // --- IMU ---
    float ax, ay, az;
    float gx, gy, gz;
    tLib.imu.readAccel(ax, ay, az);
    tLib.imu.readGyro(gx, gy, gz);

    Serial.print("Accel X: ");
    Serial.print(ax);
    Serial.print(" g, Y: ");
    Serial.print(ay);
    Serial.print(" g, Z: ");
    Serial.println(az);

    Serial.print("Gyro X: ");
    Serial.print(gx);
    Serial.print(" deg/s, Y: ");
    Serial.print(gy);
    Serial.print(" deg/s, Z: ");
    Serial.println(gz);

    // // --- Flash ---
    // uint8_t buffer[256];
    // if (tLib.flash.readPage(100, buffer)) {
    //     Serial.print("First byte of flash page 0: 0x");
    //     Serial.println(buffer[0], HEX);
    // }

    // --- GPS ---
    tLib.update();

    if (tLib.gps.hasNewData()) {
        Serial.print("Fix: ");
        Serial.print(tLib.gps.fixType());
        Serial.print(" Lat: ");
        Serial.print(tLib.gps.latitude(), 7);
        Serial.print(" Lon: ");
        Serial.print(tLib.gps.longitude(), 7);
        Serial.print(" Alt: ");
        Serial.println(tLib.gps.altitude(), 2);
        tLib.gps.clearNewData();
}


    // // --- Motors ---
    tLib.motors.set(1, 1200);
    tLib.motors.set(2, 1200);
    tLib.motors.set(3, 1200);
    tLib.motors.set(4, 1200);

    // // Must be called every loop
    tLib.update();

    Serial.println("-----------------------");
    delay(100);
}
