/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet Rewrite
 * @created     : Friday May 07, 2021 18:17:22 EDT
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

#include "Crc16.h"

union twobytes {
	volatile short s;
	volatile byte b[2];
};

volatile twobytes NN_rx_buf;
volatile bool NN_new_bit;

typedef enum State {
	NN_IDLE,
	NN_RX_WAITING,
	NN_RX_HEADER,
	NN_RX_PAYLOAD,
	NN_RX_CRC,
	NN_RX_REPLY,
	NN_TX, //I dont think i need this one but it makes sense aesthetically
} NanoNetState;

class NanoNet {
	private:
		short _tx_rate = _TX_RATE; // Clock rate in bits per second
		// How many times during one clock cycle to check for busy line...
		short _ca_rate = 4; //...before transmitting (collision avoidance)
		short _cd_rate = 4; //...while transmitting (collision detection)
		byte _status_pin = 13;
		byte _address;
		NanoNetState _state = NN_IDLE;
		bool _send_byte(byte tx_byte);
		Crc16 crc;
		void init(byte address, byte clock_pin, byte data_pin, byte status_pin) {
			_address = address;
			_clock_pin = clock_pin;
			_data_pin = data_pin;
			_status_pin = status_pin;
			pinMode(_clock_pin, INPUT);
			pinMode(_data_pin, INPUT);
			pinMode(_status_pin, OUTPUT);
		}
	public:
		byte _clock_pin;
		byte _data_pin;
		inline NanoNet(byte address, byte clock_pin, byte data_pin) {
			init(address, clock_pin, data_pin, 13);
		}
		inline NanoNet(byte address, byte clock_pin, byte data_pin, byte status_pin) {
			init(address, clock_pin, data_pin, status_pin);
		}
		void handle_interrupt() {
			bool rx_bit = digitalRead(_data_pin);
			NN_rx_buf.s = (NN_rx_buf.s << 1) | rx_bit;
			NN_new_bit = true;
		}
		byte sendFrame(char* payload, byte destination, byte options);
		byte recieveFrame(char *buf);
};

#define FULL_TICK (1000000 / _tx_rate)
#define HALF_TICK (500000 / _tx_rate)
#define DELAY(us) if(us <= 16000) { delayMicroseconds(us); } else { delay(us / 1000); }

#define _NN_SEND_BYTE(TX_BYTE) 			\
	if(digitalRead(_OPT_PIN_3) == LOW) {\
		return -1; 						\
	} 									\
	if(!_send_byte(TX_BYTE)) { 			\
		pinMode(_clock_pin, INPUT); 	\
		digitalWrite(_clock_pin, LOW); 	\
		pinMode(_data_pin, INPUT); 		\
		digitalWrite(_data_pin, LOW); 	\
		digitalWrite(_status_pin, 0); 	\
		return 1; 						\
	}

bool NanoNet::_send_byte(byte tx_byte) {
	_LOG_BYTE(tx_byte);
	for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
		bool tx_bit = tx_byte & (0x80 >> bit_idx);

		// Set data pin, ensure clock pin is unset,,
		digitalWrite(_data_pin, tx_bit);

		// wait half a tick (check for collisions _cd_rate times),,
		for (int cd_idx = 0; cd_idx < _cd_rate; cd_idx++) {
			DELAY(HALF_TICK / _cd_rate);
			// The clock pin should be low
			// if it's high then somebody else is transmitting which is bad
			if (digitalRead(_clock_pin)) {
				LOG_ERROR(F("Collision detected, aborting send"));
				return false;
			}
		}
		// finally turn the clock signal on,,
		digitalWrite(_clock_pin, HIGH);
		DELAY(HALF_TICK);
		digitalWrite(_clock_pin, LOW);
	}
	return true;
}

#define OPT_TITY 128
#define OPT_RACK 64
#define OPT_BSOM 2

/* return values:
 * -1 - Cancelled
 *  0 - Successful transmit
 *  1 - Bus in use (collision avoided)
 *  2 - Could not reach a destination
 */
