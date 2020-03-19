// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <digitalWriteFast.h>

#include <CueLightsCommon.h>
#include "constants.h"
#include "station.h"

void panelIndicatorSetup() {
    for (int i=0; i<MAX_STATIONS+1; i++) {
        pinMode(ledEnablePins[i], OUTPUT);
    }
    for (int i=0; i<3; i++) {
        pinMode(ledPwmPins[i], OUTPUT);
    }
}

