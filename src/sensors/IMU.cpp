#include "IMU.h"

IMU::IMU()
    : _spi(nullptr),
      _spiSettings(12000000, MSBFIRST, SPI_MODE0),
      _accSensitivity(4096.0f),
      _gyroSensitivity(32.8f)
{
}

uint8_t IMU::_spiRead(uint8_t reg)
{
    _spi->beginTransaction(_spiSettings);
    digitalWrite(CS_IMU, LOW);
    _spi->transfer(reg | 0x80);
    uint8_t val = _spi->transfer(0x00);
    digitalWrite(CS_IMU, HIGH);
    _spi->endTransaction();
    return val;
}

void IMU::_spiReadBytes(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    _spi->beginTransaction(_spiSettings);
    digitalWrite(CS_IMU, LOW);
    _spi->transfer(reg | 0x80);
    for (uint8_t i = 0; i < length; i++)
        buffer[i] = _spi->transfer(0x00);
    digitalWrite(CS_IMU, HIGH);
    _spi->endTransaction();
}

void IMU::_spiWrite(uint8_t reg, uint8_t val)
{
    _spi->beginTransaction(_spiSettings);
    digitalWrite(CS_IMU, LOW);
    _spi->transfer(reg & 0x7F);
    _spi->transfer(val);
    digitalWrite(CS_IMU, HIGH);
    _spi->endTransaction();
}

void IMU::_modifyRegister(uint8_t reg, uint8_t mask, uint8_t val)
{
    uint8_t current = _spiRead(reg);
    _spiWrite(reg, (current & ~mask) | (val & mask));
}

// --- Public API ---
bool IMU::works()
{
    return _spiRead(WHO_AM_I_REG) == WHO_AM_I_VAL;
}

bool IMU::init(SPIClass &spi)
{
    _spi = &spi;

    pinMode(CS_IMU, OUTPUT);
    digitalWrite(CS_IMU, HIGH);

    _spi->begin();

    if (!works())
        return false;

    _accSensitivity = 4096.0f; // ±8g
    _gyroSensitivity = 32.8f;  // ±1000 dps

    // Enable gyro + accel, set ODR and ranges
    _modifyRegister(0x4E, 0b00101111, 0b00001111); // gyro 4kHz, accel 2kHz
    _modifyRegister(0x4F, 0b11101111, 0b00100100); // gyro ±1000 dps
    _modifyRegister(0x50, 0b11101111, 0b01000101); // accel ±8g

    // // GYRO_CONFIG1: 2nd order UI filter (bits 3:2), 3rd order DEC2_M2 (bits 1:0)
    _modifyRegister(0x51, 0b00001111, 0b00000110);

    // // GYRO_ACCEL_CONFIG0: accel LPF setting ODR/40 (bits 7:4), gyro LPF ODR/40 (bits 3:0)
    _modifyRegister(0x52, 0b11111111, 0b01110111);

    delay(50);
    return true;
}

float IMU::readTemperature()
{
    uint8_t buf[2];
    _spiReadBytes(0x1D, buf, 2);
    int16_t raw = (int16_t)(buf[0] << 8 | buf[1]);
    return (raw / 132.48f) + 25.0f;
}

void IMU::readAccel(float &x, float &y, float &z)
{
    uint8_t buf[6];
    _spiReadBytes(0x1F, buf, 6);
    z = -((int16_t)(buf[0] << 8 | buf[1])) / _accSensitivity;
    x = -((int16_t)(buf[2] << 8 | buf[3])) / _accSensitivity;
    y = -((int16_t)(buf[4] << 8 | buf[5])) / _accSensitivity;
}

void IMU::readGyro(float &x, float &y, float &z)
{
    uint8_t buf[6];
    _spiReadBytes(0x25, buf, 6);
    z = -((int16_t)(buf[0] << 8 | buf[1])) / _gyroSensitivity;
    x = -((int16_t)(buf[2] << 8 | buf[3])) / _gyroSensitivity;
    y = -((int16_t)(buf[4] << 8 | buf[5])) / _gyroSensitivity;
}

void IMU::readAll(float &ax, float &ay, float &az, float &gx, float &gy, float &gz)
{
    uint8_t buf[12];
    // Read accel + gyro in one burst
    _spiReadBytes(0x1F, buf, 12);
    az = -((int16_t)(buf[0] << 8 | buf[1])) / _accSensitivity;
    ax = -((int16_t)(buf[2] << 8 | buf[3])) / _accSensitivity;
    ay = -((int16_t)(buf[4] << 8 | buf[5])) / _accSensitivity;
    gz = -((int16_t)(buf[6] << 8 | buf[7])) / _gyroSensitivity;
    gx = -((int16_t)(buf[8] << 8 | buf[9])) / _gyroSensitivity;
    gy = -((int16_t)(buf[10] << 8 | buf[11])) / _gyroSensitivity;
}