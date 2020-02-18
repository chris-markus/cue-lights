// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <CueLightsCommon.h>

#define NUM_SWITCHES 4

const int ledPins[3][2] = {3,9,5,10,6,11};

//const int switchPins[4] = {4, 7, 8, 12};
// switch at spot 0 is LSB ... at 4 is MSB
CLCDebouncedButton switches[NUM_SWITCHES] = {CLCDebouncedButton(12, ACTIVE_LOW_PULLUP), 
                                             CLCDebouncedButton(8, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(7, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(4, ACTIVE_LOW_PULLUP)};

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  for (int i=0; i<3; i++) {
    pinMode(ledPins[i][0], OUTPUT);
    pinMode(ledPins[i][1], OUTPUT);
  }
  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  // set recieve mode
  digitalWrite(2, LOW);
}

void loop() {
  static bool on = true;
  char address[5];
  int n = sprintf(address, "%d", getAddress());
  //Serial.println(getAddress());
  if (Serial.find(address, n)) {
    on = !on;
  }

  //Serial.println(switches[0].isPressed());
  digitalWrite(ledPins[0][0], on);
  digitalWrite(13, on);
  /*
  clc::flashLED(ledPins[0][0], 1000, 255);
  clc::flashLED(ledPins[0][1], 1000, 255);
  clc::flashLED(ledPins[1][0], 500, 255);
  clc::flashLED(ledPins[1][1], 500, 255);
  clc::flashLED(ledPins[2][0], 250, 255);
  clc::flashLED(ledPins[2][1], 250, 255);
  */
}

int getAddress() {
  int address = 0;
  for(int i=0; i<NUM_SWITCHES; i++) {
    address += (int)(switches[i].isPressed())<<i;
  }
  return address;
}
