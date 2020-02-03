// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

// include libraries
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

// include our source files
#include <CueLightsCommon.h>
#include "strings.h"
#include "ui_layer.h"
#include "menu.h"
#include <Wire.h>

// define constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// some global objects
UIInterface screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Encoder encoder(18,19);

NavigationController *navigation;

void doNothing();
void launchSubMenu();

void goHome() {
  navigation->goHome();
}

// back buttons are automatically added
Menu mainMenu( "Main Menu", 7,
  new HeaderItem("Main Menu"),
  new MenuItem("<  Home", false, goHome),
  new MenuItem("Item 2"),
  new Menu("Submenu 1", 2,
    new MenuItem("Home", goHome),
    new Menu("Submenu 2", 2,
      new MenuItem("Item 1"),
      new MenuItem("Item 2")
    )
  ),
  new MenuItem("Item 4"),
  new MenuItem("Item 5"),
  new MenuItem("Item 6"),
  new MenuItem("Item 7")
);

void doNothing(){
  // well, do nothing
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(53, INPUT_PULLUP);
  digitalWrite(2, LOW);
  Serial.begin(115200);
  screen.init();
  
  screen.showSplash();

  navigation = new NavigationController(&screen, &mainMenu);
}

void loop() {  
  static long offset = 0;
  static bool didPress = false;
  long reading = encoder.read();
  if (reading > offset + 3) {
    navigation->selectNext();
    offset = reading;
  }
  else if (reading < offset - 3) {
    navigation->selectPrev();
    offset = reading;
  }
  if (!digitalRead(53)) {
    if (!didPress) {
      navigation->dispatchPress();
      didPress = true;
      delay(100);
    }
  }
  else if (didPress) {
    didPress = false;
    delay(100);
  }
}
