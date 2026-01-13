/*
  Tonttulib.cpp - Library for interfacing with Tonttuboard:
    a custom PCB designed by me for the Finnish 2026 CanSat competition.
  Created by Oskar Huotari, 10.1.2026
*/

#include "Arduino.h"
#include "Tonttulib.h"
#include <math.h>  // for pow()

Tonttulib::Tonttulib()
    : baro(), imu(), flash() {}

//  1  = success
// -1  = baro init failed
// -2  = IMU init failed
int Tonttulib::init(TwoWire &wire, SPIClass &spi)
{
    _i2c = &wire;
    _spi = &spi;

    // I2C init
    _i2c->begin();
    _i2c->setClock(400000);

    // SPI init
    _spi->begin();

    // Initialize BMP388
    if (!baro.init(*_i2c)) {
        return -1;
    }

    // Initialize IMU
    if (!imu.init(*_spi)) {
        return -2;
    }

    // Initialize Flash
    if (!flash.begin()) {
        return -3;
    }

    return 1;
}

// --- Temperature calculation ---  
float Tonttulib::readTemperature() {
    float Vout = (analogRead(A1)/1023.0f) * 3.3f;
    float Vcc = 3.3f;
    float Rseries = 10000.0f;

    // Thermistor is on the bottom now
    float R = Rseries * (Vcc - Vout) / Vout;

    float T0 = 298.15f;
    float R0 = 10000.0f;
    float beta = 3950.0f;

    float tempK = 1.0f / (1.0f/T0 + log(R/R0)/beta);
    return tempK - 273.15f;
}



// --- LDR voltage calculation ---
float Tonttulib::readLDRVoltage() {
    int raw = analogRead(A0);

    float voltage = (raw / 1023.0f) * 3.3f;
    return voltage;
}