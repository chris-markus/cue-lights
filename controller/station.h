// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef REMOTE_STATION_H
#define REMOTE_STATION_H

#include <stdint.h>

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
    void setCueStatus(CueStatus status_in) { cueStatus = status_in; };
    CueStatus getCueStatus() { return cueStatus; };
private:
    uint8_t address;
    ConnectionStatus connectionStatus = DISCONNECTED;
    CueStatus cueStatus = NONE;
};

#endif