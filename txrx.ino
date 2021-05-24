/*
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

#include "Definitions.h"

char *message = "Debug Message";
char *message1 = "Hello World!!!";
char *message2 = "Goodbye Non-world!!!";
char *message3 = ":3";
char **messageptr = &message2;

void setup() {
	Serial.begin(9600);
	Serial.println(F("\n\rI'm "MY_ADDR_S"\n\r"));
	#ifdef RUN_TEST
	#include RUN_TEST
	#endif
}

void loop() {
}
