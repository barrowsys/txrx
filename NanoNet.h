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
		twobytes NN_rx_buf;
		byte send_byte(byte tx_byte); 
		void reset_send();
		virtual bool recieve_bit() = 0;
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

void NanoNet::reset_send() {
	pinMode(clock_pin, INPUT);
	digitalWrite(clock_pin, LOW);
	pinMode(data_pin, INPUT);
	digitalWrite(data_pin, LOW);
	digitalWrite(status_pin, LOW);
	state = NN_IDLE;
}

byte NanoNet::send_byte(byte tx_byte) {
	if(should_cancel()) {
		return -1;
	}
	_LOG_BYTE(tx_byte);
	for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
		bool tx_bit = tx_byte & (0x80 >> bit_idx);

		digitalWrite(data_pin, tx_bit);
		for(int cd_idx = 0; cd_idx < cd_rate; cd_idx++) {
			DELAY(HALF_TICK / cd_rate);
			if(digitalRead(clock_pin)) {
				LOG_ERROR(F("Collision detected, aborting send"));
				return 1;
			}
		}
		digitalWrite(clock_pin, HIGH);
		DELAY(HALF_TICK);
		digitalWrite(clock_pin, LOW);
	}
	return 0;
}

#define _NN_SEND_BYTE(TX_BYTE)\
	r = send_byte(TX_BYTE);\
	if(r != 0) {\
		reset_send();\
		return r;\
	}\

#ifndef INTENTIONAL_ERROR
#define INTENTIONAL_ERROR false
#endif

byte NanoNet::sendFrame(char *payload, byte destination, byte options) {
	state = NN_TX;
#if INTENTIONAL_ERROR
	bool inject_error = true;
#endif
	byte r = 0;
	uint8_t try_count = 0;
	LOG_INFO(F("Checking for busy bus"));
	digitalWrite(clock_pin, LOW);
	digitalWrite(data_pin, LOW);
	// Make sure the bus isnt in use
	for (int ca_idx = 0; ca_idx < ca_rate*2; ca_idx++) {
		DELAY(FULL_TICK / ca_rate);
		// If the clock pin goes high at any point, 
		if (digitalRead(clock_pin)) {
			LOG_INFO(F("Signal found, abort"));
			return 1;
		}
	}
	LOG_INFO(F("No signal detected on bus"));
	digitalWrite(status_pin, HIGH);
	pinMode(clock_pin, OUTPUT);
	pinMode(data_pin, OUTPUT);

BEGIN_PREAMBLE:
	_NN_SEND_BYTE(0);
	LOG_INFO(F("Sending preamble"));
	_NN_SEND_BYTE(0xFF);
	_NN_SEND_BYTE(0x01); // Start of Header (SOH)
	_LOG_BYTE_SEP();

	crc.clearCrc();

	LOG_INFO(F("Sending header"));
	crc.clearCrc();
	_NN_SEND_BYTE(destination);
	_NN_SEND_BYTE(address);
	_NN_SEND_BYTE(strlen(payload));
	_NN_SEND_BYTE(options);
	crc.updateCrc(destination);
	crc.updateCrc(address);
	crc.updateCrc(strlen(payload));
	crc.updateCrc(options);
	_LOG_BYTE_SEP();

	LOG_INFO(F("Sending payload"));
	_NN_SEND_BYTE(0x02); // Start of Text (STX)
	for(int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
		char tx_byte = payload[byte_idx];
		_NN_SEND_BYTE(tx_byte);
#if INTENTIONAL_ERROR
		if(inject_error && byte_idx == 10) {
			inject_error = false;
			continue;
		}
#endif
		crc.updateCrc(tx_byte);
}
	_LOG_BYTE_SEP();

	LOG_INFO(F("Sending trailer"));
	_NN_SEND_BYTE(0x03); // End of Text (ETX)
	twobytes crc_val;
	crc_val.s = crc.getCrc();
	_NN_SEND_BYTE(crc_val.b[0]);
	_NN_SEND_BYTE(crc_val.b[1]);
	if(options & OptRack) {
		_LOG_BYTE_SEP();
		pinMode(data_pin, INPUT);
		digitalWrite(data_pin, LOW);
		_LOG_INFO(F("Receiving Reply Byte"));
		_LOG_TRACE(F(": "));
		byte reply_byte = 0;
		DELAY(FULL_TICK*2);
		for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
			DELAY(HALF_TICK);
			digitalWrite(clock_pin, HIGH);
			bool rx_bit = digitalRead(data_pin);
			reply_byte = (reply_byte << 1) | rx_bit;
			_LOG_BIT(rx_bit);
			DELAY(HALF_TICK);
			digitalWrite(clock_pin, LOW);
		}
		pinMode(data_pin, OUTPUT);
		_LOG_TRACE(F("\rReceiving Reply Byte"));
		if(reply_byte == 0x06) {
			LOG_INFO(F(": ACK     "));
		} else if(reply_byte == 0x15) {
			LOG_INFO(F(": NACK    "));
			LOG_INFO(F("Restarting send"));
			Serial.println(F("GOT A NACK"));
			failures++;
			goto BEGIN_PREAMBLE;
		} else if(reply_byte == 0x00) {
			LOG_INFO(F(": NULL      "));
			Serial.println(F("GOT A NULL"));
			failures++;
			try_count++;
			if(try_count > 5) {
				LOG_FATAL(F("Could not reach a reciever in 5 tries"));
				reset_send();
				return 2;
			}
			IFERROR(F("Could not reach a receiver"));
			IFINFO(printbln(F("Restarting send in "), try_count, F(" seconds")));
			delay(1000 * try_count);
			LOG_INFO();
			goto BEGIN_PREAMBLE;
		}
	}
	_NN_SEND_BYTE(0x04); // End of Transmission (EOT)
	_LOG_BYTE_SEP();

	LOG_INFO(F("Cleaning up"));
	reset_send();
	return 0;
}

