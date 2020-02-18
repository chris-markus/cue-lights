// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

// Required libraries:
// Encoder: Arduino Package Manager
// Wire.h: Arduino Package Manager
// Adafruit_SSD1306: Arduino Package Manager
// Adafruit_GFX: Arduino Package Manager
// CueLightsCommon: In git repo (follow instructions in README)

// include libraries
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Wire.h>
//#include <DMXSerial.h>

// include our source files
#include <CueLightsCommon.h>
#include "strings.h"
#include "lcd_screen.h"
#include "ui_controller.h"
#include "menu.h"
#include "multi_led.h"
#include "remote_station.h"
#include "settings.h"
#include "constants.h"

// some global objects
CLCDebouncedButton encoderButton(53, ACTIVE_LOW_PULLUP);
LCDScreen screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Encoder encoder(18,19);
CLCSerialClass CLCSerial(&Serial2);

// global status
const int statusLEDPins[3] = {5,4,3};
enum GlobalStatusEnum {
  BUSY,
  OK,
  FAULT,
};
GlobalStatusEnum GlobalStatus = BUSY;

const int ledPins[MAX_STATIONS + 1] = {42,40,38,36,34,32,30,28,26,24,22};
MultiLED channelLEDs(11, 10, 9, MAX_STATIONS + 1, ledPins); // rPin, gPin, bPin, numLED, pins

// references to our UI controllers
NavigationController* navigation;
UIController* interface;

// array of remote station objects
RemoteStation stations[MAX_STATIONS] = {1,2,3,4,5,6,7,8,9,10};

// settings
Setting panelBrightness = {SETTING_PANEL_BRIGHTNESS, 90, 100, 10, VALUE};
Setting flashOnStandby = {SETTING_FLASH_STANDBY, 1, 1, 0, BOOL};

void addSettings() {
  // allocate the settings object and tell it we want 2 settings
  Settings::alloc(2);

  // add pointers to our settings
  Settings::getInstance()->add(&panelBrightness);
  Settings::getInstance()->add(&flashOnStandby);
}

// button setup
//int buttonPins[MAX_STATIONS + 1] = {47,45,43,41,39,37,35,33,31,29,27};
const int btnGrnRowEn = 25;
const int btnRedRowEn = 23;
CLCDebouncedButton stationButtons[(MAX_STATIONS + 1)][2] = {47,47,45,45,43,43,41,41,39,39,37,37,35,35,33,33,31,31,29,29,27,27};

// forward function declarations:
void updateFacePanelLights();

// Menu helper functions:
void testAll() {
  screen.clear();
  //TODO: make this work as one line and make it a string constant
  int16_t offset = screen.printCentered("Running test, press");
  screen.printCentered("encoder to stop", false, offset + 2);
  screen.display();
  loopUntilEncoderIsPressedAgain(rainbow);
  channelLEDs.allOff();
  interface->displayCurrentMenu();
}

void testOne() {
  screen.setTextColor(WHITE);
  channelLEDs.allOff();
  loopUntilEncoderIsPressedAgain(testOneHelper);
  interface->displayCurrentMenu();
}

void testOneHelper() {
  static int station = 0;
  static long offset = encoder.read();
  static bool go = true;
  long reading = encoder.read();
  if (reading >= offset + ENCODER_STEPS_PER_CLICK) {
    channelLEDs.setColor(station, OFF);
    station++;
    offset = reading;
  }
  else if (reading <= offset - ENCODER_STEPS_PER_CLICK) {
    channelLEDs.setColor(station, OFF);
    station --;
    offset = reading;
  }
  if (station < 0) 
    station = 0;
  else if (station > 10 - 1) 
    station = 9;

  rainbow(station);

  screen.clear();
  int16_t yOff = screen.printCentered("Testing station:");
  char tmp[3];
  sprintf(tmp, "%d", station);
  screen.printCentered(tmp, false, yOff + 2);
  screen.display();
}

void loopUntilEncoderIsPressedAgain(void (*callback)()) {
  while (encoderButton.isPressed())
  {
    callback();
  }
  while(!encoderButton.isPressed()) {
    callback();
  }
}

