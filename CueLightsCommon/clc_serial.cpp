// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "CueLightsCommon.h"

void CLCSerialClass::init(bool is_master, int mode_pin_in) {
    master = is_master;
    mode_pin = mode_pin_in;
    pinMode(mode_pin, OUTPUT);
    digitalWrite(mode_pin, LOW);
    if (master)
        state = WILL_SEND;
    else 
        state = WILL_RECIEVE;
    isInitialized = true;
}

void CLCSerialClass::tick() {
    if (!isInitialized) {
        return;
    }
    switch (state) {
        case WILL_RECIEVE:
            if (millis() > lastStateChange + CLC_STATE_SWITCH_DELAY) {
                state = RECIEVING;
                lastStateChange = millis();
            }
            break;
        case WILL_SEND:
            if (millis() > lastStateChange + CLC_STATE_SWITCH_DELAY) {
                for (int i=0; i < bufferLen; i++) {
                    serial->write(sendBuffer[i]);
                }
                bufferLen = 0;
                state = SENDING;
                lastStateChange = millis();
            }
            break;
        case RECIEVING:
        case SENDING:
            break;
    }
}

void CLCSerialClass::write(const char* buffer, int len) {
    static bool writing = true;
    if (state == SENDING) {
        writing = true;
        for(int i=0 ;i < bufferLen; i++) {
            serial->write(sendBuffer[i]);
        }
        bufferLen = 0;
        for(int i = 0; i < len; i++) {
            serial->write(buffer[i]);
        }
    }
    else {
        state = WILL_SEND;
        digitalWrite(mode_pin, HIGH);
        lastStateChange = millis();

        for(int i=0; i<len && bufferLen < SEND_BUFFER_LEN; bufferLen++, i++) {
            sendBuffer[bufferLen] = buffer[i];
        }
    }
}

bool CLCSerialClass::read(char& byte) {
    if (state == RECIEVING) {
        if (serial->available()) {
            byte = serial->read();
            return true;
        }
    }
    else {
        state = WILL_RECIEVE;
        digitalWrite(mode_pin, LOW);
        lastStateChange = millis();
    }
    return false;
}