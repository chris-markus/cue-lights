// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "CueLightsCommon.h"

void clc::flashLED(int pin, unsigned int delay, uint8_t brightness) {
    static unsigned long lastStateSwitch = 0;
    static bool lastState = false;
    unsigned long timeNow = millis();
    if (timeNow - delay > lastStateSwitch) {
        lastState = !lastState;
        analogWrite(pin, lastState?brightness:0);
        lastStateSwitch = timeNow;
    }
}