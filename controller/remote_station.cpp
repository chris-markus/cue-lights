// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "remote_station.h"
#include "settings.h"

// =====================================================
// RemoteStation
// =====================================================

// init static members:
unsigned long RemoteStation::lastFlashUpdate = 0;
bool RemoteStation::flashState = true;

void RemoteStation::setCueStatus(CueStatus status_in) {
    cueStatus = status_in;
    RGBColor colorTmp = {
        r: 0,
        g: 0,
        b: 0,
    };
    if (cueStatus == STBY) {
        colorTmp.r = (Settings::getInstance()->getSettingWithID(SETTING_STBY_COLOR_ID))->value;
        colorTmp.g = (Settings::getInstance()->getSettingWithID(SETTING_STBY_COLOR_ID+1))->value;
        colorTmp.b = (Settings::getInstance()->getSettingWithID(SETTING_STBY_COLOR_ID+2))->value;
        isFlashing = true;
    }
    else if (cueStatus == GO) {
        colorTmp.r = Settings::getInstance()->getSettingWithID(SETTING_GO_COLOR_ID)->value;
        colorTmp.g = Settings::getInstance()->getSettingWithID(SETTING_GO_COLOR_ID+1)->value;
        colorTmp.b = Settings::getInstance()->getSettingWithID(SETTING_GO_COLOR_ID+2)->value;
        isFlashing = false;
    }
    else {
        isFlashing = false;
    }
    color = colorTmp;
}

RGBColor RemoteStation::getColor(bool ignoreBrightness = false) {
    RGBColor retColor = {
        r: 0,
        g: 0,
        b: 0,
    };
    bool onIfFlashing = getFlashState();
    if (connectionStatus == CONNECTED && (!isFlashing || onIfFlashing)) {
        Setting* brightnessSetting = Settings::getInstance()->getSettingWithID(SETTING_STATION_BRIGHTNESS_ID + address - 1);
        retColor = {
            r: color.r*(ignoreBrightness?1:brightnessSetting->value/brightnessSetting->max),
            g: color.g*(ignoreBrightness?1:brightnessSetting->value/brightnessSetting->max),
            b: color.b*(ignoreBrightness?1:brightnessSetting->value/brightnessSetting->max),
        };
    }
    return retColor;
};

// private -----------------------------

bool RemoteStation::getFlashState() {
    bool flashEnabled = (bool)(Settings::getInstance()->getSettingWithID(SETTING_FLASH_STANDBY_ID)->value);
    if (flashEnabled && flashState && millis() > lastFlashUpdate + FLASH_DELAY_ON || 
        !flashState && millis() > lastFlashUpdate + FLASH_DELAY_OFF) {
        flashState = !flashState;
        lastFlashUpdate = millis();
    }
    return !flashEnabled || flashEnabled && flashState;
}

// =====================================================
// RemoteInterface
// =====================================================
/*
RemoteInterface* RemoteInterface::instance = NULL; 

RemoteInterface* RemoteInterface::getInstance() {
    if (!instance)
        instance = new RemoteInterface();
        instance->init();
    return instance;
}

void RemoteInterface::init() {
    for (int i=0; i<MAX_STATIONS; i++) {
        stations[i] = new RemoteStation(i);
    }
}

int RemoteInterface::getNumStationsConnected() {
    return 0;
}

void RemoteInterface::scanAndConnect() {

}

void RemoteInterface::tick() {

}*/

// helper functions:
