// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "remote_station.h"

// =====================================================
// RemoteStation
// =====================================================
void RemoteStation::setCueStatus(CueStatus status_in) {
    cueStatus = status_in;
    RGBColor color = {
        r: 0,
        g: 0,
        b: 0,
    };
    if (cueStatus == STBY) {
        color.r =  255;
    }
    else if (cueStatus == GO) {
        color.g = 255;
    }
    setColor(color);
}

// =====================================================
// RemoteInterface
// =====================================================
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

}

// helper functions:
