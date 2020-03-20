// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "macros.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define ENCODER_STEPS_PER_CLICK 4 // number of steps in the encoder reading per physical click

// serial interface for remote stations
#define STATION_SERIAL Serial2

#define FLASH_DELAY_ON 1000
#define FLASH_DELAY_OFF 300

// run panel light interrupt at 2000Hz: x = 65536-16MHz/256/2000Hz
#define TIMER_COUNTER_PRELOAD 65504

// pin definitions:
const int transmitPin = 2;
const int ledEnablePins[] = {LED_EN_0,LED_EN_1,LED_EN_2,LED_EN_3,LED_EN_4,LED_EN_5,LED_EN_6,LED_EN_7,LED_EN_8,LED_EN_9,LED_EN_10};
const int ledPwmPins[] = {11, 10, 9}; // r, g, b
const int statusLEDPins[] = {5,4,3};
const int encoderPins[] = {18,19};
const int encoderButtonPin = 53;
const int buttonStbyEnable = 23;
const int buttonGoEnable = 25; // stby row, go row
const int buttonPins[] = {47,45,43,41,39,37,35,33,31,29,27};

#endif