//
// ms5611.h
// Library for barometric pressure and temperature sensor MS5611-01BA on I2C with arduino
//
// by yin132, 2021
// some functions by gronat, 2014

#include "MS5611.h"

#define OSR_P 5 // OSR for pressure in [1, 5]
#define OSR_T 2 // OSR for temperature in [1, 5]

// commands
#define CMD_RESET 0x1E // reset
#define CMD_ADC_READ 0x00 // ADC Read
#define CMD_CONV_D1_BASE 0x40 // convert D1 (digital pressure value) with base OSR
#define CMD_CONV_D2_BASE 0x50 // convert D2 (digital temperature value) with base OSR
#define CMD_PROM_READ_BASE 0xA2 // read first register for PROM Read

#define CONV_REG_SIZE 0x02 // size of register for each OSR for convert
#define PROM_REG_SIZE 0x02 // size of register for each address for PROM Read
#define NBYTES_CONV 3 // convert needs to return 24 bits (3 bytes) of data
#define NBYTES_PROM 2 // PROM Read needs to return 16 bits (2 bytes) of data


MS5611::MS5611() {
    // initialize values
    _TEMP = 2000;
    _P = 101315;
    for (uint8_t k = 0; k <= N_PROM_PARAMS; k++)
        _C[k] = 0;
}

void MS5611::begin() {
    Wire.begin();
    reset();
    readCalibration();
}

int32_t MS5611::getTemperature() {
    uint32_t D2;
    D2 = getRawTemperature();
    _dT = D2 - (((long) _C[5]) << 8); //update '_dT'
    // Below, 'dT' and '_C[6]'' must be casted in order to prevent overflow
    _TEMP = 2000 + (((int64_t) _dT * _C[6]) >> 23);

    // second order compensation
    if (_TEMP < 2000) {
        _TEMP -= ((_dT * _dT) >> 31);
    }

    return _TEMP;
}

int32_t MS5611::getPressure() {
    getTemperature(); //updates temperature _dT and _T
    uint32_t D1 = getRawPressure();
    // Below must have type casted to prevent overflow
    int64_t OFF = ((int64_t) _C[2] << 16)
                  + (((int64_t) _C[4] * _dT) >> 7);
    int64_t SENS = ((int64_t) _C[1] << 15)
                   + (((int64_t) _C[3] * _dT) >> 8);

    // second order compensation
    if (_TEMP < 2000) {
        int32_t correction = (5 * (_TEMP - 2000) * (_TEMP - 2000));
        OFF -=  correction >> 1;
        SENS -= correction >> 2;
        if (_TEMP < 1500) {
            correction = 7 * (_TEMP + 1500) * (_TEMP + 1500);
            OFF -= correction;
            SENS -= (11 * correction / 7) >> 1;
        }
    }

    _P = (((D1 * SENS) >> 21) - OFF) >> 15;
    return _P;
}

uint32_t MS5611::getRawPressure() {
    sendCommand(CMD_CONV_D1_BASE + (OSR_P - 1) * CONV_REG_SIZE); //read sensor, prepare a data
    delay(2 * OSR_P); //wait for delay based on datasheet
    sendCommand(CMD_ADC_READ); //get ready for reading the data
    return readnBytes(NBYTES_CONV); //reading the data
}

uint32_t MS5611::getRawTemperature() {
    sendCommand(CMD_CONV_D2_BASE + (OSR_T - 1) * CONV_REG_SIZE); //read sensor, prepare a data
    delay(2 * OSR_T); //wait for delay based on datasheet
    sendCommand(CMD_ADC_READ); //get ready for reading the data
    return readnBytes(NBYTES_CONV); //reading the data
}

void MS5611::readCalibration() {
    for (uint8_t k = 0; k < N_PROM_PARAMS; k++) {
        sendCommand(CMD_PROM_READ_BASE + k * NBYTES_PROM);
        _C[k+1] = (uint16_t)(readnBytes(NBYTES_PROM) & 0xFFFF); //masking out two LSB
    }
}

void MS5611::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(ADD_MS5611);
    Wire.write(cmd);
    Wire.endTransmission();
}

uint32_t MS5611::readnBytes(uint8_t nBytes) {
    Wire.beginTransmission(ADD_MS5611);
    Wire.requestFrom((uint8_t) ADD_MS5611, nBytes);
    uint32_t data = 0;
    if (Wire.available() != nBytes) {
        Wire.endTransmission();
        return NULL;
    }

    for (int8_t k = nBytes - 1; k >= 0; k--)
        data |= ((uint32_t) Wire.read() << (8 * k)); // concatenate bytes
    Wire.endTransmission();
    return data;
}

void MS5611::reset() {
    Wire.beginTransmission(ADD_MS5611);
    Wire.write(CMD_RESET);
    Wire.endTransmission();
    delay(100);
}