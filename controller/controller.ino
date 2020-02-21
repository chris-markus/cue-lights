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
// CueLightsCommon: In git repo (follow instructions in README)

// include libraries
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Wire.h>
//#include <DMXSerial.h>
#include <EEPROM.h>

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

// TODO:
// Add back flashing - DONE
// implement brightness and color settings
// fix Testing
// debug panel led flashing - WON'T FIX
// slim down ISR code
// fix settings code
// implement issue Center
// general code clean-up

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
int stationMissCounter[MAX_STATIONS] = {0,0,0,0,0,0,0,0,0,0};
int stationHitCounter[MAX_STATIONS] = {0,0,0,0,0,0,0,0,0,0};

const int ledPins[MAX_STATIONS + 1] = {42,40,38,36,34,32,30,28,26,24,22};
MultiLED channelLEDs(11, 10, 9, MAX_STATIONS + 1, ledPins); // rPin, gPin, bPin, numLED, pins

// references to our UI controllers
NavigationController* navigation;
UIController* interface;

// array of remote station objects
RemoteStation stations[MAX_STATIONS] = {1,2,3,4,5,6,7,8,9,10};

// settings
Setting panelBrightness = {SETTING_PANEL_BRIGHTNESS, SETTING_PANEL_BRIGHTNESS_ID, 100, 100, 10, VALUE};
Setting stationBrightness[MAX_STATIONS];
Setting standbyColor[3];
Setting goColor[3];
Setting flashOnStandby = {SETTING_FLASH_STANDBY, SETTING_FLASH_STANDBY_ID, 1, 1, 0, BOOL};

