// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "settings.h"
#include "string.h"
#include "Arduino.h"

Settings* Settings::instance = NULL; 

Settings* Settings::getInstance()  {
    if (!instance)
    instance = new Settings;
    return instance;
}

Setting* Settings::getSettingWithName(const char* name) {
    Setting* iter = firstSetting;
    while (iter != NULL && strcmp(iter->name, name) != 0) {
        iter = iter->next;
    }
    return iter;
}

void Settings::add(Setting* s) {
    if (count > 0) {
        getLast()->next = s;
    }
    else {
        firstSetting = s;
    }
    count++;
}

void Settings::save(Setting* s) {
    // TODO: Save to EEPROM
}

Setting* Settings::getLast() {
    Setting* iter = firstSetting;
    while (iter->next != NULL) {
        iter = iter->next;
    }
    return iter;
}