//
// ms5611.h
// Library for barometric pressure and temperature sensor MS5611-01BA on I2C with arduino
//
// by yin132, 2021
// some functions by gronat, 2014

#ifndef MS5611_h
#define MS5611_h

// Include Arduino libraries
#include "Arduino.h"
#include <Wire.h>

// address of the device MS5611
#define ADD_MS5611 0x77 // can be 0x76 if CSB pin is connected to GND
#define N_PROM_PARAMS 6 // number of parameters in PROM for calibration

class MS5611 {
public:
    // constructor
    MS5611();

    // initialize
    void begin();

    // get calibrated temperature in 10^-2 degrees celsius
    int32_t getTemperature();

    // get calibrated pressure in Pascals
    int32_t getPressure();

private:
    //variables
    int32_t _P; // Pressure in pascals
    int32_t _TEMP; // Temperature in 10^-2 degrees celsius
    int32_t _dT; // change in temperature in 10^-2 degrees celsius
    uint16_t _C[N_PROM_PARAMS + 1];

    // reset
    void reset();

    // get raw temperature reading
    uint32_t getRawTemperature();

    // get raw pressure reading
    uint32_t getRawPressure();

    // read the calibration values to _C
    void readCalibration();

    // send command to MS5611 with Wire
    void sendCommand(uint8_t);

    // read data from MS5611 with Wire
    uint32_t readnBytes(uint8_t);
};

#endif //MS5611_h