void addSettings() {
  // allocate the settings object and tell it we want 12 settings
  Settings::alloc(18, &EEPROM);

  for (int i=0; i<MAX_STATIONS; i++) {
    char* nameBuf = new char[21];
    sprintf(nameBuf, SETTING_STATION_BRIGHTNESS, (i+1));
    stationBrightness[i] = Setting {nameBuf, SETTING_STATION_BRIGHTNESS_ID + i, 100, 100, 10, VALUE};
    Settings::getInstance()->add((Setting*)(stationBrightness + i));
  }

  standbyColor[0] = Setting {STR_RED, SETTING_STBY_COLOR_ID, 255, 255, 0, VALUE};
  standbyColor[1] = Setting {STR_GREEN, SETTING_STBY_COLOR_ID+1, 0, 255, 0, VALUE};
  standbyColor[2] = Setting {STR_BLUE, SETTING_STBY_COLOR_ID+2, 0, 255, 0, VALUE};

  goColor[0] = Setting {STR_RED, SETTING_GO_COLOR_ID, 0, 255, 0, VALUE};
  goColor[1] = Setting {STR_GREEN, SETTING_GO_COLOR_ID+1, 255, 255, 0, VALUE};
  goColor[2] = Setting {STR_BLUE, SETTING_GO_COLOR_ID+2, 0, 255, 0, VALUE};

  for (int i=0; i<3; i++) {
    Settings::getInstance()->add(&(standbyColor[i]));
    Settings::getInstance()->add(&(goColor[i]));
  }

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
/*
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
  int16_t yOff = screen.printCentered(STR_TESTING_STATION);
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
}*/

void getConnStatusString(const char* input) {
  uint8_t count = 0;
  for(int i=0; i<MAX_STATIONS; i++) {
    count += stations[i].getConnStatus() == CONNECTED;
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
    if (stations[currentStation].getConnStatus() != CONNECTED 
        && stations[i].getLastSeen() != 0
        && millis() - stations[i].getLastSeen() > STATION_DISCONNECT_TIMEOUT 
        && millis() - stations[i].getLastSeen() < 100000)
    {
      if (millis() - stations[i].getLastSeen() < 10000) {
        fault = true;
      }
      connected[i] = false;
      numDisconnected++;
    }
    else {
      connected[i] = true;
    }
  }
  if (stations[currentStation].getConnStatus() == CONNECTED || millis() > lastUpdate + updateInterval) {
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

void getGlobalStatusString(const char* input) {
  int numIssues = 0;
  int i = getNextDisconnectedStation(numIssues);
  if (i != -1) {
    sprintf(input, STR_ISSUES, numIssues);
  }
  else {
    sprintf(input, STR_NO_ISSUES);
  }
}

void getDescriptiveGlobalStatusString(const char* input) {
  int numIssues = 0;
  int i = getNextDisconnectedStation(numIssues);
  if (i != -1) {
    sprintf(input, STR_STATION_DISCONNECTED, i+1, (millis() - stations[i].getLastSeen())/1000);
  }
  else {
    sprintf(input, STR_NO_ISSUES);
  }
}

void resetSettings() {
  Settings::getInstance()->resetDefaults();
  navigation->goHome();
}

// back buttons are automatically added
Menu mainMenu(STR_MENU_MAIN, 6,
  new HeaderItem(STR_MENU_MAIN, STR_CLOSE_BUTTON, NULL, true),
  new FullScreenDisplay(SETTING_PANEL_BRIGHTNESS, 0,1,
    new SettingChanger(&panelBrightness, &encoder)
  ),
  new Menu(STR_STATION_LIGHTS, 4,
    makeBrightnessMenu(),
    new FullScreenDisplay(SETTING_FLASH_STANDBY, 0,1,
      new SettingChanger(&flashOnStandby, &encoder)
    ),
    makeColorMenu(STR_STBY_COLOR, standbyColor),
    makeColorMenu(STR_GO_COLOR, goColor)
  ),
  new MenuItem(STR_RESET, resetSettings),
  /*new Menu(STR_TEST_STATIONS, 2,
    new MenuItem(STR_TEST_ALL_STATIONS, testAll),
    new MenuItem(STR_TEST_ONE_STATION, testOne)
  ),*/
  new Divider(),
  new FullScreenDisplay(STR_ABOUT, 0, 2,
    new FullScreenElement(STR_BACK_BUTTON, {x: LEFT, y: TOP}, true, NULL, new BackButton()),
    new FullScreenElement(STR_ABOUT_DESCRIPTION,{x: LEFT, y: BOTTOM}, false)
  )
);

FullScreenDisplay homeScreen("", /*default selection*/ 1, /*count=*/3,
  new FullScreenElement("", {x: LEFT, y: BOTTOM}, true, getGlobalStatusString,
    new FullScreenDisplay("", 0, 2,
      new FullScreenElement(STR_BACK_BUTTON, {x: LEFT, y: TOP}, true, NULL, new BackButton()),
      new FullScreenElement("", {x: CENTER, y: MIDDLE}, false, getDescriptiveGlobalStatusString)
    )
  ),
  new FullScreenElement(STR_MENU_MAIN, {x: RIGHT, y: BOTTOM}, true, NULL, &mainMenu),
  new FullScreenElement("", {x: CENTER, y: MIDDLE}, false, getConnStatusString)
);

Menu* makeBrightnessMenu() {
  MenuItemBase* stationItems[MAX_STATIONS];
  for (int i = 0; i<MAX_STATIONS; i++) {
    char* nameTemp = new char[11];
    sprintf(nameTemp, STR_STATION_I, (i+1));
    stationItems[i] = new FullScreenDisplay(nameTemp, 0,1,
      new SettingChanger((Setting*)(stationBrightness + i), &encoder)
    );
  }
  return new Menu(STR_BRIGHTNESS, MAX_STATIONS, stationItems);
}

Menu* makeColorMenu(const char* name, Setting* setting) {
  const char* colorNames[3] = {STR_RED, STR_GREEN, STR_BLUE};
  MenuItemBase* colorItems[3];
  for (int i = 0; i<3; i++) {
    colorItems[i] = new FullScreenDisplay(colorNames[i], 0,1,
      new SettingChanger(&(setting[i]), &encoder)
    );
  }
  return new Menu(name, 3, colorItems);
}

void updateFacePanelLights() {
  // row 0 is standby, 1 is go
  static int buttonRow = false;
  static bool canSwap[MAX_STATIONS];
  static bool isInit = false;
  if (!isInit) {
    for(int i=0; i<MAX_STATIONS; i++) {
      canSwap[i] = true;
    }
    isInit = true;
  }
  digitalWrite(btnRedRowEn, buttonRow);
  digitalWrite(btnGrnRowEn, !buttonRow);

  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[i].getConnStatus() == CONNECTED) {
      CueStatus status = stations[i].getCueStatus();
      if (stationButtons[i][buttonRow].isPressed()) {
        if (canSwap[i]) {
          if (status == NONE || buttonRow != (status == STBY)) {
            // if the status is none or if the status is not the same as the 
            // button we are pressing, set the status to correspond to the button
            stations[i].setCueStatus(buttonRow?STBY:GO);
          }
          else {
            // otherwise, set it to NONE
            stations[i].setCueStatus(NONE);
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

  int stby = -1;
  int go = -1;
  int connected = -1;
  for (int i=0; i<MAX_STATIONS; i++) {
    if (stations[i].getCueStatus() == STBY) {
      stby = i;
    }
    else if (stations[i].getCueStatus() == GO) {
      go = i;
    }
    else if (stations[i].getConnStatus() == CONNECTED) {
      connected = i;
    }
  }

  if (stby != -1) {
    channelLEDs.setColor(MAX_STATIONS, channelLEDs.getColor(stby));
  }
  else if (go != -1) {
    channelLEDs.setColor(MAX_STATIONS, channelLEDs.getColor(go));
  }
  else if (connected != -1) {
    channelLEDs.setColor(MAX_STATIONS, channelLEDs.getColor(connected));
  }
  else {
    channelLEDs.setColor(MAX_STATIONS, OFF);
  }

  for(int i=0; i< MAX_STATIONS; i++) {
    if (stations[i].getConnStatus() == CONNECTED && stations[i].getCueStatus() == NONE) {
      channelLEDs.setColor(i, BLUE, 0.1);
    }
    else {
      channelLEDs.setColor(i, stations[i].getColor());
    }
  }
}

/*
// TODO: fix this...
void rainbow() {
  for (int i=0; i<MAX_STATIONS + 1; i++) {
    //channelLEDs.stopFlash(i);
    float indexOffset = 2*PI*((float)(i)/(float)(MAX_STATIONS+1));
    RGBColor c = {r: floor(127.5 + 127.5*sin((float)(millis())/300.0 + indexOffset)),
                  g: floor(127.5 + 127.5*sin((float)(millis())/600.0 + indexOffset + 2*PI/3)) / 2,
                  b: floor(127.5 + 127.5*sin((float)(millis())/900.0 + indexOffset + 4*PI/3)) / 2};
    stations[i].setColor(c);
  }
}

void rainbow(int index) {
  //channelLEDs.stopFlash(index);
  channelLEDs.setColor(index, floor(127.5 + 127.5*sin((float)(millis())/1000.0)),
                          floor(127.5 + 127.5*sin((float)(millis())/1000.0 + 2*PI/3)) / 2,
                          floor(127.5 + 127.5*sin((float)(millis())/1000.0 + 4*PI/3)) / 2);
}*/

// this is almost definitely too much code for an ISR but it runs just fine...
ISR(TIMER2_OVF_vect) {
  channelLEDs.tick();
  CLCSerial.tick();
  updateRemote();
}

unsigned long birthTime;

void setup() {
  CLCSerial.init(true, 2);
  Serial2.begin(CLC_DEFAULT_BAUD_RATE);
  //Serial.begin(CLC_DEFAULT_BAUD_RATE);

  for (int i=0; i<3; i++) {
    pinMode(statusLEDPins[i], OUTPUT);
  }
  updateGlobalStatusLED();

  //DMXSerial.init(DMXReciever, 2);

  // add settings
  addSettings();

  // LED setup
  channelLEDs.setup();

  // timer interrupt for important code
  TIMSK2 = (TIMSK2 & B11111110) | 0x01;
  TCCR2B = (TCCR2B & B11111000) | 0x03;

  //Button setup
  pinMode(btnRedRowEn, OUTPUT);
  pinMode(btnGrnRowEn, OUTPUT);
  
  // screen setup
  screen.init();
  
  screen.showSplash();

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
  updateFacePanelLights();
  interface->tick();
  updateGlobalStatusLED();
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
        colorStation++;
        lastSend = millis();
      }
      if (colorStation >= MAX_STATIONS) {
        status = STATION_REQUEST;
      }
      lastStatus = CONTROL_LOOP;
      break;

    case BREAK:
      sendBreak();
      status = CONTROL_LOOP;
      lastStatus = BREAK;
      break;

    case STATION_REQUEST:
      if (lastStatus != STATION_REQUEST) {
        lastRequest = millis();
        sendStationRequest(stations[responseStation].getAddress());
      }
      else {
        bool doneRequesting = false;
        if (getStationResponse(stations[responseStation].getAddress())) {
          //stationHitCounter[responseStation]++;
          stations[responseStation].setConnStatus(CONNECTED);
          stations[responseStation].setLastSeen();
          doneRequesting = true;
        }
        else if (millis() > lastRequest + CLC_RESPONSE_DELAY + CLC_STATE_SWITCH_DELAY*2) {
          /*if (stations[responseStation].getConnStatus == CONNECTED) {
            stationMissCounter[responseStation]++;
            stationHitCounter[responseStation]++;
          }*/
          // station request timed out, disconnect if it's been long enough
          if (stations[responseStation].getLastSeen() < millis() - STATION_DISCONNECT_TIMEOUT) {
            stations[responseStation].setConnStatus(DISCONNECTED);
          }
          doneRequesting = true;
        }

        if (doneRequesting) {
          status = BREAK;
          colorStation = 0;
          responseStation = (responseStation + 1) % MAX_STATIONS;
        }
      }
      lastStatus = STATION_REQUEST;
      break;
  }
}

/*
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}*/

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
  if (CLCSerial.read(thisByte)) {
    if ((lastByte == station) && ((uint8_t)(thisByte) == RESPONSE)) {
      lastByte = thisByte;
      return true;
    }
    lastByte = thisByte;
  }
  return false;
}

void updateGlobalStatusLED() {
  const int brightness = 3;
  switch(GlobalStatus) {
    case BUSY:
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[0], 0);
      clc::flashLED(statusLEDPins[2], 50, brightness);
      break;
    case OK:
      analogWrite(statusLEDPins[0], brightness*1.5);
      analogWrite(statusLEDPins[1], brightness/2.0);
      analogWrite(statusLEDPins[2], brightness/1.5);
      break;
    case FAULT:
      clc::flashLED(statusLEDPins[0], 100, 30);
      digitalWrite(statusLEDPins[1], 0);
      digitalWrite(statusLEDPins[2], 0);
      break;
  }
}