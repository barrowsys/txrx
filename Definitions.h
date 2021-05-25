/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet Extra Definitions
 * @created     : Sunday May 09, 2021 13:48:26 EDT
 *
 * THIS FILE IS LICENSED UNDER THE FOLLOWING TERMS
 *
 * this code may not be used for any purpose. be gay, do crime
 *
 * THE FOLLOWING MESSAGE IS NOT A LICENSE
 *
 * <barrow@tilde.team> wrote this file.
 * by reading this message, you are reading "TRANS RIGHTS".
 * this file and the content within it is the queer agenda.
 * if we meet some day, and you think this stuff is worth it,
 * you can buy me a beer, tea, or something stronger.
 * -Ezra Barrow
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifndef MAKE_OVERRIDE
#include "/home/barrow/Apps/arduino/hardware/arduino/avr/cores/arduino/Arduino.h"
#include "/home/barrow/Apps/arduino/hardware/arduino/avr/variants/standard/pins_arduino.h"
#define MY_ADDR 1
#define NN_CLOCK_PIN 2
#define NN_DATA_PIN 4
#define NN_STATUS_PIN 13
#define NN_TX_RATE 20
#endif

#define ONE_SECOND 1000000
#define HALF_SECOND 500000
#define PER_SECOND(HZ) (ONE_SECOND / (HZ))
#define DELAY(us) if(us <= 16000) { delayMicroseconds(us); } else { delay(us / 1000); }

// String versions of compiler flags
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MY_ADDR_S STR(MY_ADDR)

union twobytes {
	volatile short s;
	volatile byte b[2];
};

#endif