byte NanoNet::sendFrame(char* payload, byte destination, byte options) {
	_state = NN_TX;
	bool inject_error = true;
	byte try_count = 0;
	LOG_INFO(F("Checking for busy bus"));
	digitalWrite(_clock_pin, LOW);
	digitalWrite(_data_pin, LOW);
	// Make sure the bus isnt in use
	for (int ca_idx = 0; ca_idx < _ca_rate; ca_idx++) {
		DELAY(FULL_TICK / _ca_rate);
		// If the clock pin goes high at any point, 
		if (digitalRead(_clock_pin)) {
			LOG_INFO(F("Signal found, abort"));
			return 1;
		}
	}
	LOG_INFO(F("No signal detected on bus"));
	digitalWrite(_status_pin, HIGH);
	pinMode(_clock_pin, OUTPUT);
	pinMode(_data_pin, OUTPUT);

BEGIN_PREAMBLE:
	LOG_INFO(F("Sending preamble"));
	_NN_SEND_BYTE(0xFF);
	_NN_SEND_BYTE(0x01); // Start of Header (SOH)
	_LOG_BYTE_SEP();

	crc.clearCrc();

	LOG_INFO(F("Sending header"));
	crc.clearCrc();
	_NN_SEND_BYTE(destination);
	_NN_SEND_BYTE(_address);
	_NN_SEND_BYTE(strlen(payload));
	_NN_SEND_BYTE(options);
	crc.updateCrc(destination);
	crc.updateCrc(_address);
	crc.updateCrc(strlen(payload));
	crc.updateCrc(options);
	_LOG_BYTE_SEP();

	LOG_INFO(F("Sending payload"));
	_NN_SEND_BYTE(0x02); // Start of Text (STX)
	for(int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
		char tx_byte = payload[byte_idx];
		_NN_SEND_BYTE(tx_byte);
#define INTENTIONAL_ERROR
#ifdef INTENTIONAL_ERROR
		if(inject_error && byte_idx == 10) { inject_error = false; continue; }
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
	if(options >> 6 & 1 == 1) {
		_LOG_BYTE_SEP();
		short old_tx_rate = _tx_rate;
		_tx_rate = 20;
		pinMode(_data_pin, INPUT);
		digitalWrite(_data_pin, LOW);
		_LOG_INFO(F("Receiving Reply Byte"));
		_LOG_TRACE(F(": "));
		byte reply_byte = 0;
		for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
			DELAY(HALF_TICK);
			digitalWrite(_clock_pin, HIGH);
			bool rx_bit = digitalRead(_data_pin);
			reply_byte = (reply_byte << 1) | rx_bit;
			_LOG_BIT(rx_bit);
			DELAY(HALF_TICK);
			digitalWrite(_clock_pin, LOW);
		}
		_tx_rate = old_tx_rate;
		pinMode(_data_pin, OUTPUT);
		_LOG_TRACE(F("\rReceiving Reply Byte"));
		if(reply_byte == 0x06) {
			LOG_INFO(F(": ACK     "));
		} else if(reply_byte == 0x15) {
			LOG_INFO(F(": NACK    "));
			LOG_INFO(F("Restarting send"));
			goto BEGIN_PREAMBLE;
		} else if(reply_byte == 0x00) {
			LOG_INFO(F(": NULL      "));
			try_count++;
			if(try_count > 5) {
				LOG_FATAL(F("Could not reach a reciever in 5 tries"));
				pinMode(_clock_pin, INPUT);
				pinMode(_data_pin, INPUT);
				digitalWrite(_clock_pin, LOW);
				digitalWrite(_data_pin, LOW);
				digitalWrite(_status_pin, LOW);
				_state = NN_IDLE;
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
	pinMode(_clock_pin, INPUT);
	pinMode(_data_pin, INPUT);
	digitalWrite(_clock_pin, LOW);
	digitalWrite(_data_pin, LOW);
	digitalWrite(_status_pin, LOW);
	_state = NN_IDLE;
	return 0;
}

/* return values:
 * -1 - Cancelled
 *  0 - Bad data
 *  * - source address
 */
byte NanoNet::recieveFrame(char *buf) {
	int buf_pos = 0;
	int bit_pos = 0;
	twobytes rx_crc;
	twobytes comp_crc;
BEGIN_RECIEVE_FRAME:
	_state = NN_RX_WAITING;
	byte destination = 0;
	byte source = 0;
	byte length = 0;
	byte options = 0;
	byte reply = 0;
	pinMode(_clock_pin, INPUT);
	digitalWrite(_clock_pin, LOW);
	pinMode(_data_pin, INPUT);
	digitalWrite(_data_pin, LOW);
	crc.clearCrc();
	while(true) {
		if(digitalRead(_OPT_PIN_3) == LOW) {
			return -1;
		}
		if(NN_new_bit == true) {
			NN_new_bit = false;
			switch(_state) {
			case NN_RX_WAITING:
				if(NN_rx_buf.s == 0xFF01) {
					_LOG_INFO(F("Entered Frame"));
					_LOG_TRACE(F(": "))
					_LOG_BYTE(NN_rx_buf.b[1]);
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					_state = NN_RX_HEADER;
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
					if(destination != _address) {
						LOG_INFO(F("Message not for us, resetting"));
						_state = NN_RX_WAITING;
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
					if(options >> 6 & 1 == 1) {
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
						_state = NN_RX_PAYLOAD;
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
						_state = NN_RX_CRC;
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
					if(options >> 6 & 1 == 1) {
						if(comp_crc.s != rx_crc.s) {
							LOG_INFO(F("Bad data, Sending NACK"));
							reply = 0x15;
						} else {
							LOG_INFO(F("Success, Sending ACK"));
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
				_state = NN_RX_REPLY;
				bit_pos = 0;
				bool last_clock = 1;
				bool reply_bit;
				pinMode(_data_pin, OUTPUT);
				while(bit_pos < 9) {
					if(digitalRead(_clock_pin) != last_clock) {
						last_clock = !last_clock;
						if(last_clock == LOW) {
							reply_bit = reply >> 7-bit_pos & 1;
							digitalWrite(_data_pin, reply_bit);
							bit_pos++;
						}
					}
				}
				pinMode(_data_pin, INPUT);
				digitalWrite(_data_pin, LOW);
				if(reply == 0x15) {
					LOG_TRACE(F("Restarting Receive"));
					goto BEGIN_RECIEVE_FRAME;
				} else {
					LOG_TRACE(F("Finished Sending"));
					_state = NN_IDLE;
					pinMode(_data_pin, INPUT);
					digitalWrite(_data_pin, LOW);
					return source;
				}
			}
		}
	}
}
