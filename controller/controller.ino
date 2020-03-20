// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

// Required libraries:
// Encoder: Arduino Package Manager
// Wire.h: Arduino Package Manager
// Adafruit_SSD1306: Arduino Package Manager
// EEPROM: included with Arduino
// Adafruit_GFX: Arduino Package Manager
// digitalWriteFast: install from here: https://github.com/NicksonYap/digitalWriteFast
// CueLightsCommon: In git repo (follow instructions in README)

// include libraries
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Wire.h>
#include <EEPROM.h>
#include <digitalWriteFast.h>

// include our source files
#include <CueLightsCommon.h>
#include "strings.h"
#include "lcd_screen.h"
#include "ui_controller.h"
#include "menu.h"
#include "settings.h"
#include "constants.h"
#include "station.h"

// uncomment this line to allow debug printing
 #define DEBUG

// some global objects
CLCDebouncedButton encoderButton(encoderButtonPin, ACTIVE_LOW_PULLUP);
LCDScreen screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Encoder encoder(encoderPins[0], encoderPins[1]);
// references to our UI controllers
NavigationController* navigation;
UIController* interface;
// panel buttons
CLCDebouncedButton stationButtons[(MAX_STATIONS + 1)][2] = {47,47,45,45,43,43,41,41,39,39,37,37,35,35,33,33,31,31,29,29,27,27};

// global status
enum GlobalStatusEnum {
  BUSY,
  OK,
  FAULT,
} GlobalStatus = BUSY;

// states for cue light communication state machine
enum ControllerState {
  STATION_REQUEST,
  STATION_RESPONSE,
  SEND_COLOR,
  BREAK,
  WAIT,
};

// Interrupt Service Routine to update panel LEDs every 0.5ms and ensure no flickering
// If you use pixel-mapped leds, this will no longer be neccessary!
ISR(TIMER4_OVF_vect) {
  TCNT4 = TIMER_COUNTER_PRELOAD;
  static int i = 0;
  analogWrite(ledPwmPins[0], 0);
  analogWrite(ledPwmPins[1], 0);
  analogWrite(ledPwmPins[2], 0);
  // use a macro to write using digitalWriteFast
  digitalWriteLEDEnable(i, LOW);
  i = (i + 1) % (MAX_STATIONS + 1);
  digitalWriteLEDEnable(i, HIGH);
  analogWrite(ledPwmPins[0], panelIndicators[i].r);
  analogWrite(ledPwmPins[1], panelIndicators[i].g);
  analogWrite(ledPwmPins[2], panelIndicators[i].b);
}

unsigned long birthTime;

void setup() {
  // pin setup that isn't taken care of by classes/other functions:
  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin,LOW);
  for (int i=0; i<3; i++) {
    pinMode(statusLEDPins[i], OUTPUT);
  }
  updateGlobalStatusLED();

  pinMode(buttonStbyEnable, OUTPUT);
  pinMode(buttonGoEnable, OUTPUT);

  #ifdef DEBUG
  Serial.begin(CLC_DEFAULT_BAUD_RATE);
  #endif

  // initialize settings and stations (cue light boxes):
  InitSettings();
  InitStations();

  // begin serial connection(s)
  STATION_SERIAL.begin(CLC_DEFAULT_BAUD_RATE);

  // setup ISR and change pwm frequency
  noInterrupts();
  // setup ISR on timer 4 to run face panel light code
  TCCR4A = 0;
  TCCR4B = 0;
  TIMSK4 |= (1 << TOIE1); // enable timer overflow interrupt
  TCCR4B |= (1 << CS12); // 256 prescaler
  TCNT4 = TIMER_COUNTER_PRELOAD; // preload timer 65536-16MHz/256/2Hz

  // increase PWM frequency of timers 1 and 2 to help reduce panel light flickering
  TCCR2B = TCCR2B & 0b11111000 | 0x01;
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  interrupts();

  // screen setup
  screen.init();
  screen.showSplash();

  // Screens created in menu-definitions.ino
  navigation = new NavigationController(&screen, &homeScreen);
  interface = new UIController(&encoder, &encoderButton, navigation);

  birthTime = millis();
}

bool initialSet = false;

void loop() {
  if (!initialSet && millis() > birthTime + 1000) {
    GlobalStatus = OK;
    initialSet = true;
  }
  detectButtonPresses();
  updateFacePanelLights();
  interface->tick();
  updateGlobalStatusLED();
  HandleCommunication();
}

void resetSettings() {
  resetDefaultSettings();
  navigation->goHome();
}

void getConnStatusString(char* input) {
  uint8_t count = 0;
  for(int i=0; i<MAX_STATIONS; i++) {
    count += stations[i].connStatus == CONNECTED;
  }
  sprintf(input, STR_LIGHTS_STATUS, count);
}

int getNextDisconnectedStation(int& numDisconnected) {
  static unsigned long lastUpdate = 0;
  const uint16_t updateInterval = 1500;
  static int currentStation = 0;
  bool connected[MAX_STATIONS];
  bool fault = false;
  numDisconnected = 0;
  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[currentStation].connStatus != CONNECTED 
        && stations[i].lastSeen != 0
        && millis() - stations[i].lastSeen > STATION_DISCONNECT_TIMEOUT 
        && millis() - stations[i].lastSeen < 100000)
    {
      if (millis() - stations[i].lastSeen < 10000) {
        fault = true;
      }
      connected[i] = false;
      numDisconnected++;
    }
    else {
      connected[i] = true;
    }
  }
  if (stations[currentStation].connStatus == CONNECTED || millis() > lastUpdate + updateInterval) {
    lastUpdate = millis();
    for (int i=currentStation+1; i<MAX_STATIONS + currentStation + 1; i++) {
      if (!connected[i%MAX_STATIONS]) {
        currentStation = i%MAX_STATIONS;
        break;
      }
    }
  }
  if (fault) {
    GlobalStatus = FAULT;
  }
  else {
    GlobalStatus = OK;
  }
  if (numDisconnected > 0) {
    return currentStation;
  }
  else {
    return -1;
  }
}

void getGlobalStatusString(char* input) {
  int numIssues = 0;
  int i = getNextDisconnectedStation(numIssues);
  if (i != -1) {
    sprintf(input, STR_ISSUES, numIssues);
  }
  else {
    sprintf(input, STR_NO_ISSUES);
  }
}

void getDescriptiveGlobalStatusString(char* input) {
  int numIssues = 0;
  int i = getNextDisconnectedStation(numIssues);
  if (i != -1) {
    sprintf(input, STR_STATION_DISCONNECTED, i+1, (millis() - stations[i].lastSeen)/1000);
  }
  else {
    sprintf(input, STR_NO_ISSUES);
  }
}

void detectButtonPresses() {
  // row 0 is standby, 1 is go
  static int buttonRow = false;
  static bool canSwap[MAX_STATIONS] = {1,1,1,1,1,1,1,1,1,1};

  digitalWrite(buttonStbyEnable, buttonRow);
  digitalWrite(buttonGoEnable, !buttonRow);

  // button press detection loop
  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[i].connStatus == CONNECTED) {
      CueStatus status = stations[i].cueStatus;
      if (stationButtons[i][buttonRow].isPressed()) {
        if (canSwap[i]) {
          if (status == NONE || buttonRow != (status == STBY)) {
            // if the status is none or if the status is not the same as the 
            // button we are pressing, set the status to correspond to the button
            stations[i].cueStatus = buttonRow?STBY:GO;
          }
          else {
            // otherwise, set it to NONE
            stations[i].cueStatus = NONE;
          }
          canSwap[i] = false;
        }
      }
      else if (stationButtons[i][buttonRow].wasReleased()) {
        canSwap[i] = true;
      }
    }
  }

  // master stby or go pressed
  if (stationButtons[MAX_STATIONS][buttonRow].isPressed()) {
    for (int i=0; i<MAX_STATIONS; i++) {
      switch (stations[i].cueStatus) {
        case STBY:
            stations[i].cueStatus = buttonRow?NONE:GO;
          break;
        case GO:
          if (buttonRow) {
            stations[i].cueStatus = NONE;
          }
          break;
        case NONE:
          break;
      }
    }
  }

  buttonRow = !buttonRow;
}

