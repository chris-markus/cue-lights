// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <CueLightsCommon.h>
#include <SoftwareSerial.h>

#define NUM_SWITCHES 4

const int ledPins[3][2] = {6,9,5,10,3,11};
const int transmitPin = 2;

//const int switchPins[4] = {4, 7, 8, 12};
// switch at spot 0 is LSB ... at 4 is MSB
CLCDebouncedButton switches[NUM_SWITCHES] = {CLCDebouncedButton(12, ACTIVE_LOW_PULLUP), 
                                             CLCDebouncedButton(8, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(7, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(4, ACTIVE_LOW_PULLUP)};

CLCSerialClass CLCSerial(&Serial);

void setup() {
  //CLCSerial.init(false, 2);
  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, LOW);
  Serial.begin(CLC_DEFAULT_BAUD_RATE);
  // put your setup code here, to run once:
  for (int i=0; i<3; i++) {
    pinMode(ledPins[i][0], OUTPUT);
    pinMode(ledPins[i][1], OUTPUT);
  }
  pinMode(13, OUTPUT);
}

enum RecieveState {
  STATE_START,
  STATE_ADDRESS,
  STATE_TYPE,
  STATE_DATA,
  STATE_STATUS,
  STATE_END,
};

uint8_t ledColor[3] = {0,0,0};

void loop() {
  //static char buff[CLC_DATA_LEN];
  static bool awaitingAddr = false;
  static bool addressedToMe = false;
  static RecieveState state = STATE_START;
  static CLCPacketType type;
  //static int len = 0;
  static int dataCounter = 0;
  static bool colorValid = true;

  char newChar;
  if (Serial.available()) {
    newChar = Serial.read();
    //CLCSerial.write(newChar, 1);
    switch(state) {
      case STATE_START:
        //digitalWrite(13, HIGH);
        //Serial.println("start");
        if (((uint8_t)(newChar)) == CLC_PKT_START) {
          //len = 0;
          awaitingAddr = true;
          addressedToMe = false;
          dataCounter = 0;
          state = STATE_ADDRESS;
        }
        break;
      case STATE_ADDRESS:
        //Serial.println("addr");
        addressedToMe = (((uint8_t)(newChar)) == getAddress());
        state = STATE_TYPE;
        if (addressedToMe) {
          colorValid = false;
        }
        break;
      case STATE_TYPE:
        type = (CLCPacketType)(newChar);
        if (type == CONTROL) {
          state = STATE_DATA;
        }
        else if (type == STATUS) {
          state = STATE_STATUS;
        }
        break;
      case STATE_DATA:
        //Serial.println("data");
        //digitalWrite(13, addressedToMe);
        if (addressedToMe) {
          ledColor[dataCounter] = newChar;
        }
        dataCounter++;
        if (dataCounter >= CLC_DATA_LEN) {
          state = STATE_END;
        }
        break;
      case STATE_STATUS:
        if (addressedToMe) {
          digitalWrite(13, HIGH);
          digitalWrite(transmitPin, HIGH);
          delay(CLC_STATE_SWITCH_DELAY + 1);
          char buf[4];
          buf[0] = CLC_PKT_START;
          buf[1] = getAddress();
          buf[2] = RESPONSE;
          buf[3] = CLC_PKT_END;
          Serial.write(buf, 4);
          delay(1);
          digitalWrite(transmitPin, LOW);
          delay(CLC_STATE_SWITCH_DELAY);
        }
        state = STATE_END;
        break;
      case STATE_END:
        //Serial.println("end");
        digitalWrite(13, LOW);
        colorValid = true;
        addressedToMe = false;
        if(((uint8_t)(newChar)) == CLC_PKT_END) {
          state = STATE_START;
        }
        break;
    }
  }
  if (colorValid) {
    updateLEDs();
  }
  //Serial.println(getAddress());
}

uint8_t getAddress() {
  uint8_t address = 0;
  for(uint8_t i=0; i<NUM_SWITCHES; i++) {
    address += (uint8_t)(switches[i].isPressed())<<i;
  }
  return address;
}

void updateLEDs() {
  for(int i=0; i<3; i++) {
    analogWrite(ledPins[i][0], ledColor[i]);
    analogWrite(ledPins[i][1], ledColor[i]);
  }
}
