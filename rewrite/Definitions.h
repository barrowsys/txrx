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

#include "/home/barrow/Apps/arduino/hardware/arduino/avr/cores/arduino/Arduino.h"
#include "/home/barrow/Apps/arduino/hardware/arduino/avr/variants/standard/pins_arduino.h"

#ifndef OVERRIDE_DEFS
#define _CLOCK_PIN 2
#define _DATA_PIN 4
#define _STATUS_PIN 13
#define _TX_RATE 20
#define _MAX_DEBUG 4
#endif

#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(...)
#endif

#define FULL_TICK (1000000 / tx_rate)
#define HALF_TICK (500000 / tx_rate)
#define DELAY(us) if(us <= 16000) { delayMicroseconds(us); } else { delay(us / 1000); }

// String versions of compiler flags
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MY_ADDR_S STR(MY_ADDR)
#define OTHER_ADDR_S STR(OTHER_ADDR)

//Serial Printing Helpers
void printb(byte b) {
	if(b < 0x10) { Serial.print(F("0")); }
	Serial.print(b, HEX);
}
void printb(char* pre, byte b) 				{ Serial.print(pre); printb(b); }
void printb(char* pre, byte b, char* post) 	{ Serial.print(pre); printb(b); Serial.print(post); }
void printb(byte b, char* post) 			{ printb(b); Serial.print(post); }
void printb(__FlashStringHelper* pre, byte b) 				{ Serial.print(pre); printb(b); }
void printb(__FlashStringHelper* pre, byte b, __FlashStringHelper* post) 	{ Serial.print(pre); printb(b); Serial.print(post); }
void printb(byte b, __FlashStringHelper* post) 			{ printb(b); Serial.print(post); }
#define printbln(...) printb(__VA_ARGS__); Serial.println();
#define printpin(pin) if(pin) { Serial.print(F("HIGH")); } else { Serial.print(F("LOW")); }
#define printo(pin) if(pin) { Serial.print(F("ON")); } else { Serial.print(F("OFF")); }
#define printpinln(pin) printpin(pin); Serial.println();
#define printoln(pin) printo(pin); Serial.println();
#define printbit(pin) if(pin) { Serial.print(F("1")); } else { Serial.print(F("0")); }
#define printbitln(pin) printbit(pin); Serial.println();

// Logging Helpers
short DEBUG_LEVEL = _DEBUG_LEVEL;
#if _MAX_DEBUG >= 0
#define LOG_FATAL(...) 	if(DEBUG_LEVEL >= 0) { Serial.println(__VA_ARGS__); }
#define _LOG_FATAL(...) if(DEBUG_LEVEL >= 0) { Serial.print(__VA_ARGS__); }
#define IFFATAL(BODY) 	if(DEBUG_LEVEL >= 0) { BODY; }
#else
#define LOG_FATAL(...)
#define _LOG_FATAL(...)
#define IFFATAL(BODY)
#endif

#if _MAX_DEBUG >= 1
#define LOG_ERROR(...) 	if(DEBUG_LEVEL >= 1) { Serial.println(__VA_ARGS__); }
#define _LOG_ERROR(...) if(DEBUG_LEVEL >= 1) { Serial.print(__VA_ARGS__); }
#define IFERROR(BODY) 	if(DEBUG_LEVEL >= 1) { BODY; }
#else
#define LOG_ERROR(...)
#define _LOG_ERROR(...)
#define IFERROR(BODY)
#endif

#if _MAX_DEBUG >= 2
#define LOG_WARN(...) 	if(DEBUG_LEVEL >= 2) { Serial.println(__VA_ARGS__); }
#define _LOG_WARN(...) 	if(DEBUG_LEVEL >= 2) { Serial.print(__VA_ARGS__); }
#define IFWARN(BODY) 	if(DEBUG_LEVEL >= 2) { BODY; }
#else
#define LOG_WARN(...)
#define _LOG_WARN(...)
#define IFWARN(BODY)
#endif

#if _MAX_DEBUG >= 3
#define LOG_INFO(...) 	if(DEBUG_LEVEL >= 3) { Serial.println(__VA_ARGS__); }
#define _LOG_INFO(...) 	if(DEBUG_LEVEL >= 3) { Serial.print(__VA_ARGS__); }
#define IFINFO(BODY) 	if(DEBUG_LEVEL >= 3) { BODY; }
#else
#define LOG_INFO(...)
#define _LOG_INFO(...)
#define IFINFO(BODY)
#endif

#if _MAX_DEBUG >= 4
#define LOG_TRACE(...) 	if(DEBUG_LEVEL >= 4) { Serial.println(__VA_ARGS__); }
#define _LOG_TRACE(...) if(DEBUG_LEVEL >= 4) { Serial.print(__VA_ARGS__); }
#define IFTRACE(BODY) 	if(DEBUG_LEVEL >= 4) { BODY; }
#define LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printbln(MSG, " "); }
#define LOG_BIT(MSG) 	if(DEBUG_LEVEL >= 4) { printbitln(MSG); }
#define LOG_PIN(MSG) 	if(DEBUG_LEVEL >= 4) { print_pinln(MSG); }
#define _LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printb(MSG, " "); }
#define _LOG_BIT(MSG) 	if(DEBUG_LEVEL >= 4) { printbit(MSG); }
#define _LOG_PIN(MSG) 	if(DEBUG_LEVEL >= 4) { print_pin(MSG); }
#define _LOG_BYTE_SEP() if(DEBUG_LEVEL >= 4) { Serial.println(); }
#else
#define LOG_TRACE(...)
#define _LOG_TRACE(...)
#define IFTRACE(BODY)
#define LOG_BYTE(MSG)
#define LOG_BIT(MSG)
#define LOG_PIN(MSG)
#define _LOG_BYTE(MSG)
#define _LOG_BIT(MSG)
#define _LOG_PIN(MSG)
#define _LOG_BYTE_SEP()
#endif

// Hoisted Functions
bool should_cancel();
/* void read_remote(*byte array); */

#endif
