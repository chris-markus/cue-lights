// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef CLC_COMMON_H
#define CLC_COMMON_H

#include <stdint.h>
#include "Arduino.h"

// some global constants
#define CLC_DEFAULT_BAUD_RATE 115200

// serial comms packets
#define CLC_PKT_START 0xAD
#define CLC_PKT_END 0xAB
#define CLC_DATA_LEN 3

// masks
#define CLC_MASK_ADDRESS 0b11111000
#define CLC_MASK_TYPE 0b00000111

// packet types
#define CLC_PKTTYPE_CONTROL 0b001
#define CLC_PKTTYPE_STATUS 0b010
#define CLC_PKTTYPE_ACK 0b011

// button constants
#define BUTTON_DEBOUNCE_TIME 20 // ms

enum CLCButtonType {
    ACTIVE_HIGH,
    ACTIVE_LOW,
    ACTIVE_LOW_PULLUP,
};

class CLCDebouncedButton {
public:
    CLCDebouncedButton(unsigned int pin_in, CLCButtonType type_in = ACTIVE_HIGH, int debounceTime_in = BUTTON_DEBOUNCE_TIME);
    bool isPressed();
    bool wasReleased();
private:
    int debounceTime;
    bool state = false;
    bool lastAcceptedState = false;
    bool awaitingRelease = true;
    unsigned long lastStateUpdate = 0;
    unsigned int pin;
    CLCButtonType type;
};

// packet definition
// [Start byte][Address bits (5) | packet type bits (3)][Data][Data][Data][Stop byte]

/*
class CLCSerial {
    public:
        CLCSerial();
        int begin(uint8_t address, int baud_rate);
        void setAddress(uint8_t address);
        CLCPacketType asyncRecieve(char* msg);
    private:
        uint8_t int_address;
        char* pktBuffer;
        uint8_t msgLen;
        bool int_isMessageReady();
        bool int_sendMessage(char* msg, CLCPacketType type);
        bool int_isValidPacket(char* pkt);
        bool int_isAddressedToMe();
        bool int_getPktData(char* pkt, char* data);
};

enum CLCPacketType{
    NONE,
    CONTROL,
    STATUS,
    ACK,
}*/

namespace clc {
    void flashLED(int pin, unsigned int delay, uint8_t brightness);
};

#endif