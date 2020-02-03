// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "ui_layer.h"
#include <String.h>

UIInterface::UIInterface(uint16_t w, uint16_t h, TwoWire *twi, uint8_t screen_i2cAddr) 
: Adafruit_SSD1306(w, h, twi, -1) {
  i2cAddr = screen_i2cAddr;
}

void UIInterface::init() {
  if(!begin(SSD1306_SWITCHCAPVCC, i2cAddr)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // set default text attributes
  setTextSize(1);
  setTextColor(WHITE);

  // clear the display
  clearDisplay();
}

void UIInterface::showSplash() {
  setTextColor(WHITE);
  printCentered(STR_SPLASH);
  invertDisplay(true);
  update();
  delay(500);
  invertDisplay(false);
  update();
}

void UIInterface::printCentered(const char *string, bool centerVertically, uint16_t y_offset) {
  int16_t x, y;
  uint16_t w, h;
  getTextBounds(string,0,0,&x,&y,&w,&h);
  x = (width() - w)/2;
  if (!centerVertically) {
    y = y_offset;
  }
  else {
    y = (height() - h)/2;
  }
  setCursor(x,y);
  println(string);
}

void UIInterface::printCentered(const String &string, bool centerVertically, uint16_t y_offset) {
  if (string.length() != 0) {
    printCentered(const_cast<char *>(string.c_str()), centerVertically, y_offset);
  }
}

// Some remapping
void UIInterface::update() {
  display();
}

void UIInterface::clear() {
  clearDisplay();
}

/*
void UIInterface::testLoop() {
  static bool clear = true;
  if (Serial.available()) {
    if (clear) {
      display.clearDisplay();
      clear = false;
    }
    display.write(Serial.read());
    display.display();
  }
  else{
    display.setCursor(0,0);
    clear = true;
  }
}*/
