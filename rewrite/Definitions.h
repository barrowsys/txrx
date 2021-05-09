/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet Extra Definitions
 * @created     : Sunday May 09, 2021 13:48:26 EDT
 *
 * THIS FILE IS LICENSED UNDER MIT
 * THE FOLLOWING MESSAGE IS NOT A LICENSE
 *
 * <barrow@tilde.team> wrote this file.
 * by reading this message, you are reading "TRANS RIGHTS".
 * this file and the content within it is the queer agenda.
 * if we meet some day, and you think this stuff is worth it,
 * you can buy me a beer, tea, or something stronger.
 * -Ezra Barrow
 */

// String versions of compiler flags
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MY_ADDR_S STR(MY_ADDR)
#define OTHER_ADDR_S STR(OTHER_ADDR)

//Serial Printing Helpers
void printb(byte b) {
	if(b < 0x10) { Serial.print("0"); }
	Serial.print(b, HEX);
}
void printb(char* pre, byte b) 				{ Serial.print(pre); printb(b); }
void printb(char* pre, byte b, char* post) 	{ Serial.print(pre); printb(b); Serial.print(post); }
void printb(byte b, char* post) 			{ printb(b); Serial.print(post); }
#define printbln(...) printb(__VA_ARGS__); Serial.println();

// Logging Helpers
char DEBUG_LEVEL = _DEBUG_LEVEL;
#define LOG_FATAL(MSG) 	if(DEBUG_LEVEL >= 0) { Serial.println(MSG); }
#define LOG_ERROR(MSG) 	if(DEBUG_LEVEL >= 1) { Serial.println(MSG); }
#define LOG_WARN(MSG) 	if(DEBUG_LEVEL >= 2) { Serial.println(MSG); }
#define LOG_INFO(MSG) 	if(DEBUG_LEVEL >= 3) { Serial.println(MSG); }
#define LOG_TRACE(MSG) 	if(DEBUG_LEVEL >= 4) { Serial.println(MSG); }
#define LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printbln(MSG, " "); }
#define _LOG_FATAL(MSG) if(DEBUG_LEVEL >= 0) { Serial.print(MSG); }
#define _LOG_ERROR(MSG) if(DEBUG_LEVEL >= 1) { Serial.print(MSG); }
#define _LOG_WARN(MSG) 	if(DEBUG_LEVEL >= 2) { Serial.print(MSG); }
#define _LOG_INFO(MSG) 	if(DEBUG_LEVEL >= 3) { Serial.print(MSG); }
#define _LOG_TRACE(MSG) if(DEBUG_LEVEL >= 4) { Serial.print(MSG); }
#define _LOG_BYTE(MSG) 	if(DEBUG_LEVEL >= 4) { printb(MSG, " "); }
#define _LOG_BYTE_SEP() if(DEBUG_LEVEL >= 4) { Serial.println(); }

