// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef CLC_COMMON_H
#define CLC_COMMON_H

#include <stdint.h>
#include "Arduino.h"

#define CHANNELS_PER_STATION 3

// some global constants
#define CLC_DEFAULT_BAUD_RATE 115200
#define SEND_BUFFER_LEN 200

// packet definitions... very loosely based on DMX packet principles
// inspired by http://github.com/mathertel/DmxSerial
#define CLC_PKT_START 's'
#define CLC_PKT_END 'e'
#define CLC_DATA_LEN 3

typedef uint8_t CLCPacketType;

#define CONTROL 0x11
#define STATUS 0xCC
#define RESPONSE 0xEE

// delays and timeouts
#define CLC_RESPONSE_DELAY 50 // ms
#define CLC_STATE_SWITCH_DELAY 1 // ms
#define CLC_COLOR_SEND_DELAY 2 // ms + 1

enum SerialState {
    WILL_RECIEVE,
    RECIEVING,
    WILL_SEND,
    SENDING
};

class CLCSerialClass {
public:
    CLCSerialClass(HardwareSerial *ser) { serial = ser; };
    void init(bool is_master, int mode_pin_in);
    void setAddress(uint8_t address) { int_address = address; };
    void write(const char* buffer, int len);
    bool read(char& byte);
    void tick();
private:
    bool isInitialized = false;
    int mode_pin;
    unsigned long lastStateChange = 0;
    char sendBuffer[SEND_BUFFER_LEN];
    int bufferLen = 0;
    bool master = true;
    uint8_t int_address;
    uint8_t msgLen;
    SerialState state;
    HardwareSerial *serial;
};

// General Structs and enums
struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// buttons
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
    bool state;
    bool lastAcceptedState;
    bool awaitingRelease = true;
    unsigned long lastStateUpdate = 0;
    unsigned int pin;
    CLCButtonType type;
};

namespace clc {
    void flashLED(int pin, unsigned int delay, uint8_t brightness);
};

#endif