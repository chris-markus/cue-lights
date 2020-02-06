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

// include our source files
#include <CueLightsCommon.h>
#include "strings.h"
#include "lcd_screen.h"
#include "ui_controller.h"
#include "menu.h"

// define constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// some global objects
CLCDebouncedButton encoderButton(53, ACTIVE_LOW_PULLUP);
LCDScreen screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Encoder encoder(18,19);

// references to our UI controllers
NavigationController* navigation;
UIController* interface;

// Menu definition and helper methods
void goHome() {
  navigation->goHome();
}

// back buttons are automatically added
Menu mainMenu( "", 7,
  new HeaderItem("Main Menu"),
  new MenuItem("X Close", false, goHome),
  new MenuItem("Item 2"),
  new Menu("Submenu 1", 2,
    new MenuItem("Home", goHome),
    new Menu("Submenu 2", 2,
      new MenuItem("Item 1"),
      new MenuItem("Item 2")
    )
  ),
  new MenuItem("Loooooooooong item"),
  new MenuItem("Item 5"),
  new MenuItem("Item 6")
);

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Serial.begin(115200);
  screen.init();
  
  screen.showSplash();

  navigation = new NavigationController(&screen, &mainMenu);
  interface = new UIController(&encoder, &encoderButton, navigation);
}

void loop() {  
  interface->tick();
}