void adjustSetting(Setting* s) {
  screen.setTextColor(WHITE);
  bool releasedOnce = encoderButton.isPressed();
  bool stop = false;
  long offset = encoder.read();
  int value = s->value;
  while (!stop) {
    bool pressed = encoderButton.isPressed();
    if (pressed && !releasedOnce) {
      stop = true;
    }
    if (!pressed && releasedOnce) {
      releasedOnce = false;
    }
    long reading = encoder.read();
    value += (reading - offset)/ENCODER_STEPS_PER_CLICK;
    // add the remainder back on to keep track of those half steps
    offset = reading - (reading - offset)%ENCODER_STEPS_PER_CLICK;
    if (value < s->min) 
      value = s->min;
    else if (value > s->max) 
      value = s->max;

    s->value = value;

    screen.clear();
    int16_t yOff = screen.printCentered(s->name);
    char tmp[4];
    switch (s->type) {
      case VALUE:
        sprintf(tmp, "%d", value);
        break;
      case BOOL:
        sprintf(tmp, s->value?STR_ON:STR_OFF);
    }
    screen.printCentered(tmp, false, yOff + 2);
    screen.display();
    // TODO: this really shouldn't go here...
    updateFacePanelLights();
  }
  Settings::getInstance()->save(s);
  interface->displayCurrentMenu();
}

void adjustBrightness() {
  adjustSetting(&panelBrightness);
}

void adjustFlash() {
  adjustSetting(&flashOnStandby);
}

// TODO: add this into the UI controller - it needs to be updated dynamically
void showHomeScreen() {
  
}

// back buttons are automatically added
Menu mainMenu( "", 5,
  new HeaderItem(STR_MENU_MAIN, showHomeScreen),
  new Menu(STR_PANEL_INDICATORS, 2,
    new MenuItem(SETTING_PANEL_BRIGHTNESS, adjustBrightness),
    new MenuItem(SETTING_FLASH_STANDBY, adjustFlash)
  ),
  new Divider(),
  new Menu(STR_TEST_STATIONS, 2,
    new MenuItem(STR_TEST_ALL_STATIONS, testAll),
    new MenuItem(STR_TEST_ONE_STATION, testOne)
  ),
  new Menu("Color", 2,
    new MenuItem("Standby Color"),
    new MenuItem("Go Color")
  )
);

void updateFacePanelLights() {
  // row 0 is standby, 1 is go
  static int buttonRow = false;
  //static bool secondRelease = false;
  digitalWrite(btnRedRowEn, buttonRow);
  digitalWrite(btnGrnRowEn, !buttonRow);

  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[i].getConnStatus() == CONNECTED) {
      // we are checking the stby buttonRow
      if (buttonRow && stationButtons[i][buttonRow].isPressed()) {
          stations[i].setCueStatus(STBY);
      }
      // TODO: make this work
      // if we were in standby and pressed the standby button, clear the standby on the button release
      /*else if (buttonRow && stations[i].getCueStatus()==STBY && stationButtons[i][buttonRow].wasReleased()) {
        if (secondRelease) {
          stations[i].setCueStatus(NONE);
          secondRelease = false;
        }
        else {
          secondRelease = true;
        }
      }*/
      // we are checking the go buttonRow
      else if ((!buttonRow) && stationButtons[i][buttonRow].isPressed()) {
        stations[i].setCueStatus(GO);
      }
    }
  }

  // master stby or go pressed
  if (stationButtons[MAX_STATIONS][buttonRow].isPressed()) {
    for (int i=0; i<MAX_STATIONS; i++) {
      switch (stations[i].getCueStatus()) {
        case STBY:
            stations[i].setCueStatus(buttonRow?NONE:GO);
          break;
        case GO:
          if (buttonRow) {
            stations[i].setCueStatus(NONE);
          }
          break;
        case NONE:
          break;
      }
    }
  }

  buttonRow = !buttonRow;

  bool someStby = false;
  bool someGo = false;
  bool someConnected = false;
  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[i].getCueStatus() == STBY) {
      /*channelLEDs.setColor(i, RED);
      channelLEDs.flash(i);*/
      someStby = true;
    }
    else if (stations[i].getCueStatus() == GO) {
      /*channelLEDs.setColor(i, GREEN);
      channelLEDs.stopFlash(i);*/
      someGo = true;
    }
    else if (stations[i].getConnStatus() == CONNECTED) {
      /*channelLEDs.setColor(i, BLUE, 0.05);
      channelLEDs.stopFlash(i);*/
      someConnected = true;
    }
    else {
      /*channelLEDs.setColor(i, OFF, 0.0);
      channelLEDs.stopFlash(i);*/
    }
  }

  if (someStby) {
    channelLEDs.setColor(MAX_STATIONS, RED);
    channelLEDs.flash(MAX_STATIONS);
  }
  else if (someGo) {
    channelLEDs.setColor(MAX_STATIONS, GREEN);
    channelLEDs.stopFlash(MAX_STATIONS);
  }
  else if (someConnected) {
    channelLEDs.setColor(MAX_STATIONS, BLUE, 0.05);
    channelLEDs.stopFlash(MAX_STATIONS);
  }
  else {
    channelLEDs.setColor(MAX_STATIONS, OFF);
    channelLEDs.stopFlash(MAX_STATIONS);
  }

  for(int i=0; i< MAX_STATIONS; i++) {
    if (stations[i].getConnStatus() == CONNECTED && stations[i].getCueStatus() == NONE) {
      channelLEDs.setColor(i, BLUE, 0.05);
    }
    else {
      channelLEDs.setColor(i, stations[i].getColor());
    }
  }
}

