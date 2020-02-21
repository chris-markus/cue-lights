// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>

#define DEFAULT_NUM_SETTINGS 10

#define SETTING_PANEL_BRIGHTNESS "Panel Brightness"
#define SETTING_PANEL_BRIGHTNESS_ID 0
#define SETTING_FLASH_STANDBY "Flash on Standby"
#define SETTING_FLASH_STANDBY_ID 1
#define SETTING_STATION_BRIGHTNESS "Station %d Brightness"
#define SETTING_STATION_BRIGHTNESS_ID 2 // there are 10 of them

#define SETTING_STBY_COLOR_ID 12 // 3 of them
#define SETTING_GO_COLOR_ID 15 // 3 of them

enum SettingType {
    BOOL,
    VALUE
};

struct Setting {
    const char* name;
    uint8_t id;
    uint8_t value;
    uint8_t max;
    uint8_t min;
    SettingType type;
};

class Settings {
private:
    static Settings *instance;
    Setting** settings;
    // Private constructor so that no objects can be created.
    Settings() { /*do nothing*/ }

    // we don't want to delete the setting objects because they may be used elsewhere
    ~Settings() { delete settings; delete instance; }

    int count = 0;
    int maxSettings = DEFAULT_NUM_SETTINGS;
    uint8_t* defaults;
    EEPROMClass *eeprom;
public:
    static Settings *alloc(int num_settings, EEPROMClass* rom);
    static Settings *getInstance();
    bool add(Setting* s);
    void save(Setting* s);
    void resetDefaults();
    //Setting* getSettingWithName(const char* name);
    Setting* getSettingWithID(uint8_t id);
};


#endif