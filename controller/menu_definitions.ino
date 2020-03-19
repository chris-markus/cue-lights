// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "menu.h"

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