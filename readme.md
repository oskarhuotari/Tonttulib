# Tonttulib

**Tonttulib** is a custom Arduino/Teensy library for **Tonttuboard**, a PCB designed for the Finnish CanSat 2026 competition. It provides easy access to the board’s sensors, actuators, storage devices, and GPS with a clean interface.

---

## Features

### Sensors

| Sensor | Usage |
|--------|-------|
| **BMP388 Barometer** | Read pressure via: `tLib.baro.readPressure()` |
| **IMU** | Read accelerometer: `tLib.imu.readAccel(ax, ay, az)` <br>Read gyroscope: `tLib.imu.readGyro(gx, gy, gz)` |
| **Thermistor** | Read temperature in °C: `tLib.thermistor.readCelsius()` |
| **LDR** | Read light voltage (0–3.3V): `tLib.ldr.readVoltage()` |
| **GPS (UBX NAV-PVT)** | Non-blocking parsing of latitude, longitude, altitude, fix type, satellites, heading. <br>Usage: <br>`tLib.gps.update()` (called automatically in `tLib.update()`) <br>`if (tLib.gps.hasNewData()) { ... }` <br>`tLib.gps.clearNewData()` to reset the flag |
| **Read LiPo voltage** | Read battery voltage (V): `tLib.vlipo.readVoltage()` <br>Read battery charge percentage (0–100%): `tLib.vlipo.readPercentage()` |

### Actuators

| Actuator | Usage |
|----------|-------|
| **Motors (4x)** | Set ESC pulse width in µs (1000–2000): <br>`tLib.motors.set(index, us)` where `index = 1..4` |
| **LED** | Non-blocking control: <br>`tLib.led.on()`, `tLib.led.off()`, `tLib.led.blinkSlow()`, `tLib.led.blinkFast()` |

> **Note:** `tLib.update()` must be called in every `loop()` iteration. This will also update GPS and LED states.

### Storage

| Device | Usage |
|--------|-------|
| **SPI Flash** | Read pages: `tLib.flash.readPage(page, buffer)` <br>Write pages: `tLib.flash.writePage(page, buffer)` <br>Erase pages: `tLib.flash.eraseUpToPage(pageNumber)` |

---

## Initialization Return Codes

- `1` = All sensors initialized successfully  
- `-1` = BMP388 initialization failed  
- `-2` = IMU initialization failed  
- `-3` = Flash initialization failed  

---

## PCB design

- 4-layer PCB designed to turn Teensy 4.0 into a full-fledged custom flight controller  
- For more info see [PCB Repository](https://github.com/hasorez/Tonttuboard-PCB)

---

## Author

Oskar Huotari  
CanSat Finland 2026
