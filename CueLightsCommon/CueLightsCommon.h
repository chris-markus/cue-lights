// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef CLC_COMMON_H
#define CLC_COMMON_H

#include <stdint.h>
#include "Arduino.h"

// Max number of connected stations
#define MAX_STATIONS 10
#define CHANNELS_PER_STATION 3

// some global constants
#define CLC_DEFAULT_BAUD_RATE 115200

// packet definitions... very very loosely based on DMX/RDM packet principles
/* 
 * 3 packet types:
 * 
 * CONTROL: Sent by the control station, contains the color data for each cue light
 * STATUS: Sent by the control station, contains the address of a cue light that
 *  should respond to this message
 * RESPONSE: Sent by a cue light station, contains the address of the station it 
 *  was sent from and whether or not that station is synchronized
 * 
 * Each packet begins with a start character and then the packet type. Then the
 * packet data is sent, followed by an end character.
 * For instance, the RESPONSE packet looks like this:
 * |START|TYPE|ADDRESS (as char)|SYNCHRONIZED (as char)|END|
 * | '<' |'R' |      0x03       |         0x01         |'>'|
 * 
 * 
*/
#define CLC_PKT_START '<'
#define CLC_PKT_END '>'
#define CLC_DATA_LEN (CHANNELS_PER_STATION * MAX_STATIONS)
#define CLC_STATUS_LEN 2 // 
#define CLC_OVERHEAD 3 // start + type + end
#define CLC_BREAK_LEN CLC_DATA_LEN

typedef uint8_t CLCPacketType;

#define PKT_TYPE_CONTROL 'C'
#define PKT_TYPE_STATUS 'S'
#define PKT_TYPE_RESPONSE 'R'

// delays and timeouts
#define CLC_RESPONSE_TIMEOUT 50 // ms
#define CLC_RESPONSE_DELAY 10 // ms
#define CLC_STATE_SWITCH_DELAY 1 // ms
#define CLC_COLOR_SEND_DELAY 2 // ms + 1

#define STATION_DISCONNECT_TIMEOUT 2000 // ms

// General Structs and enums
struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// message recieve progress state (only used in reciever code for now...)
enum MessageProgressState {
  STATE_START,
  STATE_TYPE,
  STATE_DATA,
  STATE_STATUS,
  STATE_END,
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

#endif