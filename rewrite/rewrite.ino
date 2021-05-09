/*
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
#include "Definitions.h"
#include "NanoNet.h"

char *message = "Debug Message";
char *message1 = "Goodbye Non-world!!!";
char *message2 = "Hello World!!!";
char **messageptr = &message1;
char rx_buf[256];
volatile int bits_n = 0;
NanoNet nanonet = NanoNet(MY_ADDR, 2, 4);

void clear_rx_buf() {
	memset(rx_buf, 0, 256);
}

void isr() {
	if(digitalRead(2) == LOW) {
		bits_n += 1;
		Serial.println(bits_n, DEC);
	}
	bool rx_bit = digitalRead(4) == HIGH;
	NN_rx_buf.s = NN_rx_buf.s << 1;
	if(rx_bit) {
		NN_rx_buf.s |= 0x01;
	}
	NN_new_bit = true;
}

void setup() {
	Serial.begin(9600);
	Serial.println("I'm "MY_ADDR_S);
	pinMode(2, INPUT);
	pinMode(4, INPUT);
	pinMode(13, OUTPUT);
	attachInterrupt(digitalPinToInterrupt(2), isr, RISING);
}

void loop() {
	byte r = 0;
	char cmd_ltr = 0;
	if(Serial.available() > 0) {
		cmd_ltr = Serial.read();
	}
	switch(cmd_ltr) {
	case 'S':
		printbln("DEBUG_LEVEL: ", DEBUG_LEVEL);
	case 's':
		Serial.println("Address: "MY_ADDR_S);
		Serial.print("Transmit Buffer: \"");
		Serial.print(*messageptr);
		Serial.println("\"");
		Serial.print("Recieve Buffer: \"");
		Serial.print(rx_buf);
		Serial.println("\"");
		break;
	case 'd':
		DEBUG_LEVEL = (DEBUG_LEVEL + 1) % 5;
		printbln("Set Debug Level to ", DEBUG_LEVEL);
		break;
	case '0':
		messageptr = &message;
		Serial.println("Set tx buffer to message 0");
		break;
	case '1':
		messageptr = &message1;
		Serial.println("Set tx buffer to message 1");
		break;
	case '2':
		messageptr = &message2;
		Serial.println("Set tx buffer to message 2");
		break;
	case 'r':
		Serial.println("Waiting for frame...");
		clear_rx_buf();
		r = nanonet.recieveFrame(rx_buf);
		if(r == 0) {
			Serial.print("Garbled Message: ");
		} else {
			printb("Message from 0x", r, ": ");
		}
		Serial.println(rx_buf);
		break;
	case 't':
		Serial.println("Sending to "OTHER_ADDR_S"...");
		while(nanonet.sendFrame(*messageptr, OTHER_ADDR)) { delay(100); }
		Serial.println("Successfully sent to "OTHER_ADDR_S);
		break;
	case 'e':
		Serial.println("Waiting for frame...");
		clear_rx_buf();
		r = nanonet.recieveFrame(rx_buf);
		if(r == 0) {
			Serial.print("Garbled Message: ");
		} else {
			printb("Message from 0x", r, ": ");
		}
		Serial.println(rx_buf);
		if(r != 0) {
			printbln("Echoing back to 0x", r, ",,,");
			while(nanonet.sendFrame(rx_buf, r)) { delay(100); }
			printbln("Successfully sent to 0x", r);
		}
		break;
	case 'c':
		clear_rx_buf();
		bits_n = 0;
		Serial.println("Recieve buffer cleared");
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
/* 			Serial.print("Recieved: "); */
/* 			Serial.println(incoming_byte, DEC); */
/* 			/1* Serial.write('\n'); *1/ */
/* 		} */
/* 	} */
/* } */
