// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef MULTI_LED_H
#define MULTI_LED_H

#include <stdint.h>
#include "Arduino.h"
#include <CueLightsCommon.h>

#define FLASH_DELAY_ON 1000
#define FLASH_DELAY_OFF 300

enum Color {
    OFF,
    RED,
    GREEN,
    BLUE,
};

class MultiLED {
public:
    MultiLED(int rPin_in, int gPin_in, int bPin_in, int count, int* pins_in);
    void setup();
    void tick();
    void setColor(int index, uint8_t r, uint8_t g, uint8_t b);
    void setColor(int index, RGBColor color);
    void setColor(int index, Color, float intensity = 1.0);
    void flash(int index);
    void stopFlash(int index);
    void allOff();
    void getColor(int index, uint8_t* r, uint8_t* g, uint8_t* b);
private:
    int rPin;
    int gPin;
    int bPin;
    int num_LED;
    int* pins;
    uint8_t** states;
    unsigned long lastFlashUpdate;
    bool flashState = true;
    bool* flashing;
};

#endif