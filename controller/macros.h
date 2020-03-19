// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef MACROS_H
#define MACROS_H

// macros to allow us to use digitalWriteFast on the led enable pins

// pin definitions:
#define LED_EN_0 42
#define LED_EN_1 40
#define LED_EN_2 38
#define LED_EN_3 36
#define LED_EN_4 34
#define LED_EN_5 32
#define LED_EN_6 30
#define LED_EN_7 28
#define LED_EN_8 26
#define LED_EN_9 24
#define LED_EN_10 22

#define digitalWriteLEDEnable(i,x) switch(i) {\
    case 0: digitalWriteFast(LED_EN_0,x);break;\
    case 1: digitalWriteFast(LED_EN_1,x); break;\
    case 2: digitalWriteFast(LED_EN_2,x); break;\
    case 3: digitalWriteFast(LED_EN_3,x); break;\
    case 4: digitalWriteFast(LED_EN_4,x); break;\
    case 5: digitalWriteFast(LED_EN_5,x); break;\
    case 6: digitalWriteFast(LED_EN_6,x); break;\
    case 7: digitalWriteFast(LED_EN_7,x); break;\
    case 8: digitalWriteFast(LED_EN_8,x); break;\
    case 9: digitalWriteFast(LED_EN_9,x); break;\
    case 10: digitalWriteFast(LED_EN_10,x); break;}

#endif