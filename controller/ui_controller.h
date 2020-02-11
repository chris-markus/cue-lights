// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <Encoder.h>
#include <CueLightsCommon.h>
#include "lcd_screen.h"
#include "menu.h"

#define ENCODER_STEPS_PER_CLICK 4

class UIController {
public:
    UIController(Encoder* encoder_in, CLCDebouncedButton* encoder_btn_in, NavigationController* navigation_in);
    void tick();
    void displayCurrentMenu();
private:
    Encoder* encoder;
    CLCDebouncedButton* encoder_btn;
    NavigationController* navigation;
    long offset = 0;
};

#endif