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
    static bool didPress = false;
    long reading = encoder->read();
    if (reading >= offset + ENCODER_STEPS_PER_CLICK) {
        navigation->selectNext();
        offset = reading;
    }
    else if (reading <= offset - ENCODER_STEPS_PER_CLICK) {
        navigation->selectPrev();
        offset = reading;
    }
    if (encoder_btn->isPressed()) {
        if (!didPress) {
            navigation->dispatchPress();
            didPress = true;
        }
    }
    else if (didPress) {
        didPress = false;
    }
    navigation->tick();
}

void UIController::displayCurrentMenu() {
    offset = encoder->read();
    navigation->display();
}