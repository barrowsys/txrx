/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : 28.8kbps test
 * @created     : Saturday May 22, 2021 15:35:09 EDT
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

#define ECHOS 10
#define ECHOSM 100
for(int m = 0; m < ECHOSM; m++) {
	#if DEVICE == 1
		set_msg(3);
		for(int i = 0; i < ECHOS; i++) {
			/* recieve(); */
			echo_full();
		}
	#elif DEVICE == 2
		set_msg(m % 4);
		delay(500);
		transmit();
		set_msg(3);
		for(int i = 1; i < ECHOS; i++) {
			/* transmit(); */
			echo_full();
		}
		recieve();
	#endif
}
Serial.println();
Serial.print(F("Tx Rate: "));
Serial.print(_TX_RATE, DEC);
Serial.print(F("bps, "));
Serial.print(500000 / _TX_RATE, DEC);
Serial.println(F("us"));
Serial.print(F("Echos: "));
Serial.println(ECHOS * ECHOSM, DEC);
Serial.print(F("Failures: "));
Serial.println(failures, DEC);
