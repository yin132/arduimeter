# cmake_minimum_required(VERSION <specify CMake version here>)
project(MS5611_custom)

set(CMAKE_CXX_STANDARD 14)

include_directories(MS5611-master)
include_directories(MS5611-master/examples)
include_directories(MS5611-master/examples/MS5611data2lcd)
include_directories(MS5611-master/examples/MS5611data2serial)
include_directories(MS5611-master/examples/MS5611data2serialWfilter)

add_executable(MS5611_custom
        MS5611-master/examples/MS5611data2lcd/MS5611data2lcd.ino
        MS5611-master/examples/MS5611data2serial/MS5611data2serial.ino
        MS5611-master/examples/MS5611data2serialWfilter/MS5611data2serialWfilter.ino
        MS5611-master/MS5611.cpp
        MS5611-master/MS5611.h
        MS5611-master/MS5611Sample.cpp
        MS5611-master/MS5611Sample.h
        MS5611-master/README.md)