void rainbow() {
  for (int i=0; i<MAX_STATIONS + 1; i++) {
    channelLEDs.stopFlash(i);
    float indexOffset = 2*PI*((float)(i)/(float)(MAX_STATIONS+1));
    channelLEDs.setColor(i, floor(127.5 + 127.5*sin((float)(millis())/300.0 + indexOffset)),
                            floor(127.5 + 127.5*sin((float)(millis())/600.0 + indexOffset + 2*PI/3)) / 2,
                            floor(127.5 + 127.5*sin((float)(millis())/900.0 + indexOffset + 4*PI/3)) / 2);
  }
}

void rainbow(int index) {
  channelLEDs.stopFlash(index);
  channelLEDs.setColor(index, floor(127.5 + 127.5*sin((float)(millis())/1000.0)),
                          floor(127.5 + 127.5*sin((float)(millis())/1000.0 + 2*PI/3)) / 2,
                          floor(127.5 + 127.5*sin((float)(millis())/1000.0 + 4*PI/3)) / 2);
}

ISR(TIMER2_OVF_vect) {
  channelLEDs.tick();
}

unsigned long birthTime;

void setup() {
  CLCSerial.init(true, 2);
  Serial2.begin(CLC_DEFAULT_BAUD_RATE);
  //CLCSerial.begin(CLC_DEFAULT_BAUD_RATE);
  Serial.begin(CLC_DEFAULT_BAUD_RATE);

  for (int i=0; i<3; i++) {
    pinMode(statusLEDPins[i], OUTPUT);
  }
  updateGlobalStatusLED();

  //DMXSerial.init(DMXReciever, 2);

  // add settings
  addSettings();

  // LED setup
  channelLEDs.setup();

  // timer interrupt for LED code
  // (on an interrupt to prioritize our LEDs looking smooth)
  TIMSK2 = (TIMSK2 & B11111110) | 0x01;
  TCCR2B = (TCCR2B & B11111000) | 0x03;

  //Button setup
  pinMode(btnRedRowEn, OUTPUT);
  pinMode(btnGrnRowEn, OUTPUT);
  
  // screen setup
  screen.init();
  
  screen.showSplash();

  navigation = new NavigationController(&screen, &mainMenu);
  interface = new UIController(&encoder, &encoderButton, navigation);

  birthTime = millis();
}

bool initialSet = false;

void loop() {
  if (!initialSet && millis() > birthTime + 1000) {
    GlobalStatus = OK;
    initialSet = true;
  }
  updateFacePanelLights();
  interface->tick();
  CLCSerial.tick();
  updateGlobalStatusLED();
  updateRemote();
}

