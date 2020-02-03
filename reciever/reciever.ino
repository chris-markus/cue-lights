// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <CueLightsCommon.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(4,OUTPUT);
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  clc::flashLED(4, 1000, 255);
  Serial.println("hi!");
  delay(1000);
}
