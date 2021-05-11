/**
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
#include "Crc16.h"
#define DEBUG
#include "NanoNet.h"

char rx_buf[1024];
NanoNet nanonet = NanoNet(0x01, false);

void setup() {
	Serial.begin(9600);
	Serial.println();
	Serial.println("I'm 0x01 (Recieve Only)");
	Serial.println();
}

void loop() {
	/* serial_echo(); */
	rx_nanonet_frame();
	/* serial_commands(); */
}

byte rx_nanonet_frame() {
	byte r = nanonet.recieveFrame(rx_buf);
	if(r != 0) {
		Serial.print("Message from 0x");
		Serial.print(r, HEX);
		Serial.print(": ");
		Serial.println(rx_buf);
	}
	return r;
}

void print_pin_state(byte pin) {
	Serial.print("Pin ");
	Serial.print(pin, DEC);
	Serial.print(": ");
	if(digitalRead(pin)) {
		Serial.print("HIGH");
	} else {
		Serial.print("LOW");
	}
	Serial.println();
}

bool PPS_DELAY_ENABLED = false;
unsigned long PPS_DELAY_MS = 50;

void print_pin_states() {
	if(PPS_DELAY_ENABLED) {
		delay(PPS_DELAY_MS);
	}
	print_pin_state(10);
	print_pin_state(12);
	Serial.println();
}

void test_pinmode_stuff_2() {
	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pins 10 and 12 set to INPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set HIGH");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pin 12 set LOW");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set HIGH");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, HIGH);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pin 12 set LOW");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
}

void test_pinmode_stuff() {
	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
	Serial.println("pins 10 and 12 set to INPUT");
	print_pin_states();

	pinMode(12, OUTPUT);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	digitalWrite(12, HIGH);
	Serial.println("pin 12 set HIGH");
	print_pin_states();

	pinMode(12, INPUT);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	pinMode(12, OUTPUT);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	digitalWrite(12, LOW);
	Serial.println("pin 12 set LOW");
	print_pin_states();

	pinMode(12, INPUT);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	digitalWrite(12, HIGH);
	Serial.println("pin 12 set HIGH");
	print_pin_states();

	pinMode(12, OUTPUT);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	pinMode(12, INPUT);
	Serial.println("pin 12 set to INPUT");
	print_pin_states();

	pinMode(12, OUTPUT);
	Serial.println("pin 12 set to OUTPUT");
	print_pin_states();

	digitalWrite(12, LOW);
	Serial.println("pin 12 set LOW");
	print_pin_states();

	pinMode(10, INPUT);
	pinMode(12, INPUT);
	digitalWrite(10, LOW);
	digitalWrite(12, LOW);
}

void serial_commands() {
	if(Serial.available() > 0) {
		byte incoming_byte = Serial.read();
		switch(incoming_byte) {
			case '?': {
				Serial.println("?: help");
				Serial.println("r: recieve nanonet frame");
				Serial.println("p: pinmode test");
				Serial.println("P: pinmode test 2");
				Serial.println("d: toggle delay in pinmode test");
				Serial.println("D: show pinmode test config");
				Serial.println("l: increase delay by 50ms");
				Serial.println("L: increase delay by 250ms");
				Serial.println("o: decrease delay by 50ms");
				Serial.println("O: decrease delay by 250ms");
				break;
			}
			case 'r': {
				byte r = rx_nanonet_frame();
				if(r == 0) {
					Serial.println("Bad Data!");
				}
				break;
			}
			case 'p': {
				test_pinmode_stuff();
				break;
			}
			case 'P': {
				test_pinmode_stuff_2();
				break;
			}
			case 'D': {
				if(PPS_DELAY_ENABLED) {
					Serial.println("pps delay enabled");
				} else {
					Serial.println("pps delay disabled");
				}
				Serial.print("pps delay: ");
				Serial.println(PPS_DELAY_MS, DEC);
				break;
			}
			case 'd': {
				PPS_DELAY_ENABLED = !PPS_DELAY_ENABLED;
				if(PPS_DELAY_ENABLED) {
					Serial.println("pps delay enabled");
				} else {
					Serial.println("pps delay disabled");
				}
				break;
			}
			case 'l': {
				PPS_DELAY_MS += 50;
				Serial.print("pps delay increased to ");
				Serial.println(PPS_DELAY_MS, DEC);
				break;
			}
			case 'L': {
				PPS_DELAY_MS += 250;
				Serial.print("pps delay increased to ");
				Serial.println(PPS_DELAY_MS, DEC);
				break;
			}
			case 'o': {
				if(PPS_DELAY_MS <= 100) {
					PPS_DELAY_MS = 50;
				} else {
					PPS_DELAY_MS -= 50;
				}
				Serial.print("pps delay decreased to ");
				Serial.println(PPS_DELAY_MS, DEC);
				break;
			}
			case 'O': {
				if(PPS_DELAY_MS <= 300) {
					PPS_DELAY_MS = 50;
				} else {
					PPS_DELAY_MS -= 250;
				}
				Serial.print("pps delay decreased to ");
				Serial.println(PPS_DELAY_MS, DEC);
				break;
			}
		}
	}
}

void serial_echo() {
	if(Serial.available() > 0) {
		byte incoming_byte = Serial.read();
		if(incoming_byte != -1) {
			Serial.print("Recieved: ");
			Serial.println(incoming_byte, DEC);
			/* Serial.write('\n'); */
		}
	}
}
