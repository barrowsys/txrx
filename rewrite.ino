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

int failures = 0;
#include "Definitions.h"
#include "NanoNet.orig.h"
#include "NanoNet.noir.h"

char *message = "Debug Message";
char *message1 = "Hello World!!!";
char *message2 = "Goodbye Non-world!!!";
char *message3 = ":3";
char **messageptr = &message2;
char rx_buf[256];
char *srx_buf = &(rx_buf[0]);
byte rx_addr;
bool remote[8];
char set_command = 0;
bool use_rack = 0;
volatile int bits_n = 0;
/* NanoNetOrig nanonet; */
NanoNetNoir nanonet;

void clear_rx_buf() {
	memset(rx_buf, 0, 256);
}

bool should_cancel() {
	return false;
}

void setup() {
	Serial.begin(9600);
	Serial.println(F("\n\rI'm "MY_ADDR_S));
	nanonet.init(MY_ADDR, _CLOCK_PIN, _DATA_PIN);
	pinMode(_CLOCK_PIN, INPUT);
	pinMode(_DATA_PIN, INPUT);
	pinMode(_STATUS_PIN, OUTPUT);
	/* Serial.print(F("Tx Rate: ")); */
	/* Serial.print(_TX_RATE, DEC); */
	/* Serial.print(F("bps, ")); */
	/* Serial.print(500000 / _TX_RATE, DEC); */
	/* Serial.println(F("us")); */
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
}

void loop() {
	char cmd_ltr = 0;
	if(Serial.available() > 0) {
		cmd_ltr = Serial.read();
	}
	switch(cmd_ltr) {
		case 's':
			print_status();
			break;

		case '0':
		case '1':
		case '2':
		case '3':
			set_msg(cmd_ltr);
			break;

		case 'd':
			Serial.print(F("Set Debug Level to "));
			Serial.println(((DEBUG_LEVEL + 2) % 6)-1, DEC);
		case 'D':
			DEBUG_LEVEL = ((DEBUG_LEVEL + 2) % 6)-1;
			break;

		case 'c':
			clear_rx_buf();
			bits_n = 0;
			LOG_TRACE(F("Recieve buffer cleared"));
			break;

		case 13:
			Serial.println();
			break;

		case 'r':
			recieve();
			break;

		case 't':
			transmit(true);
			break;
		case 'T':
			transmit(false);
			break;

		case 'e':
			echo_full();
			break;
		case 'E':
			echo_skip();
			break;
	}
}

void print_status() {
	printbln(F("Debug Level: "), DEBUG_LEVEL);
	Serial.println(F("Address: "MY_ADDR_S));
	Serial.print(F("Transmit Buffer: \""));
	Serial.print(*messageptr);
	Serial.println(F("\""));
	Serial.print(F("Recieve Buffer: \""));
	Serial.print(rx_buf);
	Serial.println(F("\""));
	Serial.print(F("Remote Data: "));
	for(byte i = 0; i < 8; i++) {
		printbit(remote[i]);
		if(i == 5) { Serial.print(F(" ")); }
	}
	Serial.println();
	Serial.println();
}

bool recieve() {
	LOG_ERROR(F("Waiting for frame..."));
	clear_rx_buf();
	rx_addr = nanonet.recieveFrame(rx_buf);
	if(rx_addr != 0 && rx_addr != 255) {
		LOG_ERROR(F("Message from "));
		printb(F("0x"), rx_addr, F(": "));
		Serial.println(rx_buf);
		return true;
	} else {
		return false;
	}
}

void transmit() 				{ transmit(true, false, false); }
void transmit(bool use_rack) 	{ transmit(use_rack, false, false); }
void transmit(bool use_rack, bool to_rx, bool use_rx_buf) {
	byte options = 0;
	byte tx_addr = OTHER_ADDR;
	byte r;
	char *_tx_buf = *messageptr;
	if(to_rx) { tx_addr = rx_addr; }
	if(use_rx_buf) { _tx_buf = &rx_buf[0]; }
	_LOG_ERROR(F("Sending to "OTHER_ADDR_S));
	if(use_rack) { options = OptRack; }
	if(use_rack) { _LOG_ERROR(F(" with RACK")); }
	LOG_ERROR(F("..."));
	/* delay(50); */
	do {
		r = nanonet.sendFrame(_tx_buf, tx_addr, options);
		delay(50);
	} while (r == 1 && !should_cancel());
	if(r == 0) {
		LOG_FATAL(F("Sent message to "OTHER_ADDR_S));
	}
}

void echo_full() 				{ echo(true, false, false); }
void echo_skip() 				{ echo(true, true, true); }
void echo_full(bool use_rack) 	{ echo(use_rack, false, false); }
void echo_skip(bool use_rack) 	{ echo(use_rack, true, true); }
void echo(bool use_rack, bool to_other, bool skip_recieve) {
	if(skip_recieve || recieve()) {
		append_str(rx_buf, *messageptr, 256);
		_LOG_FATAL(F("Echoing"));
		IFERROR(printb(F("back to 0x"), rx_addr, F(",,,")));
		LOG_FATAL();
		delay(50);
		transmit(use_rack, !to_other, true);
	}
}

void set_msg(byte msg) {
	if(msg == '0' || msg == 0) {
		messageptr = &message;
	} else if(msg == '1' || msg == 1) {
		messageptr = &message1;
	} else if(msg == '2' || msg == 2) {
		messageptr = &message2;
	} else if(msg == '3' || msg == 3) {
		messageptr = &message3;
	}
}

bool append_str(char* buffer, char* str, long length) {
	long alen = strlen(str);
	long blen = strlen(buffer);
	long bsize = length-1;
	if(blen > 0 && blen < bsize) {
		buffer[blen++] = ' ';
	}
	for(long i = 0; i < alen; i++) {
		if(blen < bsize) {
			buffer[blen++] = str[i];
		} else {
			return false;
		}
	}
	return true;
}
