// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "settings.h"
#include "string.h"
#include "Arduino.h"

Settings* Settings::instance = NULL; 

Settings* Settings::alloc(int numSettings, EEPROMClass *rom) {
    if (instance) {
        delete[] instance->settings;
        delete instance;
    }
    instance = new Settings;
    instance->maxSettings = numSettings;
    instance->settings = new Setting*[numSettings];
    instance->defaults = new uint8_t[numSettings];
    instance->eeprom = rom;
    return instance;
}

Settings* Settings::getInstance()  {
    if (!instance)
        alloc(DEFAULT_NUM_SETTINGS, &EEPROM);
    return instance;
}

Setting* Settings::getSettingWithID(uint8_t id) {
    for (int i=0; i<count; i++) {
        if (settings[i]->id == id) {
            return settings[i];
        }
    }
    return NULL;
}

bool Settings::add(Setting* s) {
    if (count < maxSettings) {
        settings[count++] = s;
        defaults[count - 1] = s->value;
        uint8_t readVal = eeprom->read(s->id);
        if (readVal <= s->max && readVal >= s->min) {
            s->value = readVal;
        }
        return true;
    }
    return false;
}

void Settings::resetDefaults() {
    for (int i=0; i<count; i++) {
        settings[i]->value = defaults[i];
        save(settings[i]);
    }
}

void Settings::save(Setting* s) {
    eeprom->update(s->id, s->value);
}