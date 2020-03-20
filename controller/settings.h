// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#include <CueLightsCommon.h>

#define NUM_SETTINGS 18

// version of settings (change this whenever changes are made to SettingsEEPROMData struct)
#define SETTINGS_ITERATION 1

#define EEPROM_CRC_ADDRESS EEPROM.length() - sizeof(unsigned long)
// offset the start of our data by the size of the crc (which is stored at 0)
#define EEPROM_SETTINGS_ADDRESS 0


#define SETTING_PANEL_BRIGHTNESS "Panel Brightness"
//#define SETTING_PANEL_BRIGHTNESS_ID 0
#define SETTING_FLASH_STANDBY "Flash on Standby"
//#define SETTING_FLASH_STANDBY_ID 1
#define SETTING_STATION_BRIGHTNESS "Station %d Brightness"
//#define SETTING_STATION_BRIGHTNESS_ID 2 // there are 10 of them

//#define SETTING_STBY_COLOR_ID 12 // 3 of them
//#define SETTING_GO_COLOR_ID 15 // 3 of them

enum SettingType {
    BOOL,
    VALUE
};

struct Setting {
    const char* name;
    uint8_t value;
    uint8_t max;
    uint8_t min;
    SettingType type;
};

extern Setting panelBrightness;
extern Setting stationBrightness[MAX_STATIONS];
extern Setting standbyColor[3];
extern Setting goColor[3];
extern Setting flashOnStandby;


// struct for storing settings values to eeprom; ALL SIZES ARE HARDCODED to avoid any unfortunate mix-ups...
#if MAX_STATIONS != 10
    #error "You changed the number of stations, please update the eeprom struct to match (in settings.h)"
#endif


struct SettingsEEPROMData {                     // Offsets from start address: (eeprom is byte-addressable)
    int iteration = SETTINGS_ITERATION;         // 0
    uint8_t panelBrightness;                    // 2
    uint8_t stationBrightness[10];              // 3
    uint8_t standbyColor[3];                    // 13
    uint8_t goColor[3];                         // 16
    uint8_t flashOnStandby;                     // 19
};

// forward declarations:
void saveSettings();

#endif