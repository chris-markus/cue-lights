// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <CueLightsCommon.h>
#include <digitalWriteFast.h>

#define NUM_SWITCHES 4

const int ledPins[3][2] = {6,9,5,10,3,11};
const int transmitPin = 2;

const int ledBuiltinPin = 13;

//const int switchPins[4] = {4, 7, 8, 12};
// switch at spot 0 is LSB ... at 4 is MSB
CLCDebouncedButton switches[NUM_SWITCHES] = {CLCDebouncedButton(12, ACTIVE_LOW_PULLUP), 
                                             CLCDebouncedButton(8, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(7, ACTIVE_LOW_PULLUP),
                                             CLCDebouncedButton(4, ACTIVE_LOW_PULLUP)};

uint8_t dataBuffer[CLC_DATA_LEN];

void setup() {
  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, LOW);
  pinMode(ledBuiltinPin, OUTPUT);
  Serial.begin(CLC_DEFAULT_BAUD_RATE);
  for (int i=0; i<3; i++) {
    pinMode(ledPins[i][0], OUTPUT);
    pinMode(ledPins[i][1], OUTPUT);
  }
}

bool synchronized = false;
uint8_t syncCounter = 0;
unsigned long lastSuccessfulPacket = 0;
unsigned long lastForceDesync = 0;

void loop() {
  uint8_t ndx = 0;
  char rc = 0;
  bool shouldRespond = false;
  MessageProgressState state = STATE_START;
  bool willDesync = false;
  // the end packet symbol could show up in our color data so we protect
  // against it here and make sure we synchronize by sending a "break" sequence
  // of end symbols for at least the length of the data every few packets.
  while (rc != CLC_PKT_END || state == STATE_DATA) {
    // force a desynchronization if it has been a while since the last successful packet
    // AND it has been a while since we forced a desynchronization (to allow time to 
    // recieve a successful packet)
    if (synchronized &&
        millis() > lastSuccessfulPacket + STATION_DISCONNECT_TIMEOUT &&
        millis() > lastForceDesync + STATION_DISCONNECT_TIMEOUT)
    {
      syncCounter = 0;
      synchronized = false;
      lastForceDesync = millis();
    }
    if (Serial.available()) {
      rc = Serial.read();

      if (!synchronized && rc == CLC_PKT_END) {
        syncCounter++;
      }
      else if (!synchronized && rc != CLC_PKT_END) {
        if (syncCounter >= CLC_BREAK_LEN - 2) {
          synchronized = true;
        }
        syncCounter = 0;
      }

      switch(state) {
        case STATE_START:
          if (rc == CLC_PKT_START) {
            state = STATE_TYPE;
          }
          break;
        case STATE_TYPE:
          if (rc == PKT_TYPE_CONTROL) {
            state = STATE_DATA;
          }
          else if (rc == PKT_TYPE_STATUS) {
            state = STATE_STATUS;
          }
          else {
            state = STATE_END;
          }
          break;
        case STATE_DATA:
          if (ndx < CLC_DATA_LEN) {
            dataBuffer[ndx++] = rc;
          }
          else {
            if (synchronized) {
              lastSuccessfulPacket = millis();
              updateLEDs();
            }
            state = STATE_END;
          }
          break;
        case STATE_STATUS:
          if (rc == getAddress()) {
            shouldRespond = true;
          }
          // desync when we reach station 10 so we force a re-sync
          if (rc == MAX_STATIONS - 1) {
            willDesync = true;
          }
          state = STATE_END;
          break;
        case STATE_END:
          // do nothing - the while loop will end us
          break;
      }
    }
    digitalWrite(ledBuiltinPin, synchronized);
  }

  if (shouldRespond) {
    digitalWriteFast(transmitPin, HIGH);
    delay(CLC_STATE_SWITCH_DELAY + 1);
    char buf[5];
    int len = 0;
    buf[len++] = CLC_PKT_START;
    buf[len++] = PKT_TYPE_RESPONSE;
    buf[len++] = getAddress();
    buf[len++] = (uint8_t)(synchronized);
    buf[len++] = CLC_PKT_END;
    Serial.write(buf, len);
    delay(1);
    digitalWriteFast(transmitPin, LOW);
    delay(CLC_STATE_SWITCH_DELAY);
  }

  if (willDesync) {
    synchronized = false;
    syncCounter = 0;
  }
}

uint8_t getAddress() {
  uint8_t address = 0;
  for(uint8_t i=0; i<NUM_SWITCHES; i++) {
    address += (uint8_t)(switches[i].isPressed())<<i;
  }
  return address;
}

void updateLEDs() {
  if (getAddress()-1 < 0 || 3*(getAddress()-1) + 2 >= MAX_STATIONS*3) {
    return;
  }
  for(int i=0, j=3*(getAddress()-1); i<3; i++, j++) {
    analogWrite(ledPins[i][0], dataBuffer[j]);
    analogWrite(ledPins[i][1], dataBuffer[j]);
  }
}
