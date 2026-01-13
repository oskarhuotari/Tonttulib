# Tonttulib

Tonttulib is a custom Arduino/Teensy library for **Tonttuboard**, a PCB designed for the Finnish CanSat 2026 competition.  
It provides easy access to the board’s sensors.

---

## Features & Sensor Usage

- **BMP388 barometer** – read pressure via `tLib.baro.readPressure()`  
- **IMU** – read accelerometer (`tLib.imu.readAccel(ax, ay, az)`) and gyroscope (`tLib.imu.readGyro(gx, gy, gz)`) over SPI  
- **SPI Flash** – read pages via `tLib.flash.readPage(page, buffer)`  and write them via `tLib.flash.writePage(page, buffer)`. Erase pages via `tLib.flash.eraseUpToPage(pageNumber)`
- **Thermistor** – read temperature via `tLib.readTemperature()`  
- **LDR** – read light voltage via `tLib.readLDRVoltage()`  
- **Status LED** – non-blocking control via `tLib.led`  
  - `on()`, `off()`, `blinkSlow()`, `blinkFast()`  

> **Note:** `tLib.update()` must be called in every `loop()` iteration to keep the LED and other non-blocking functions running.

---

## Initialization Return Codes

- `1` = All sensors initialized successfully  
- `-1` = BMP388 initialization failed  
- `-2` = IMU initialization failed  
- `-3` = Flash initialization failed  

---

## Author

Oskar Huotari  
CanSat Finland 2026
