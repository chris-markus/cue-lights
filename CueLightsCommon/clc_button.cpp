// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "CueLightsCommon.h"

CLCDebouncedButton::CLCDebouncedButton(unsigned int pin_in, CLCButtonType type_in, int debounceTime_in) {
    pin = pin_in;
    type = type_in;
    debounceTime = debounceTime_in;
    switch(type) {
        case ACTIVE_HIGH:
        case ACTIVE_LOW:
            pinMode(pin, INPUT);
            break;
        case ACTIVE_LOW_PULLUP:
            pinMode(pin, INPUT_PULLUP);
    }
}

bool CLCDebouncedButton::isPressed() {
    bool currState = digitalRead(pin);
    if (state != currState) {
        lastStateUpdate = millis();
        state = currState;
    }
    else if (lastAcceptedState != currState && millis() > lastStateUpdate + debounceTime){
        lastAcceptedState = currState;
    }

    bool returnVal;
    if (type == ACTIVE_HIGH) {
        returnVal = lastAcceptedState;
    }
    else {
        returnVal = !lastAcceptedState;
    }

    if (returnVal) {
        awaitingRelease = true;
    }

    return returnVal;
}

bool CLCDebouncedButton::wasReleased() {
    if (!isPressed() && awaitingRelease) {
        awaitingRelease = false;
        return true;
    }
    return false;
}