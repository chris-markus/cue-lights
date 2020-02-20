// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef REMOTE_STATION_H
#define REMOTE_STATION_H

#include <stdint.h>
#include "Arduino.h"
#include <CueLightsCommon.h>
#include "constants.h"

#define STATION_DISCONNECT_TIMEOUT 2000 //ms

enum ConnectionStatus {
    CONNECTED,
    DISCONNECTED,
};

enum CueStatus {
    NONE,
    STBY,
    GO
};

class RemoteStation {
public:
    RemoteStation(uint8_t address_in) { address = address_in; };
    uint8_t getAddress() { return address; };
    void setConnStatus(ConnectionStatus status_in) { connectionStatus = status_in; };
    ConnectionStatus getConnStatus() { return connectionStatus; };
    void setLastSeen() { lastSeen = millis(); };
    unsigned long getLastSeen() { return lastSeen; };
    void setCueStatus(CueStatus status_in);
    CueStatus getCueStatus() { return cueStatus; };
    void setColor(RGBColor color_in) { color = color_in; };
    RGBColor getColor() { return color; };
private:
    unsigned long lastSeen = 0;
    uint8_t address;
    RGBColor color;
    ConnectionStatus connectionStatus = DISCONNECTED;
    CueStatus cueStatus = NONE;
};

class RemoteInterface {
private:
    static RemoteInterface *instance;
    RemoteStation* stations[MAX_STATIONS];
    // Private constructor so that no objects can be created.
    RemoteInterface() { /*do nothing*/ }
    ~RemoteInterface() { delete instance; delete[] stations; }

    void init();
public:
    static RemoteInterface* getInstance();
    int getNumStationsConnected();
    void scanAndConnect();
    RemoteStation* getStation(int num) { return stations[num]; };
    void tick();
};

#endif