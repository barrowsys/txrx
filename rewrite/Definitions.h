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

#define _CLOCK_PIN 2
#define _DATA_PIN 4
#define _OPT_PIN_1 6
#define _OPT_PIN_2 5
#define _OPT_PIN_3 7
#define _OPT_PIN_4 10
#define _LOW_PIN_1 9
#define _LOW_PIN_2 12
#define _STATUS_PIN 13

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

// Logging Helpers
short DEBUG_LEVEL = _DEBUG_LEVEL;
#define LOG_FATAL(...) 	if(DEBUG_LEVEL >= 0) { Serial.println(__VA_ARGS__); }
#define LOG_ERROR(...) 	if(DEBUG_LEVEL >= 1) { Serial.println(__VA_ARGS__); }
#define LOG_WARN(...) 	if(DEBUG_LEVEL >= 2) { Serial.println(__VA_ARGS__); }
#define LOG_INFO(...) 	if(DEBUG_LEVEL >= 3) { Serial.println(__VA_ARGS__); }
#define LOG_TRACE(...) 	if(DEBUG_LEVEL >= 4) { Serial.println(__VA_ARGS__); }
#define LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printbln(MSG, " "); }
#define LOG_BIT(MSG) 	if(DEBUG_LEVEL >= 4) { if(MSG) { Serial.println(F("1")); } else { Serial.println(F("0")); } }
#define LOG_PIN(MSG) 	if(DEBUG_LEVEL >= 4) { print_pinln(MSG); }
#define _LOG_FATAL(...) if(DEBUG_LEVEL >= 0) { Serial.print(__VA_ARGS__); }
#define _LOG_ERROR(...) if(DEBUG_LEVEL >= 1) { Serial.print(__VA_ARGS__); }
#define _LOG_WARN(...) 	if(DEBUG_LEVEL >= 2) { Serial.print(__VA_ARGS__); }
#define _LOG_INFO(...) 	if(DEBUG_LEVEL >= 3) { Serial.print(__VA_ARGS__); }
#define _LOG_TRACE(...) if(DEBUG_LEVEL >= 4) { Serial.print(__VA_ARGS__); }
#define _LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printb(MSG, " "); }
#define _LOG_BIT(MSG) 	if(DEBUG_LEVEL >= 4) { if(MSG) { Serial.print(F("1")); } else { Serial.print(F("0")); } }
#define _LOG_PIN(MSG) 	if(DEBUG_LEVEL >= 4) { print_pin(MSG); }
#define _LOG_BYTE_SEP() if(DEBUG_LEVEL >= 4) { Serial.println(); }
#define IFFATAL(BODY) 	if(DEBUG_LEVEL >= 0) { BODY; }
#define IFERROR(BODY) 	if(DEBUG_LEVEL >= 1) { BODY; }
#define IFWARN(BODY) 	if(DEBUG_LEVEL >= 2) { BODY; }
#define IFINFO(BODY) 	if(DEBUG_LEVEL >= 3) { BODY; }
#define IFTRACE(BODY) 	if(DEBUG_LEVEL >= 4) { BODY; }

