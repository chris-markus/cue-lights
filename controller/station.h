// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef STATION_H
#define STATION_H

#include <CueLightsCommon.h>

enum ConnectionStatus {
    CONNECTED,
    DISCONNECTED,
};

enum CueStatus {
    NONE,
    STBY,
    GO
};

struct Station {
    RGBColor color = {0,0,0};
    uint8_t brightness = 255;
    uint8_t address = 0;
    CueStatus cueStatus = NONE;
    unsigned long lastSeen = 0;
    ConnectionStatus connStatus = DISCONNECTED;
};

Station stations[MAX_STATIONS];

RGBColor panelIndicators[MAX_STATIONS + 1];

static void InitStations() {
    for (uint8_t i=0; i<MAX_STATIONS; i++) {
        stations[i].address = i+1;
        panelIndicators[i] = {0,0,0};
    }
    panelIndicators[MAX_STATIONS] = {0,0,0};
}

#endif