void updateFacePanelLights() {
  static bool flashOn = true;
  static unsigned long lastFlashChange = 0;

  if (flashOn && millis() > FLASH_DELAY_ON + lastFlashChange || 
      !flashOn && millis() > FLASH_DELAY_OFF + lastFlashChange)
  {
    flashOn = !flashOn;
    lastFlashChange = millis();
  }

  int stby = -1;
  int go = -1;
  int connected = -1;

  // color update loop
  for(int i=0; i< MAX_STATIONS; i++) {
    if (stations[i].connStatus == CONNECTED) {
      connected = i;
      if (stations[i].cueStatus == STBY) {
        stby = i;
        if (!flashOnStandby.value || flashOnStandby.value && flashOn) {
          stations[i].color = getScaledColor(RGBColor {standbyColor[0].value, standbyColor[1].value, standbyColor[2].value},
                                             stationBrightness[i].max,
                                             stationBrightness[i].value);
        }
        else {
          stations[i].color = {0,0,0};
        }
        panelIndicators[i] = getScaledColor(stations[i].color, panelBrightness.max, panelBrightness.value);
      }
      else if (stations[i].cueStatus == GO) {
        go = i;
        stations[i].color = getScaledColor(RGBColor {goColor[0].value, goColor[1].value, goColor[2].value},
                                           stationBrightness[i].max,
                                           stationBrightness[i].value);
        panelIndicators[i] = getScaledColor(stations[i].color, panelBrightness.max, panelBrightness.value);
      }
      else {
        stations[i].color = {0,0,0};
        panelIndicators[i] = {0,0,12};
      }
    }
    else {
      panelIndicators[i] = {0,0,0};
    }
  }

  if (stby != -1) {
    if (!flashOnStandby.value || flashOnStandby.value && flashOn) {
      panelIndicators[MAX_STATIONS] = getScaledColor(RGBColor {standbyColor[0].value, standbyColor[1].value, standbyColor[2].value},
                                                     panelBrightness.max,
                                                     panelBrightness.value);
    }
    else {
      panelIndicators[MAX_STATIONS] = {0,0,0};
    }
  }
  else if (go != -1) {
    panelIndicators[MAX_STATIONS] = getScaledColor(RGBColor {goColor[0].value, goColor[1].value, goColor[2].value},
                                                   panelBrightness.max,
                                                   panelBrightness.value);
  }
  else if (connected != -1) {
    panelIndicators[MAX_STATIONS] = panelIndicators[connected];
  }
  else {
    panelIndicators[MAX_STATIONS] = {0,0,0};
  }
}

RGBColor getScaledColor(RGBColor c, uint8_t max, uint8_t value) {
  c.r = (uint8_t)((uint16_t)(c.r)*(uint16_t)(value)/(uint16_t)(max));
  c.g = (uint8_t)((uint16_t)(c.g)*(uint16_t)(value)/(uint16_t)(max));
  c.b = (uint8_t)((uint16_t)(c.b)*(uint16_t)(value)/(uint16_t)(max));
  return c;
}

