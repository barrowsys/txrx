/**
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
#ifndef NANONET_H
#define NANONET_H

#include "Crc16.h"

// Ideally this would be set when the NanoNet object is constructed,
// but you can only use static functions as interrupt routines,
// so i have to have this, NN_rx_buf, NN_new_bit, and _NN_onClockPulse all outside the class.
//
// its really bad but it works
#ifndef NN_clockPin
#define NN_clockPin 2
#endif
#ifndef NN_dataPin
#define NN_dataPin 3
#endif

// Enables Trace Logging To Serial
/* #define DEBUG */

// See http://www.asciitable.com/
#define NN_SOH 0x01
#define NN_STX 0x02
#define NN_ETX 0x03
#define NN_EOT 0x04

// This is definitely the wrong way to treat a 16 bit short as two 8 bit values
union twobytes {
  short s;
  byte b[2];
};

typedef enum State {
  NN_IDLE,
  NN_RX_WAITING,
  NN_RX_HEADER,
  NN_RX_PAYLOAD,
  NN_RX_CRC,
  NN_TX, //I dont think i need this one but it makes sense aesthetically
} NanoNetState;

Crc16 NN_crc;

// the last 16 bits recieved from the bus
volatile twobytes NN_rx_buf;
// flag to notify the instance functions that data has arrived
volatile bool NN_new_bit;
// This is really bad and evil but it works as long as nobody tries to make two NanoNet instances
// Necessary because attachInterrupt needs a static function handler
void _NN_onClockPulse() {
  bool rx_bit = digitalRead(NN_dataPin);

  // Shift the buffer left 1 bit and set the new lowest bit
  NN_rx_buf.s = NN_rx_buf.s << 1;
  if (rx_bit) {
    NN_rx_buf.s |= 0x01;
  }

  NN_new_bit = true;
}

class NanoNet {
  private:
    byte _txRate = 10; // In bits per second
    // How many divisions of a clock cycle to check for other transmitters...
    byte _caRate = 4; //...before transmitting
    byte _cdRate = 4; //...while transmitting
    NanoNetState _state = NN_IDLE;
    bool _sendByte(byte tx_byte);
  public:
    byte _address;
    inline NanoNet(byte address) {
      _address = address;
      pinMode(NN_clockPin, INPUT);
      pinMode(NN_dataPin, INPUT);
      // TODO: dont do this
      attachInterrupt(digitalPinToInterrupt(NN_clockPin), _NN_onClockPulse, RISING);
    }
    bool sendFrame(char* payload, byte addr);
    byte recieveFrame(char *buf);
};

// convienience is the mother of invention
// ...
// in this case the invention is exceptions but those are evil
// tl;dr this lets me do if(the byte doesnt send) {return false} without having to write that out 30 times
// never used the c preprocessor beyond basic object defines until this so thats cool
#define _NN_SEND_BYTE(TX_BYTE) if(!_sendByte(TX_BYTE)) { digitalWrite(NN_clockPin, LOW); digitalWrite(NN_dataPin, LOW); pinMode(NN_clockPin, INPUT); pinMode(NN_dataPin, INPUT); digitalWrite(13, LOW); return false; }

bool NanoNet::_sendByte(byte tx_byte) {
  for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
    bool tx_bit = tx_byte & (0x80 >> bit_idx);
    // Write data to pin,, then wait half a tick, THEN turn the clock signal on
    digitalWrite(NN_dataPin, tx_bit);
    // While the clock is off, check that it really is off _cdRate times 
    for (int cd_idx = 0; cd_idx < _cdRate; cd_idx++) {
      delay((1000 / _txRate) / (_cdRate * 2));
      if (digitalRead(NN_clockPin)) {
#ifdef DEBUG
        Serial.println("Collision detected, abort");
#endif
        return false;
      }
    }
    digitalWrite(NN_clockPin, HIGH);
    delay((1000 / _txRate) / 2);
    digitalWrite(NN_clockPin, LOW);
  }
  return true;
}