byte NanoNet::recieveFrame(char *buf) {
	int buf_pos = 0;
	int bit_pos = 0;
	twobytes rx_crc;
	twobytes comp_crc;
BEGIN_RECIEVE_FRAME:
	state = NN_RX_WAITING;
	byte destination = 0;
	byte source = 0;
	byte length = 0;
	byte options = 0;
	byte reply = 0;
	pinMode(clock_pin, INPUT);
	digitalWrite(clock_pin, LOW);
	pinMode(data_pin, INPUT);
	digitalWrite(data_pin, LOW);
	crc.clearCrc();
	NN_rx_buf.s = 0;
	while(!should_cancel()) {
		if(recieve_bit() == true) {
			switch(state) {
			case NN_RX_WAITING:
				if(NN_rx_buf.s == 0xFF01) {
					_LOG_WARN(F("Entered Frame"));
					_LOG_TRACE(F(": "))
					_LOG_BYTE(NN_rx_buf.b[1]);
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_WARN();
					state = NN_RX_HEADER;
					bit_pos = 0;
					buf_pos = 0;
				}
				break;
			case NN_RX_HEADER:
				bit_pos++;
				if(bit_pos == 8) {
					_LOG_INFO(F("Got Destination Address"));
					_LOG_TRACE(F(": "));
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					destination = NN_rx_buf.b[0];
					if(destination != address) {
						LOG_INFO(F("Message not for us, resetting"));
						state = NN_RX_WAITING;
						bit_pos = 0;
					} else {
						crc.updateCrc(NN_rx_buf.b[0]);
					}
				} else if(bit_pos == 16) {
					_LOG_INFO(F("Got Source Address"));
					_LOG_TRACE(F(": "));
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					source = NN_rx_buf.b[0];
					crc.updateCrc(NN_rx_buf.b[0]);
				} else if(bit_pos == 24) {
					_LOG_INFO(F("Recieved Length Byte"));
					_LOG_TRACE(F(": "));
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					length = NN_rx_buf.b[0];
					crc.updateCrc(NN_rx_buf.b[0]);
				} else if(bit_pos == 32) {
					_LOG_INFO(F("Recieved Options Byte"));
					_LOG_TRACE(F(": "));
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					options = NN_rx_buf.b[0];
					crc.updateCrc(NN_rx_buf.b[0]);
					if(options & OptRack) {
						LOG_INFO(F("RACK bit set"));
					} else {
						LOG_INFO(F("RACK bit unset"));
					}
				} else if(bit_pos > 32 && bit_pos % 8 == 0) {
					if(NN_rx_buf.b[0] != 0x02) {
						_LOG_INFO(F("Recieved unknown header byte"));
						_LOG_TRACE(F(": "));
						_LOG_BYTE(NN_rx_buf.b[0]);
						LOG_INFO();
						// Add to checksum to ensure forwards-compatibility
						crc.updateCrc(NN_rx_buf.b[0]);
					} else {
						LOG_INFO(F("End of Header"));
						state = NN_RX_PAYLOAD;
						bit_pos = 0;
						_LOG_INFO(F("Recieving payload"));
					}
				}
				break;
			case NN_RX_PAYLOAD:
				bit_pos = (bit_pos + 1) % 8;
				if(bit_pos == 0) {
					if(buf_pos > length) {
						LOG_INFO(F("Longer than length, cancelling"));
						goto BEGIN_RECIEVE_FRAME;
					} else if(NN_rx_buf.b[0] != 0x03) {
						_LOG_INFO(F("."));
						buf[buf_pos++] = NN_rx_buf.b[0];
						crc.updateCrc(NN_rx_buf.b[0]);
					} else {
						LOG_INFO();
						LOG_INFO(F("End of Text"));
						buf[buf_pos] = 0x00;
						buf_pos = 0;
						bit_pos = 0;
						state = NN_RX_CRC;
						_LOG_INFO(F("Receiving CRC"));
						_LOG_TRACE(F(": "));
					}
				}
				break;
			case NN_RX_CRC:
				bit_pos++;
				if(bit_pos == 8) {
					/* _LOG_INFO(F("Got First CRC Byte")); */
					_LOG_BYTE(NN_rx_buf.b[0]);
					rx_crc.b[0] = NN_rx_buf.b[0];
				} else if(bit_pos == 16) {
					/* _LOG_INFO(F("Got Second CRC Byte")); */
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					comp_crc.s = crc.getCrc();
					_LOG_TRACE(F("Computed CRC: "));
					_LOG_BYTE(comp_crc.b[0]);
					_LOG_BYTE(comp_crc.b[1]);
					LOG_TRACE();
					rx_crc.b[1] = NN_rx_buf.b[0];
					if(options & OptRack) {
						if(comp_crc.s != rx_crc.s) {
							LOG_WARN(F("Bad data, Sending NACK"));
							reply = 0x15;
						} else {
							LOG_WARN(F("Success, Sending ACK"));
							reply = 0x06;
						}
						goto BEGIN_NN_RX_REPLY;
					}
				} else if(bit_pos % 8 == 0 && NN_rx_buf.b[0] == 0x04) {
					LOG_INFO(F("End of Transmission"));
					if(comp_crc.s != rx_crc.s) {
						LOG_FATAL(F("Bad data"));
						return 0;
					} else {
						LOG_INFO(F("Success"));
						return source;
					}
				}
				break;
BEGIN_NN_RX_REPLY:
			case NN_RX_REPLY:
				state = NN_RX_REPLY;
				bit_pos = 0;
				bool last_clock = 1;
				bool reply_bit;
				pinMode(data_pin, OUTPUT);
				while(bit_pos < 9) {
					if(digitalRead(clock_pin) != last_clock) {
						last_clock = !last_clock;
						if(last_clock == LOW) {
							reply_bit = reply >> 7-bit_pos & 1;
							digitalWrite(data_pin, reply_bit);
							bit_pos++;
						}
					}
				}
				pinMode(data_pin, INPUT);
				digitalWrite(data_pin, LOW);
				if(reply == 0x15) {
					LOG_TRACE(F("Restarting Receive"));
					goto BEGIN_RECIEVE_FRAME;
				} else {
					LOG_TRACE(F("Finished Sending"));
					state = NN_IDLE;
					pinMode(data_pin, INPUT);
					digitalWrite(data_pin, LOW);
					return source;
				}
			}
		}
	}
	return -1;
}

#endif
