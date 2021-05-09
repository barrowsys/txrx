/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet Rewrite
 * @created     : Friday May 07, 2021 18:17:22 EDT
 *
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
	NN_TX, //I dont think i need this one but it makes sense aesthetically
} NanoNetState;

class NanoNet {
	private:
		short _tx_rate = 20; // Clock rate in bits per second
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
		byte sendFrame(char* payload, byte addr);
		byte recieveFrame(char *buf);
};

#define FULL_TICK (1000 / _tx_rate)
#define HALF_TICK (500 / _tx_rate)

#define _NN_SEND_BYTE(TX_BYTE) 			\
	if(!_send_byte(TX_BYTE)) { 			\
		pinMode(_clock_pin, INPUT); 	\
		digitalWrite(_clock_pin, LOW); 	\
		pinMode(_data_pin, INPUT); 		\
		digitalWrite(_data_pin, LOW); 	\
		digitalWrite(_status_pin, 0); 	\
		return false; 					\
	}

bool NanoNet::_send_byte(byte tx_byte) {
	_LOG_BYTE(tx_byte);
	for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
		bool tx_bit = tx_byte & (0x80 >> bit_idx);

		// Set data pin, ensure clock pin is unset,,
		digitalWrite(_data_pin, tx_bit);

		// wait half a tick (check for collisions _cd_rate times),,
		for (int cd_idx = 0; cd_idx < _cd_rate; cd_idx++) {
			delay(HALF_TICK / _cd_rate);
			// The clock pin should be low
			// if it's high then somebody else is transmitting which is bad
			if (digitalRead(_clock_pin)) {
				LOG_ERROR("Collision detected, aborting send");
				return false;
			}
		}
		// finally turn the clock signal on,,
		digitalWrite(_clock_pin, HIGH);
		delay(HALF_TICK);
		digitalWrite(_clock_pin, LOW);
	}
	return true;
}

/* return values:
 *  0 - Successful transmit
 *  1 - Bus in use (collision avoided)
 */
byte NanoNet::sendFrame(char* payload, byte destination) {
	_state = NN_TX;
	LOG_INFO("Checking for busy bus");
	crc.clearCrc();
	digitalWrite(_clock_pin, LOW);
	digitalWrite(_data_pin, LOW);
	// Make sure the bus isnt in use
	for (int ca_idx = 0; ca_idx < _ca_rate; ca_idx++) {
		delay(FULL_TICK / _ca_rate);
		// If the clock pin goes high at any point, 
		if (digitalRead(_clock_pin)) {
			LOG_INFO("Signal found, abort");
			return 1;
		}
	}
	LOG_INFO("No signal detected on bus");
	digitalWrite(_status_pin, HIGH);
	pinMode(_clock_pin, OUTPUT);
	pinMode(_data_pin, OUTPUT);

	_NN_SEND_BYTE(0x00);
	_NN_SEND_BYTE(0x00);
	_LOG_BYTE_SEP();
	LOG_INFO("Sending preamble");
	_NN_SEND_BYTE(0xFF);
	_NN_SEND_BYTE(0x01); // Start of Header (SOH)
	_LOG_BYTE_SEP();

	LOG_INFO("Sending header");
	crc.clearCrc();
	_NN_SEND_BYTE(destination);
	_NN_SEND_BYTE(_address);
	crc.updateCrc(destination);
	crc.updateCrc(_address);
	_LOG_BYTE_SEP();

	LOG_INFO("Sending payload");
	_NN_SEND_BYTE(0x02); // Start of Text (STX)
	for(int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
		char tx_byte = payload[byte_idx];
		_NN_SEND_BYTE(tx_byte);
		crc.updateCrc(tx_byte);
	}
	_LOG_BYTE_SEP();

	LOG_INFO("Sending trailer");
	_NN_SEND_BYTE(0x03); // End of Text (ETX)
	twobytes crc_val;
	crc_val.s = crc.getCrc();
	_NN_SEND_BYTE(crc_val.b[0]);
	_NN_SEND_BYTE(crc_val.b[1]);
	_NN_SEND_BYTE(0x04); // End of Transmission (EOT)
	_LOG_BYTE_SEP();

	LOG_INFO("Cleaning up");
	pinMode(_clock_pin, INPUT);
	pinMode(_data_pin, INPUT);
	digitalWrite(_clock_pin, LOW);
	digitalWrite(_data_pin, LOW);
	digitalWrite(_status_pin, LOW);
	_state = NN_IDLE;
	return 0;
}

/* return values:
 *  0 - Bad data
 * !0 -> source address
 */