enum MessageStatus {
  STATION_REQUEST,
  CONTROL_LOOP,
  BREAK,
};

void updateRemote() {
  static uint8_t responseStation = 0;
  static uint8_t colorStation = 0;
  static unsigned long lastRequest = 0;
  static unsigned long lastSend = 0;
  static MessageStatus status = CONTROL_LOOP;
  static MessageStatus lastStatus = CONTROL_LOOP;
  switch(status) {
    case CONTROL_LOOP:
      if (millis() > lastSend + CLC_COLOR_SEND_DELAY) {
        sendColor(&stations[colorStation]);
        //Serial.println("sent color");
        colorStation++;
        lastSend = millis();
      }
      if (colorStation >= MAX_STATIONS) {
        status = BREAK;
      }
      lastStatus = CONTROL_LOOP;
      break;

    case BREAK:
      sendBreak();
      status = STATION_REQUEST;
      lastStatus = BREAK;
      break;

    case STATION_REQUEST:
      if (lastStatus != STATION_REQUEST) {
        lastRequest = millis();
        sendStationRequest(stations[responseStation].getAddress());
      }
      else {
        bool doneRequesting = false;
        //Serial.println("awaiting resp");
        if (getStationResponse(stations[responseStation].getAddress())) {
          stations[responseStation].setConnStatus(CONNECTED);
          doneRequesting = true;
        }
        else if (millis() > lastRequest + CLC_RESPONSE_DELAY + CLC_STATE_SWITCH_DELAY*2) {
          // station request timed out
          stations[responseStation].setConnStatus(DISCONNECTED);
          doneRequesting = true;
        }

        if (doneRequesting) {
          status = CONTROL_LOOP;
          colorStation = 0;
          responseStation = (responseStation + 1) % MAX_STATIONS;
        }
      }
      lastStatus = STATION_REQUEST;
      break;
  }
}

void sendColor(RemoteStation* station) {
  const int len = 7;
  char buff[len];
  RGBColor color = station->getColor();
  buff[0] = CLC_PKT_START;
  buff[1] = (uint8_t)(station->getAddress());
  buff[2] = (char)(CONTROL);
  buff[3] = color.r;
  buff[4] = color.g;
  buff[5] = color.b;
  buff[6] = CLC_PKT_END;
  CLCSerial.write(buff, len);
}

void sendStationRequest(uint8_t station) {
  const int len = 4;
  char buff[len];
  buff[0] = CLC_PKT_START;
  buff[1] = station;
  buff[2] = (char)(STATUS);
  buff[3] = CLC_PKT_END;
  CLCSerial.write(buff, len);
}

void sendBreak() {
  const int len = CLC_DATA_LEN*4 + 4; // overhead is 4
  char buff[len];
  for (int i=0; i<len; i++) {
    buff[i] = CLC_PKT_END;
  }
  CLCSerial.write(buff, len);
}

bool getStationResponse(uint8_t station) {
  static char lastByte = 0;
  char thisByte = 0;
  /*Serial.print((uint8_t)(lastByte));
  Serial.print(", ");
  Serial.print((uint8_t)(thisByte) == RESPONSE);
  Serial.println();*/
  if (CLCSerial.read(thisByte)) {
    //Serial.println("got resp");
    if ((lastByte == station) && ((uint8_t)(thisByte) == RESPONSE)) {
      lastByte = thisByte;
      return true;
    }
    lastByte = thisByte;
  }
  return false;
}

void updateGlobalStatusLED() {
  const int brightness = 2;
  switch(GlobalStatus) {
    case BUSY:
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[0], 0);
      clc::flashLED(statusLEDPins[2], 50, brightness);
      break;
    case OK:
      analogWrite(statusLEDPins[0], brightness);
      analogWrite(statusLEDPins[1], brightness/1.5);
      analogWrite(statusLEDPins[2], brightness/1.6);
      break;
    case FAULT:
      analogWrite(statusLEDPins[0], 60);
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[2], 0);
      break;
  }
}