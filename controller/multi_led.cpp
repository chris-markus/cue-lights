// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "multi_led.h"
#include "settings.h"

MultiLED::MultiLED(int rPin_in, int gPin_in, int bPin_in, int count, int* pins_in) {
    rPin = rPin_in;
    gPin = gPin_in;
    bPin = bPin_in;
    num_LED = count;
    pins = new int[num_LED];
    states = new uint8_t* [num_LED];
    flashing = new bool[num_LED];
    for (int i=0; i < num_LED; i++) {
        states[i] = new uint8_t[3];
        flashing[i] = false;
        pins[i] = pins_in[i];
        states[i][0] = 0;
        states[i][1] = 0;
        states[i][2] = 0;
    }
}

void MultiLED::setup() {
    pinMode(rPin, OUTPUT);
    pinMode(gPin, OUTPUT);
    pinMode(bPin, OUTPUT);
    for (int i=0; i<num_LED; i++) {
        pinMode(pins, OUTPUT);
    }
}

void MultiLED::tick() {
    bool flash = (bool)(Settings::getInstance()->getSettingWithName(SETTING_FLASH_STANDBY)->value);
    //Serial.println(Settings::getInstance()->getSettingWithName(SETTING_FLASH_STANDBY)->value);
    if (flash && flashState && millis() > lastFlashUpdate + FLASH_DELAY_ON || 
        !flashState && millis() > lastFlashUpdate + FLASH_DELAY_OFF) {
        flashState = !flashState;
        lastFlashUpdate = millis();
    }
    static int i = 0;
    digitalWrite(pins[i], LOW);
    i = (i + 1) % num_LED;
    bool flashOn = !flash || !flashing[i] || flashState;
    analogWrite(rPin, states[i][0]*flashOn);
    analogWrite(gPin, states[i][1]*flashOn);
    analogWrite(bPin, states[i][2]*flashOn);
    digitalWrite(pins[i], HIGH);
}

void MultiLED::setColor(int index, uint8_t r, uint8_t g, uint8_t b) {
    int brightness = 100;
    Setting* s = Settings::getInstance()->getSettingWithName(SETTING_PANEL_BRIGHTNESS);
    if (s != NULL) {
        brightness = s->value;
    }
    if (index >=0 && index < num_LED) {
        // add 1 if the brightness is not actually supposed to be 0
        states[index][0] = r*brightness/100 + (uint8_t)(r>0&&r<255);
        states[index][1] = g*brightness/100 + (uint8_t)(g>0&&g<255);
        states[index][2] = b*brightness/100 + (uint8_t)(b>0&&b<255);
    }
}

void MultiLED::setColor(int index, Color color, float intensity) {
    int r=0,g=0,b=0;
    switch (color) {
        case RED:
            r = 255*intensity;
            break;
        case GREEN:
            g = 100*intensity;
            break;
        case BLUE:
            b = 100*intensity;
            break;
        case OFF:
            break;
    }
    setColor(index, r, g, b);
}

void MultiLED::setColor(int index, RGBColor color) {
    setColor(index, color.r, color.g, color.b);
}

// TODO: this should really just be done in the tick function
void MultiLED::flash(int index) {
    flashing[index] = true;
}

void MultiLED::stopFlash(int index) {
    flashing[index] = false;
}

void MultiLED::allOff() {
    for (int i=0; i<num_LED; i++) {
        states[i][0] = 0;
        states[i][1] = 0;
        states[i][2] = 0;
        analogWrite(gPin, 0);
        analogWrite(rPin, 0);
        analogWrite(bPin, 0);
    }
}

void MultiLED::getColor(int index, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (index >=0 && index < num_LED) {
        *r = states[index][0];
        *g = states[index][1];
        *b = states[index][2];
    }
}