/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet.noir
 * @created     : Friday May 21, 2021 20:22:25 EDT
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

/* this implementation does NOT use interrupts. any pin can be used for clock
 * hypothetically this would have more errors at high line speeds but it seems to handle it.
 */
class NanoNetNoir: public NanoNet {
	private:
		bool last_clock;
	protected:
		virtual bool recieve_bit();
	public:
		void init(byte _address, byte _clock_pin, byte _data_pin);
};

void NanoNetNoir::init(byte _address, byte _clock_pin, byte _data_pin) {
	NanoNet::init(_address, _clock_pin, _data_pin);
}

bool NanoNetNoir::recieve_bit() {
	bool clock = digitalRead(clock_pin);
	if(clock != last_clock) {
		last_clock = clock;
		if(clock == HIGH) {
			bool rx_bit = digitalRead(_DATA_PIN) == HIGH;
			NN_rx_buf.s = NN_rx_buf.s << 1;
			if(rx_bit) {
				NN_rx_buf.s |= 0x01;
			}
			return true;
		}
	}
	return false;
}