// Handles all communication with the cue light stations. See "CueLightsCommon.h" for 
// in-depth protocol descriptions.
void HandleCommunication() {
  static ControllerState controllerState = BREAK;
  static uint8_t responseStation = 0;
  int len = 0;
  char sendBuff[CLC_DATA_LEN + CLC_OVERHEAD]; // start + type + data + 2 end chars
  static char recvBuffer[CLC_STATUS_LEN + CLC_OVERHEAD + 1]; // start + type + synced + end
  static int recvIndex = 0;
  static unsigned long requestSent = 0;
  static unsigned long waitStart = 0;
  bool doneReceiving = false;
  char ch;

  switch(controllerState) {
    case SEND_COLOR:
      sendBuff[len++] = CLC_PKT_START;
      sendBuff[len++] = PKT_TYPE_CONTROL;
      for (int i=0; i<MAX_STATIONS; i++) {
        sendBuff[len++] = stations[i].color.r;
        sendBuff[len++] = stations[i].color.g;
        sendBuff[len++] = stations[i].color.b;
      }
      sendBuff[len++] = CLC_PKT_END;

      STATION_SERIAL.write(sendBuff, len);
      controllerState = STATION_REQUEST;
      break;
    case STATION_REQUEST:
      sendBuff[len++] = CLC_PKT_START;
      sendBuff[len++] = PKT_TYPE_STATUS;
      sendBuff[len++] = (char)(stations[responseStation].address);
      sendBuff[len++] = CLC_PKT_END;

      STATION_SERIAL.write(sendBuff, len);
      requestSent = millis();

      // need to make sure everything has been sent before we switch
      // no need to do this asynchronously since the delay is so short
      STATION_SERIAL.flush();
      delay(1);
      // ready to recieve
      digitalWriteFast(transmitPin, LOW);
      delay(1);

      controllerState = STATION_RESPONSE;
      break;
    case STATION_RESPONSE:
      if(STATION_SERIAL.available()) {
        ch = STATION_SERIAL.read();
        if (ch == CLC_PKT_START) {
          recvIndex = 0;
          recvBuffer[recvIndex++] = ch;
        }
        else if (recvIndex < CLC_STATUS_LEN + CLC_OVERHEAD) {
          recvBuffer[recvIndex++] = ch;
        }
      }

      // recieved packet: |START|TYPE|ADDRESS (as char)|SYNCHRONIZED (as char)|END|
      //         example: | '<' |'R' |      0x03       |         0x01         |'>'|
      // we care about the synchronized byte because the cue light will not accept color commands if it is not synchronized
      if (recvIndex == CLC_STATUS_LEN + CLC_OVERHEAD &&
          recvBuffer[0] == CLC_PKT_START &&
          recvBuffer[1] == PKT_TYPE_RESPONSE &&
          recvBuffer[2] == stations[responseStation].address &&
          recvBuffer[3] == 1) {
        stations[responseStation].connStatus = CONNECTED;
        stations[responseStation].lastSeen = millis();
        doneReceiving = true;
      }
      else if (millis() > requestSent + CLC_RESPONSE_TIMEOUT) {
        if (stations[responseStation].lastSeen < millis() - STATION_DISCONNECT_TIMEOUT) {
          stations[responseStation].connStatus = DISCONNECTED;
        }
        doneReceiving = true;
      }

      if (doneReceiving) {
        digitalWriteFast(transmitPin, HIGH);
        responseStation = (responseStation + 1) % MAX_STATIONS;
        recvIndex = 0;
        delay(1);
        controllerState = BREAK;
      }
      break;
    case BREAK:
      for (int i=0; i<CLC_BREAK_LEN; i++) {
        sendBuff[len++] = CLC_PKT_END;
      }
      STATION_SERIAL.write(sendBuff, len);
      waitStart = millis();
      controllerState = WAIT;
      break;
    case WAIT:
      if (millis() > waitStart + CLC_RESPONSE_DELAY) {
        controllerState = SEND_COLOR;
      }
      break;
  }
}

void updateGlobalStatusLED() {
  const int brightness = 3;
  switch(GlobalStatus) {
    case BUSY:
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[0], 0);
      flashLED(statusLEDPins[2], 50, brightness);
      break;
    case OK:
      analogWrite(statusLEDPins[0], brightness*1.5);
      analogWrite(statusLEDPins[1], brightness/2.0);
      analogWrite(statusLEDPins[2], brightness/1.5);
      break;
    case FAULT:
      flashLED(statusLEDPins[0], 100, 30);
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[2], 0);
      break;
  }
}

void flashLED(int pin, unsigned int delay, uint8_t brightness) {
    static unsigned long lastStateSwitch = 0;
    static bool lastState = false;
    unsigned long timeNow = millis();
    if (timeNow - delay > lastStateSwitch) {
        lastState = !lastState;
        analogWrite(pin, lastState?brightness:0);
        lastStateSwitch = timeNow;
    }
}