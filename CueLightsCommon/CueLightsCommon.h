// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <stdint.h>

#ifndef CLC_COMMON_H
#define CLC_COMMON_H

class CLCSerial {
    public:
        CLCSerial();
        int begin(int baud_rate);
        int asyncRecieve(char* msg);
    private:
        int sendMessage();
};

#endif