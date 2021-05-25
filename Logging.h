/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : Logging
 * @created     : Monday May 24, 2021 17:25:34 EDT
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

#ifndef LOGGING_H
#define LOGGING_H

//Serial Printing Helpers
inline void printn(int n, byte base, byte width) {
	int power = 1;
	for(int i = 2; i <= width; i++) {
		power *= base;
		if(n < power) { Serial.print(F("0")); }
	}
	Serial.print(n, base);
}
inline void printb(byte b) { printn(b, 16, 2); }
#define printnln(...) printn(__VA_ARGS__); Serial.println();
#define printbln(...) printb(__VA_ARGS__); Serial.println();

// Logging Helpers
#ifndef LOG_LEVEL_R
#define LOG_LEVEL_R 4
#endif
#ifndef LOG_LEVEL_C
#define LOG_LEVEL_C 4
#endif
short DEBUG_LEVEL = LOG_LEVEL_R;
#if LOG_LEVEL_C >= 0
#define IFFATAL(BODY) 	if(DEBUG_LEVEL >= 0) { BODY; }
#else 
#define IFFATAL(BODY)
#endif

#if LOG_LEVEL_C >= 1
#define IFERROR(BODY) 	if(DEBUG_LEVEL >= 1) { BODY; }
#else
#define IFERROR(BODY)
#endif

#if LOG_LEVEL_C >= 2
#define IFWARN(BODY) 	if(DEBUG_LEVEL >= 2) { BODY; }
#else
#define IFWARN(BODY)
#endif

#if LOG_LEVEL_C >= 3
#define IFINFO(BODY) 	if(DEBUG_LEVEL >= 3) { BODY; }
#else
#define IFINFO(BODY)
#endif

#if LOG_LEVEL_C >= 4
#define IFTRACE(BODY) 	if(DEBUG_LEVEL >= 4) { BODY; }
#else
#define IFTRACE(BODY)
#endif

#define LOG_FATAL(...) 	IFFATAL(Serial.println(__VA_ARGS__))
#define _LOG_FATAL(...) IFFATAL(Serial.print(__VA_ARGS__))
#define LOG_ERROR(...) 	IFERROR(Serial.println(__VA_ARGS__))
#define _LOG_ERROR(...) IFERROR(Serial.print(__VA_ARGS__))
#define LOG_WARN(...) 	IFWARN(Serial.println(__VA_ARGS__))
#define _LOG_WARN(...)  IFWARN(Serial.print(__VA_ARGS__))
#define LOG_INFO(...) 	IFINFO(Serial.println(__VA_ARGS__))
#define _LOG_INFO(...)  IFINFO(Serial.print(__VA_ARGS__))
#define LOG_TRACE(...) 	IFTRACE(Serial.println(__VA_ARGS__))
#define _LOG_TRACE(...) IFTRACE(Serial.print(__VA_ARGS__))
#define LOG_BYTE(MSG) 	IFTRACE(printb(MSG); Serial.println();)
#define _LOG_BYTE(MSG) 	IFTRACE(printb(MSG); Serial.print(F(" "));)
#define _LOG_BYTE_SEP() IFTRACE(Serial.println();)

#endif
