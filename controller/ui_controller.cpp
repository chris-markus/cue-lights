// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "ui_controller.h"

UIController::UIController(Encoder* encoder_in, CLCDebouncedButton* encoder_btn_in, NavigationController* navigation_in) {
    encoder = encoder_in;
    encoder_btn = encoder_btn_in;
    navigation = navigation_in;
}

void UIController::tick() {
    static long offset = 0;
    static bool didPress = false;
    long reading = encoder->read();
    if (reading > offset + 3) {
        navigation->selectNext();
        offset = reading;
    }
    else if (reading < offset - 3) {
        navigation->selectPrev();
        offset = reading;
    }
    if (!digitalRead(53)) {
        if (!didPress) {
        navigation->dispatchPress();
        didPress = true;
        delay(100);
        }
    }
    else if (didPress) {
        didPress = false;
        delay(100);
    }
}