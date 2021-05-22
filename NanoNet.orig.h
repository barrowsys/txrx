/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file	: Original NanoNet Impl as a subclass
 * @created     : Friday May 21, 2021 17:33:56 EDT
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


#include "Definitions.h"
#include "NanoNet.h"

volatile twobytes _NN_rx_buf;
volatile bool NN_new_bit;

/* this implementation uses an interrupt on the clock pin.
 * this makes it much more reliable at high speed but also 
 * restricts you to pins 2 and 3 for clock.
 */
class NanoNetOrig: public NanoNet {
	private:
		bool send_byte(byte tx_byte); 
	protected:
		virtual bool recieve_bit() = 0;
	public:
		static void isr();
		void init(byte _address, byte _clock_pin, byte _data_pin);
};

void NanoNetOrig::init(byte _address, byte _clock_pin, byte _data_pin) {
	NanoNet::init(_address, _clock_pin, _data_pin);
	attachInterrupt(digitalPinToInterrupt(clock_pin), NanoNetOrig::isr, RISING);
}

void NanoNetOrig::isr() {
	if(digitalRead(_CLOCK_PIN) != LOW) {
		bool rx_bit = digitalRead(_DATA_PIN) == HIGH;
		_NN_rx_buf.s = _NN_rx_buf.s << 1;
		if(rx_bit) {
			_NN_rx_buf.s |= 0x01;
		}
		NN_new_bit = true;
	}
}

bool NanoNetOrig::recieve_bit() {
	if(NN_new_bit == true) {
		NN_new_bit = false;
		NanoNetOrig::NN_rx_buf.s = _NN_rx_buf.s;
		return true;
	} else {
		return false;
	}
}
