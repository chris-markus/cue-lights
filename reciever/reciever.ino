// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <CueLightsCommon.h>

const int ledPins[3][2] = {3,9,5,10,6,11};

void setup() {
  // put your setup code here, to run once:
  for (int i=0; i<3; i++) {
    pinMode(ledPins[i][0], OUTPUT);
    pinMode(ledPins[i][1], OUTPUT);
  }
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  // set recieve mode
  digitalWrite(2, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  clc::flashLED(ledPins[0][0], 1000, 255);
  clc::flashLED(ledPins[0][1], 1000, 255);
  clc::flashLED(ledPins[1][0], 500, 255);
  clc::flashLED(ledPins[1][1], 500, 255);
  clc::flashLED(ledPins[2][0], 250, 255);
  clc::flashLED(ledPins[2][1], 250, 255);
}
