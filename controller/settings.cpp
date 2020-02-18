// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "settings.h"
#include "string.h"
#include "Arduino.h"

Settings* Settings::instance = NULL; 

Settings* Settings::alloc(int numSettings = 0) {
    if (instance) {
        delete[] instance->settings;
        delete instance;
    }
    instance = new Settings;
    instance->maxSettings = numSettings;
    instance->settings = new Setting*[numSettings];
    return instance;
}

Settings* Settings::getInstance()  {
    if (!instance)
        alloc(DEFAULT_NUM_SETTINGS);
    return instance;
}

Setting* Settings::getSettingWithName(const char* name) {
    for (int i=0; i<count; i++) {
        if (strcmp(settings[i]->name, name) == 0) {
            return settings[i];
        }
    }
    return NULL;
}

bool Settings::add(Setting* s) {
    if (count < maxSettings) {
        settings[count++] = s;
        return true;
    }
    return false;
}

void Settings::save(Setting* s) {
    // TODO: Save to EEPROM
}