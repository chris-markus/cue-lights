// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "CueLightsCommon.h"

CLCDebouncedButton::CLCDebouncedButton(unsigned int pin_in, CLCButtonType type_in) {
    pin = pin_in;
    type = type_in;
    switch(type) {
        case ACTIVE_HIGH:
        case ACTIVE_LOW:
            pinMode(pin, INPUT);
            break;
        case ACTIVE_LOW_PULLUP:
            pinMode(pin, INPUT_PULLUP);
    }
}