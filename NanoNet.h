/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet (but with interfaces this time)
 * @created     : Friday May 21, 2021 17:24:33 EDT
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
#ifndef NANONET_H
#define NANONET_H

#include "Definitions.h"
#include "Crc16.h"

typedef enum State {
	NN_IDLE,
	NN_RX_WAITING,
	NN_RX_HEADER,
	NN_RX_PAYLOAD,
	NN_RX_CRC,
	NN_RX_REPLY,
	NN_TX,
} NanoNetState;

enum NanoNetOptions {
	OptRack = 0x40,
};

class NanoNet {
	protected:
		byte address;
		byte clock_pin;
		byte data_pin;
		byte status_pin = 13;
		NanoNetState state = NN_IDLE;
		Crc16 crc;
	public:
		short tx_rate = _TX_RATE;
		short ca_rate = 4;
		short cd_rate = 4;
		void init(byte _address, byte _clock_pin, byte _data_pin) {
			address = _address;
			clock_pin = _clock_pin;
			data_pin = _data_pin;
			pinMode(clock_pin, INPUT);
			pinMode(data_pin, INPUT);
			pinMode(status_pin, OUTPUT);
		}
		byte sendFrame(char *payload, byte destination, byte options);
		byte recieveFrame(char *buf);
};

#endif
