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
#include "NanoNet.orig.h"

char *message = "Debug Message";
char *message1 = "Hello World!!!";
char *message2 = "Goodbye Non-world!!!";
char *message3 = ":3";
char **messageptr = &message2;
char **tx_buf = &message2;
char rx_buf[256];
char *srx_buf = &(rx_buf[0]);
char rx_addr;
bool remote[8];
char set_command = 0;
bool use_rack = 0;
volatile int bits_n = 0;
NanoNetOrig nanonet;

void clear_rx_buf() {
	memset(rx_buf, 0, 256);
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
		

void setup() {
	Serial.begin(9600);
	Serial.println(F("\n\n\n\n\rI'm "MY_ADDR_S));
	nanonet.init(MY_ADDR, _CLOCK_PIN, _DATA_PIN);
	pinMode(_CLOCK_PIN, INPUT);
	pinMode(_DATA_PIN, INPUT);
	pinMode(_STATUS_PIN, OUTPUT);
	pinMode(3, INPUT);
	for(byte i = 5; i <= 12; i++) {
		pinMode(i, INPUT_PULLUP);
	}
}

bool should_cancel() {
	if(!digitalRead(8)) {
		nanonet.tx_rate = 1;
	} else {
		nanonet.tx_rate = _TX_RATE;
	}
	return !digitalRead(6);
}

bool read_remote() {
	bool changed = false;
	for(byte i = 0; i < 8; i++) {
		bool ns = !digitalRead(12-i);
		if(i<6 && remote[i] != ns) {
			changed = true;
		}
		remote[i] = ns;
	}
	return changed;
}
bool last_set = 0;
void parse_input() {
	bool changed = read_remote();
	use_rack = remote[5];
	set_msg(((remote[2] << 1) | remote[3]) + '0');
	tx_buf = messageptr;
	if(remote[4]) {
		nanonet.tx_rate = 1;
	} else {
		nanonet.tx_rate = _TX_RATE;
	}
	if(set_command == '+' || set_command == 'E') {
		set_command = 'e';
		LOG_ERROR(F("Did first transmission, echoing"));
	}
	char new_command = 0;
	if(remote[0]) {
		if(remote[1]) {
			new_command = 'E';
		} else {
			new_command = 'e';
		}
	} else {
		if(remote[1]) {
			new_command = 't';
		} else {
			new_command = 'r';
		}
	}
	if(changed) { print_remote(); }
	if(should_cancel()) {
		set_command = 0;
	} else if(remote[7] != last_set && remote[7]) {
		last_set = remote[7];
		set_command = new_command;
		Serial.println();
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

void print_remote() {
	Serial.print(F("Command In Remote: "));
	if(remote[0]) {
		if(remote[1]) {
			Serial.println(F("start echo"));
		} else {
			Serial.println(F("echo"));
		}
	} else {
		if(remote[1]) {
			Serial.println(F("transmit"));
		} else {
			Serial.println(F("receive"));
		}
	}
	Serial.print(F("RACK bit: "));
	printbitln(use_rack);
	Serial.print(F("Tx Buffer: \""));
	Serial.print(*messageptr);
	Serial.println(F("\""));
	Serial.print(F("Rx Buffer: \""));
	Serial.print(rx_buf);
	Serial.println(F("\""));
	Serial.print(F("Baud Rate: "));
	Serial.print(nanonet.tx_rate, DEC);
	Serial.println(F("bps"));
	Serial.println();
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
}

void loop() {
	byte r = 0;
	char cmd_ltr = 0;
	byte options = 0;
	parse_input();
	if(Serial.available() > 0) {
		cmd_ltr = Serial.read();
	} else if(set_command != 0) {
		cmd_ltr = set_command;
	}
	switch(cmd_ltr) {
		case 's':
			print_status();
			Serial.println();
			break;
		case 'S':
			print_remote();
			Serial.println();
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
			LOG_ERROR(F("Waiting for frame..."));
			clear_rx_buf();
			r = nanonet.recieveFrame(rx_buf);
			if(r != 0 && r != 255) {
				LOG_ERROR(F("Message from "));
				printb(F("0x"), r, F(": "));
				Serial.println(rx_buf);
			}
			break;

		case 'T':
			use_rack |= 2;
		case '+':
		case 't':
			if(use_rack) {
				LOG_ERROR(F("Sending to "OTHER_ADDR_S" with RACK..."));
				options = OptRack;
			} else {
				_LOG_ERROR(F("Sending to "OTHER_ADDR_S"..."));
			}
			use_rack &= 1;
			do {
				r = nanonet.sendFrame(*tx_buf, OTHER_ADDR, options);
				delay(100);
			} while (r == 1 && !should_cancel());
			if(r == 0) {
				LOG_FATAL(F("Sent message to "OTHER_ADDR_S));
			}
			break;

		case 'e':
			LOG_ERROR(F("Waiting for frame..."));
			clear_rx_buf();
			rx_addr = nanonet.recieveFrame(rx_buf);
			if(rx_addr != 0 && rx_addr != -1) {
				LOG_ERROR(F("Message from "));
				printb(F("0x"), rx_addr, F(": "));
				Serial.println(rx_buf);
				append_str(rx_buf, *messageptr, 256);
				/* rx_buf[strlen(rx_buf)] = ' '; */
				/* rx_buf[strlen(rx_buf)] = ':'; */
				/* rx_buf[strlen(rx_buf)] = '3'; */
				/* rx_buf[strlen(rx_buf)] = 0; */
		case 'E':
				if(cmd_ltr == 'E') {
					rx_addr = OTHER_ADDR;
				}
				_LOG_FATAL(F("Echoing"));
				IFERROR(printb(F("back to 0x"), rx_addr, F(",,,")));
				LOG_FATAL();
				if(use_rack) {
					options = OptRack;
				}
				do {
					r = nanonet.sendFrame(rx_buf, rx_addr, options);
					delay(100);
				} while (r == 1 && !should_cancel());
				if(r == 0) {
					IFERROR(printbln(F("Sent message to "), rx_addr));
				}
			}
			break;
	}
}

/* void serial_echo() { */
/* 	if(Serial.available() > 0) { */
/* 		byte incoming_byte = Serial.read(); */
/* 		if(incoming_byte != -1) { */
/* 			Serial.print(F("Recieved: ")); */
/* 			Serial.println(incoming_byte, DEC); */
/* 			/1* Serial.write(F('\n')); *1/ */
/* 		} */
/* 	} */
/* } */