bool NanoNet::sendFrame(char *payload, byte destination) {
  _state = NN_TX;
#ifdef DEBUG
  Serial.println("Checking for signal");
#endif
  // Make sure the bus isnt in use
  for (int ca_idx = 0; ca_idx < _caRate; ca_idx++) {
    delay((1000 / _txRate) / _caRate);
    if (digitalRead(NN_clockPin)) {
#ifdef DEBUG
      Serial.println("Signal found, abort");
#endif
      return false;
    }
  }
#ifdef DEBUG
  Serial.println("No signal found");
#endif
  digitalWrite(13, HIGH);
  pinMode(NN_clockPin, OUTPUT);
  pinMode(NN_dataPin, OUTPUT);
  digitalWrite(NN_clockPin, LOW);
  digitalWrite(NN_dataPin, LOW);
#ifdef DEBUG
  Serial.println("Sending start header");
#endif
  // our recieve buffer is 16 bits, and 0xFF shouldnt reaaaally be intentionally transmitted,
  // so we can use that in front of our start header byte to make a start transmission doublebyte
  _NN_SEND_BYTE(0xFF);
#ifdef DEBUG
  Serial.println("Sending header");
#endif
  NN_crc.clearCrc();
  _NN_SEND_BYTE(NN_SOH);
  _NN_SEND_BYTE(destination);
  NN_crc.updateCrc(destination);
  _NN_SEND_BYTE(_address);
  NN_crc.updateCrc(_address);
#ifdef DEBUG
  Serial.println("Sending payload");
#endif
  _NN_SEND_BYTE(NN_STX);
  for (int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
    char tx_byte = payload[byte_idx];
    _NN_SEND_BYTE(payload[byte_idx]);
    NN_crc.updateCrc(payload[byte_idx]);
  }
#ifdef DEBUG
  Serial.println("Sending CRC");
#endif
  _NN_SEND_BYTE(NN_ETX);
  twobytes value;
  value.s = NN_crc.getCrc();
  _NN_SEND_BYTE(value.b[0]);
  _NN_SEND_BYTE(value.b[1]);
  _NN_SEND_BYTE(NN_EOT);
  // make sure everything is back to floating
  // this is also done by the _NN_SEND_BYTE macro
  digitalWrite(NN_clockPin, LOW);
  digitalWrite(NN_dataPin, LOW);
  pinMode(NN_clockPin, INPUT);
  pinMode(NN_dataPin, INPUT);
  digitalWrite(13, LOW);
  _state = NN_IDLE;
  return true;
}

// This is execution coordinate hell so just bear with me
// "serial ifs considered harmful" -ezra june barrow 2020
byte NanoNet::recieveFrame(char *buf) {
  int buf_pos = 0;
  int bit_pos = 0;
  twobytes rx_crc;
  _state = NN_RX_WAITING;
  byte destination = 0;
  byte source = 0;
  while (true) {
    if (NN_new_bit == true) {
      if(_state == NN_RX_HEADER) {
      // Step 2
        bit_pos++;
        if(bit_pos == 8) {
        // Step 2.1
          if (NN_rx_buf.b[0] != _address) {
            _state = NN_RX_WAITING;
          } else {
            destination = NN_rx_buf.b[0];
            NN_crc.updateCrc(NN_rx_buf.b[0]);
#ifdef DEBUG
            Serial.println("Got Destination Byte");
#endif
          }
        } else if(bit_pos == 16) {
        // Step 2.2
          source = NN_rx_buf.b[0];
          NN_crc.updateCrc(NN_rx_buf.b[0]);
#ifdef DEBUG
          Serial.println("Got Source Byte");
#endif
        } else if(bit_pos % 8 == 0) {
          if(NN_rx_buf.b[0] == NN_STX) {
          // Step 2.4
#ifdef DEBUG
            Serial.println("End of Header");
#endif
            _state = NN_RX_PAYLOAD;
            bit_pos = 0;
          } else {
          // Step 2.3
            // This ensures backwards compatibility
            NN_crc.updateCrc(NN_rx_buf.b[0]);
          }
        }
      } else if (_state == NN_RX_PAYLOAD) {
      // Step 3
        if (++bit_pos == 8) {
          bit_pos = 0;
          if (NN_rx_buf.b[0] == NN_ETX) {
  		    // Step 3.2
#ifdef DEBUG
            Serial.println("Entered CRC");
#endif
            buf[buf_pos++] = 0x00;
            _state = NN_RX_CRC;
            buf_pos = 0;
          } else {
  	      // Step 3.1
            buf[buf_pos++] = NN_rx_buf.b[0];
            NN_crc.updateCrc(NN_rx_buf.b[0]);
          }
        }
      } else if (_state = NN_RX_CRC) {
    	// Step 4
        bit_pos++;
        if (bit_pos == 8) {
    	  // Step 4.1
          rx_crc.b[0] = NN_rx_buf.b[0];
#ifdef DEBUG
          Serial.println("Got First CRC Byte");
#endif
        } else if (bit_pos == 16) {
    	  // Step 4.2
          rx_crc.b[1] = NN_rx_buf.b[0];
#ifdef DEBUG
          Serial.println("Got Second CRC Byte");
#endif
        } else if (bit_pos == 24 && NN_rx_buf.b[0] == NN_EOT) {
    	  // Step 4.3
          if (NN_crc.getCrc() != rx_crc.s) {
#ifdef DEBUG
            Serial.println("Bad data");
#endif
            return 0;
          } else {
            return source;
          }
        }
      }
      if (NN_rx_buf.b[1] == 0xFF && NN_rx_buf.b[0] == NN_SOH) {
      // Step 1
        _state = NN_RX_HEADER;
        bit_pos = 0;
        buf_pos = 0;
        NN_crc.clearCrc();
#ifdef DEBUG
        Serial.println("Entered Frame");
#endif
      }
      NN_new_bit = false;
    }
  }
  _state = NN_IDLE;
}


#endif
