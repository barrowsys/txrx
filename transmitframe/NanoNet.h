#ifndef NANONET_H
#define NANONET_H

#include "Crc16.h"

#ifndef NN_clockPin
#define NN_clockPin 2
#endif
#ifndef NN_dataPin
#define NN_dataPin 3
#endif

// See http://www.asciitable.com/
#define NN_SOT 0xFF02 //Proceeded by eight 1s to ensure it would never show up otherwise
#define NN_SOH 0x02
#define NN_STX 0x03
#define NN_ETX 0x04
#define NN_EOT 0x05

// This is definitely the wrong way to treat a 16 bit short as two 8 bit values
union twobytes {
  short s;
  byte b[2];
};

typedef enum State {
  NN_IDLE,
  NN_RX_WAITING,
  NN_RX_FRAME,
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
    byte _txRate; // In bits per second
    // How many divisions of a clock cycle to check for other transmitters...
    byte _caRate; //...before transmitting
    byte _cdRate; //...while transmitting
    NanoNetState _state;
    bool _sendByte(byte tx_byte);
  public:
    inline NanoNet() {
      _txRate = 1;
      _caRate = 4;
      _cdRate = 4;
      _state = NN_IDLE;
      pinMode(NN_clockPin, INPUT);
      pinMode(NN_dataPin, INPUT);
      // TODO: dont do this
      attachInterrupt(digitalPinToInterrupt(NN_clockPin), _NN_onClockPulse, RISING);
    }
    inline NanoNet(byte txRate) {
      _txRate = txRate;
      _caRate = 4;
      _cdRate = 4;
      _state = NN_IDLE;
      pinMode(NN_clockPin, INPUT);
      pinMode(NN_dataPin, INPUT);
      // TODO: dont do this
      attachInterrupt(digitalPinToInterrupt(NN_clockPin), _NN_onClockPulse, RISING);
    }
    bool sendFrame(char* payload);
    bool recieveFrame(char *buf);
};

// convienience is the mother of invention
// ...
// in this case the invention is exceptions but those are evil
// tl;dr this lets me do if(the byte doesnt send) {return false} without having to write that out 30 times
// never used the c preprocessor beyond basic object defines until this so thats cool
#define _NN_SEND_BYTE(TX_BYTE) if(!_sendByte(TX_BYTE)) { digitalWrite(NN_clockPin, LOW); digitalWrite(NN_dataPin, LOW); pinMode(NN_clockPin, INPUT); pinMode(NN_dataPin, INPUT); return false; }
bool NanoNet::_sendByte(byte tx_byte) {
  for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
    bool tx_bit = tx_byte & (0x80 >> bit_idx);
    // Write data to pin,, then wait half a tick, THEN turn the clock signal on
    digitalWrite(NN_dataPin, tx_bit);
    // While the clock is off, check that it really is off _cdRate times 
    for (int cd_idx = 0; cd_idx < _cdRate; cd_idx++) {
      delay((1000 / _txRate) / (_cdRate * 2));
      if (digitalRead(NN_clockPin)) {
        Serial.println("Collision detected, abort");
        return false;
      }
    }
    digitalWrite(NN_clockPin, HIGH);
    delay((1000 / _txRate) / 2);
    digitalWrite(NN_clockPin, LOW);
  }
  return true;
}

bool NanoNet::sendFrame(char *payload) {
  _state = NN_TX;
  Serial.println("Checking for signal");
  // Make sure the bus isnt in use
  for (int ca_idx = 0; ca_idx < _caRate; ca_idx++) {
    delay((1000 / _txRate) / _caRate);
    if (digitalRead(NN_clockPin)) {
      Serial.println("Signal found, abort");
      return false;
    }
  }
  Serial.println("No signal found");
  pinMode(NN_clockPin, OUTPUT);
  pinMode(NN_dataPin, OUTPUT);
  digitalWrite(NN_clockPin, LOW);
  digitalWrite(NN_dataPin, LOW);
  Serial.println("Sending start header");
  // our recieve buffer is 16 bytes, and 0xFF shouldnt reaaaally be intentionally transmitted,
  // so we can use that in front of our start header byte to make a start transmission doublebyte
  _NN_SEND_BYTE(0xFF);
  _NN_SEND_BYTE(0x02);
  NN_crc.clearCrc();
  Serial.println("Sending payload");
  for (int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
    char tx_byte = payload[byte_idx];
    _NN_SEND_BYTE(payload[byte_idx]);
    NN_crc.updateCrc(payload[byte_idx]);
  }
  Serial.println("Sending CRC");
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
  _state = NN_IDLE;
  return true;
}

// This is execution coordinate hell so just bear with me
// "serial ifs considered harmful" -ezra june barrow 2020
bool NanoNet::recieveFrame(char *buf) {
  int buf_pos = 0;
  int bit_pos = 0;
  twobytes rx_crc;
  _state = NN_RX_WAITING;
  while (true) {
    if (NN_new_bit == true) {
      // Step 2
      if (_state == NN_RX_FRAME) {
        if (++bit_pos == 8) {
          bit_pos = 0;
          if (NN_rx_buf.b[0] == NN_ETX) {
    		// This ends step 2
            Serial.println("Entered CRC");
            buf[buf_pos++] = 0x00;
            _state = NN_RX_CRC;
            buf_pos = 0;
          } else {
    	    // This is the meat of step 2
            buf[buf_pos++] = NN_rx_buf.b[0];
            NN_crc.updateCrc(NN_rx_buf.b[0]);
          }
        }
      } else if (_state = NN_RX_CRC) {
    	// Step 3
        bit_pos++;
        if (bit_pos == 8) {
    	  // Step 3.1
          rx_crc.b[0] = NN_rx_buf.b[0];
          Serial.println("Got First CRC Byte");
        } else if (bit_pos == 16) {
    	  // Step 3.2
          rx_crc.b[1] = NN_rx_buf.b[0];
          Serial.println("Got Second CRC Byte");
        } else if (bit_pos == 24 && NN_rx_buf.b[0] == NN_EOT) {
    	  // Step 3.3
          if (NN_crc.getCrc() != rx_crc.s) {
            Serial.println("Bad data");
          } else {
            Serial.println(buf);
            break;
          }
        }
      }
      // Step 1
      if (NN_rx_buf.s == NN_SOT) {
        _state = NN_RX_FRAME;
        bit_pos = 0;
        buf_pos = 0;
        NN_crc.clearCrc();
        Serial.println("Entered Frame");
      }
      NN_new_bit = false;
    }
  }
  _state = NN_IDLE;
}


#endif
