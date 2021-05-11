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
#include "NanoNet.h"

char *message = "Debug Message";
char *message1 = "Goodbye Non-world!!!";
char *message2 = "Hello World!!!";
char *message3 = ":3";
char **messageptr = &message2;
char rx_buf[256];
char r2;
volatile int bits_n = 0;
NanoNet nanonet = NanoNet(MY_ADDR, _CLOCK_PIN, _DATA_PIN);

void clear_rx_buf() {
	memset(rx_buf, 0, 256);
}

void isr() {
	if(digitalRead(_CLOCK_PIN) != LOW) {
		bool rx_bit = digitalRead(_DATA_PIN) == HIGH;
		NN_rx_buf.s = NN_rx_buf.s << 1;
		if(rx_bit) {
			NN_rx_buf.s |= 0x01;
		}
		NN_new_bit = true;
	}
}

void set_msg(byte msg) {
	if(msg == '0') {
		messageptr = &message;
	} else if(msg == '1') {
		messageptr = &message1;
	} else if(msg == '2') {
		messageptr = &message2;
	} else if(msg == '3') {
		messageptr = &message3;
	}
}
		

void setup() {
	Serial.begin(9600);
	Serial.println("\n\n\n\n\rI'm "MY_ADDR_S);
	pinMode(_CLOCK_PIN, INPUT);
	pinMode(_DATA_PIN, INPUT);
	pinMode(_STATUS_PIN, OUTPUT);
	pinMode(_OPT_PIN_1, INPUT_PULLUP);
	pinMode(_OPT_PIN_2, INPUT_PULLUP);
	pinMode(_OPT_PIN_3, INPUT_PULLUP);
	pinMode(_OPT_PIN_4, INPUT_PULLUP);
	pinMode(_LOW_PIN_1, OUTPUT);
	pinMode(_LOW_PIN_2, OUTPUT);
	digitalWrite(_LOW_PIN_1, LOW);
	digitalWrite(_LOW_PIN_2, LOW);
	attachInterrupt(digitalPinToInterrupt(_CLOCK_PIN), isr, RISING);
}

char set_cmd_ltr = 0;
void loop() {
	byte r = 0;
	char cmd_ltr;
	bool o1 = !digitalRead(_OPT_PIN_1);
	bool o2 = !digitalRead(_OPT_PIN_2);
	bool o3 = !digitalRead(_OPT_PIN_3);
	bool o4 = !digitalRead(_OPT_PIN_4);
	if(o1) {
		nanonet._tx_rate = TX_RATE;
	} else {
		nanonet._tx_rate = 1;
	}
	if(o3) {
		set_cmd_ltr = 0;
	} else if(o2) {
		if(o4) {
			set_cmd_ltr = 'E';
		} else {
			set_cmd_ltr = 'e';
		}
	}
	if(set_cmd_ltr != 0) {
		cmd_ltr = set_cmd_ltr;
	} else {
		cmd_ltr = 0;
	}
	if(Serial.available() > 0) {
		cmd_ltr = Serial.read();
	}
	switch(cmd_ltr) {
	case 's':
		printbln(F("Debug Level: "), DEBUG_LEVEL);
		Serial.println(F("Address: "MY_ADDR_S));
		Serial.print(F("Transmit Buffer: \""));
		Serial.print(*messageptr);
		Serial.println(F("\""));
		Serial.print(F("Recieve Buffer: \""));
		Serial.print(rx_buf);
		Serial.println(F("\""));
		Serial.print(F("Opt Pin 1: "));
		printoln(!digitalRead(_OPT_PIN_1));
		Serial.print(F("Opt Pin 2: "));
		printoln(!digitalRead(_OPT_PIN_2));
		Serial.print(F("Opt Pin 3: "));
		printpinln(digitalRead(_OPT_PIN_3));
		Serial.print(F("Opt Pin 4: "));
		printpinln(digitalRead(_OPT_PIN_4));
		break;
	case 'd':
		Serial.print(F("Set Debug Level to "));
		Serial.println(((DEBUG_LEVEL + 2) % 5)-1, DEC);
	case 'D':
		DEBUG_LEVEL = ((DEBUG_LEVEL + 2) % 5)-1;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
		set_msg(cmd_ltr);
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
		LOG_ERROR(F("Sending to "OTHER_ADDR_S"..."));
		do {
			r = nanonet.sendFrame(*messageptr, OTHER_ADDR, 0);
			delay(100);
		} while (r == 1);
		if(r == 0) {
			LOG_FATAL(F("Sent message to "OTHER_ADDR_S));
		}
		break;
	case 't':
		LOG_ERROR(F("Sending to "OTHER_ADDR_S" with RACK..."));
		do {
			r = nanonet.sendFrame(*messageptr, OTHER_ADDR, OPT_RACK);
			delay(100);
		} while (r == 1);
		if(r == 0) {
			LOG_FATAL(F("Sent message to "OTHER_ADDR_S));
		}
		break;
	case 'e':
		LOG_ERROR(F("Waiting for frame..."));
		clear_rx_buf();
		r2 = nanonet.recieveFrame(rx_buf);
		if(r2 != 0 && r2 != -1) {
			LOG_ERROR(F("Message from "));
			printb(F("0x"), r2, F(": "));
			Serial.println(rx_buf);
			rx_buf[strlen(rx_buf)] = ' ';
			rx_buf[strlen(rx_buf)] = ':';
			rx_buf[strlen(rx_buf)] = '3';
			rx_buf[strlen(rx_buf)] = 0;
	case 'E':
			_LOG_FATAL(F("Echoing"));
			IFERROR(printb(F("back to 0x"), r2, F(",,,")));
			LOG_FATAL();
			do {
				r = nanonet.sendFrame(rx_buf, r2, OPT_RACK);
				delay(100);
			} while (r == 1);
			if(r == 0) {
				IFERROR(printbln(F("Sent message to "), r2));
			}
		}
		break;
	case 'c':
		clear_rx_buf();
		bits_n = 0;
		LOG_TRACE(F("Recieve buffer cleared"));
		break;
	case 13:
		Serial.println();
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
