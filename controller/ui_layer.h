// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef UI_LAYER_H 
#define UI_LAYER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "strings.h"


class UIInterface: public Adafruit_SSD1306 {
  public:
    // address defaults to 0x3C because that is what mine is
    // supposedly adafruit displays are 0x3D...
    // if your display doesn't work after trying both, use this sketch to figure
    // out your display's i2c address: https://raw.githubusercontent.com/RuiSantosdotme/Random-Nerd-Tutorials/master/Projects/LCD_I2C/I2C_Scanner.ino
    UIInterface(uint16_t w, uint16_t h, TwoWire *twi=&Wire, uint8_t i2cAddr = 0x3C);

    void init();
    void showSplash();
    void printCentered(const char *string, bool centerVertically = true, uint16_t y_offset = 0);
    void printCentered(const String &string, bool centerVertically = true, uint16_t y_offset = 0);
    void update();
    void clear();
    
  private:
    uint8_t i2cAddr;
};

#endif