byte NanoNet::recieveFrame(char *buf) {
	int buf_pos = 0;
	int bit_pos = 0;
	twobytes rx_crc;
	_state = NN_RX_WAITING;
	byte destination = 0;
	byte source = 0;
	digitalWrite(_clock_pin, LOW);
	pinMode(_clock_pin, INPUT);
	digitalWrite(_clock_pin, LOW);
	digitalWrite(_data_pin, LOW);
	pinMode(_data_pin, INPUT);
	digitalWrite(_data_pin, LOW);
	crc.clearCrc();
	while(true) {
		if(NN_new_bit == true) {
			NN_new_bit = false;
			switch(_state) {
			case NN_RX_WAITING:
				if(NN_rx_buf.s == 0xFF01) {
					/* if(DEBUG_LEVEL >= 3) { */
					/* 	Serial.print("Entered Frame"); */
					/* } */
					/* if(DEBUG_LEVEL >= 4) { */
					/* 	Serial.print(": "); */
					/* } */
					/* if(DEBUG_LEVEL >= 4) { */
					/* 	if(NN_rx_buf.b[1] < 0x10) { */
					/* 		Serial.print("0"); */
					/* 	} */
					/* 	Serial.print(NN_rx_buf.b[1], HEX); */
					/* 	Serial.print(" "); */
					/* } */
					/* if(DEBUG_LEVEL >= 4) { */
					/* 	if(NN_rx_buf.b[0] < 0x10) { */
					/* 		Serial.print("0"); */
					/* 	} */
					/* 	Serial.print(NN_rx_buf.b[0], HEX); */
					/* 	Serial.print(" "); */
					/* } */
					/* if(DEBUG_LEVEL >= 3) { */
					/* 	Serial.println(); */
					/* } */
					_LOG_INFO("Entered Frame");
					_LOG_TRACE(": ")
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
					_LOG_INFO("Got Destination Address");
					_LOG_TRACE(": ");
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					destination = NN_rx_buf.b[0];
					if(destination != _address) {
						LOG_INFO("Message not for us, resetting");
						_state = NN_RX_WAITING;
						bit_pos = 0;
					} else {
						crc.updateCrc(destination);
					}
				} else if(bit_pos == 16) {
					_LOG_INFO("Got Source Address");
					_LOG_TRACE(": ");
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_INFO();
					source = NN_rx_buf.b[0];
					crc.updateCrc(source);
				} else if(bit_pos > 16 && bit_pos % 8 == 0) {
					if(NN_rx_buf.b[0] != 0x02) {
						_LOG_INFO("Recieved unknown header byte");
						_LOG_TRACE(": ");
						_LOG_BYTE(NN_rx_buf.b[0]);
						LOG_INFO();
						// Add to checksum to ensure forwards-compatibility
						crc.updateCrc(NN_rx_buf.b[0]);
					} else {
						LOG_INFO("End of Header");
						_state = NN_RX_PAYLOAD;
						bit_pos = 0;
						_LOG_INFO("Recieving payload");
					}
				}
				break;
			case NN_RX_PAYLOAD:
				bit_pos = (bit_pos + 1) % 8;
				if(bit_pos == 0) {
					if(NN_rx_buf.b[0] != 0x03) {
						_LOG_INFO(".");
						buf[buf_pos++] = NN_rx_buf.b[0];
						crc.updateCrc(NN_rx_buf.b[0]);
					} else {
						LOG_INFO();
						LOG_INFO("End of Text");
						buf[buf_pos] = 0x00;
						buf_pos = 0;
						bit_pos = 0;
						_state = NN_RX_CRC;
						LOG_INFO("Receiving CRC");
					}
				}
				break;
			case NN_RX_CRC:
				bit_pos++;
				if(bit_pos == 8) {
					/* _LOG_INFO("Got First CRC Byte"); */
					_LOG_BYTE(NN_rx_buf.b[0]);
					rx_crc.b[0] = NN_rx_buf.b[0];
				} else if(bit_pos == 16) {
					/* _LOG_INFO("Got Second CRC Byte"); */
					_LOG_BYTE(NN_rx_buf.b[0]);
					LOG_TRACE();
					rx_crc.b[1] = NN_rx_buf.b[0];
				} else if(bit_pos % 8 == 0 && NN_rx_buf.b[0] == 0x04) {
					LOG_INFO("End of Transmission");
					twobytes comp_crc;
					comp_crc.s = crc.getCrc();
					_LOG_TRACE("Computed CRC: ");
					_LOG_BYTE(comp_crc.b[0]);
					_LOG_BYTE(comp_crc.b[1]);
					LOG_TRACE();
					if(comp_crc.s != rx_crc.s) {
						LOG_ERROR("Bad data");
						return 0;
					} else {
						LOG_INFO("Success");
						return source;
					}
				}
				break;
			}
		}
	}
}
