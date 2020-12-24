#ifndef NANONET_H
#define NANONET_H

#include "Crc16.h"

#ifndef NN_clockPin
#define NN_clockPin 2
#endif
#ifndef NN_dataPin
#define NN_dataPin 3
#endif

#define NN_SOT 0xFF02
#define NN_SOH 0x02
#define NN_STX 0x03
#define NN_ETX 0x04
#define NN_EOT 0x05

union twobytes {
  short s;
  byte b[2];
};

typedef enum State {
  NN_IDLE,
  NN_RX_WAITING,
  NN_RX_FRAME,
  NN_RX_CRC,
  NN_TX,
} NanoNetState;

Crc16 NN_crc;

volatile twobytes NN_rx_buf;
volatile bool NN_new_bit;
void _NN_onClockPulse() {
  bool rx_bit = digitalRead(NN_dataPin);

  NN_rx_buf.s = NN_rx_buf.s << 1;
  NN_new_bit = true;
  
  if(rx_bit) {
    NN_rx_buf.s |= 0x01;
  }
}

class NanoNet {
  private:
    byte _txRate;
    byte _caRate;
    byte _cdRate;
    NanoNetState _state;
    bool _sendByte(byte tx_byte);
    void _onClockPulse();
  public:
    inline NanoNet() {
      _txRate = 1;
      _caRate = 4;
      _cdRate = 4;
      _state = NN_IDLE;
      pinMode(NN_clockPin, INPUT);
      pinMode(NN_dataPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(NN_clockPin), _NN_onClockPulse, RISING);
    }
    inline NanoNet(byte txRate) {
      _txRate = txRate;
      _caRate = 4;
      _cdRate = 4;
      _state = NN_IDLE;
      pinMode(NN_clockPin, INPUT);
      pinMode(NN_dataPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(NN_clockPin), _NN_onClockPulse, RISING);
    }
    bool sendFrame(char* payload);
    bool recieveFrame(char *buf);
};

#define _NN_SEND_BYTE(TX_BYTE) if(!_sendByte(TX_BYTE)) { digitalWrite(NN_clockPin, LOW); digitalWrite(NN_dataPin, LOW); pinMode(NN_clockPin, INPUT); pinMode(NN_dataPin, INPUT); return false; }
bool NanoNet::_sendByte(byte tx_byte) {
  for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
    bool tx_bit = tx_byte & (0x80>>bit_idx);
    digitalWrite(NN_dataPin, tx_bit);
    for(int cd_idx = 0; cd_idx < _cdRate; cd_idx++) {
      delay((1000 / _txRate) / (_cdRate*2));
      if(digitalRead(NN_clockPin)) {
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
  for(int ca_idx = 0; ca_idx < _caRate; ca_idx++) {
    delay((1000 / _txRate) / _caRate);
    if(digitalRead(NN_clockPin)) {
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
  _NN_SEND_BYTE(0xFF);
  _NN_SEND_BYTE(0x02);
  NN_crc.clearCrc();
  Serial.println("Sending payload");
  for(int byte_idx = 0; byte_idx < strlen(payload); byte_idx++) {
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
  digitalWrite(NN_clockPin, LOW);
  digitalWrite(NN_dataPin, LOW);
  pinMode(NN_clockPin, INPUT);
  pinMode(NN_dataPin, INPUT);
  _state = NN_IDLE;
  return true;
}

bool NanoNet::recieveFrame(char *buf) {
  int buf_pos = 0;
  int bit_pos = 0;
  twobytes rx_crc;
  _state = NN_RX_WAITING;
  while(true) {
    if(NN_new_bit == true) {
      if(_state == NN_RX_FRAME) {
        if(++bit_pos == 8) {
          bit_pos = 0;
          if(NN_rx_buf.b[0] == NN_ETX) {
            Serial.println("Entered CRC");
            buf[buf_pos++] = 0x00;
            _state = NN_RX_CRC;
            buf_pos = 0;
          } else {
            buf[buf_pos++] = NN_rx_buf.b[0];
            NN_crc.updateCrc(NN_rx_buf.b[0]);
          }
        }
      } else if(_state = NN_RX_CRC) {
        bit_pos++;
        if(bit_pos == 8) {
          rx_crc.b[0] = NN_rx_buf.b[0];
          Serial.println("Got First CRC Byte");
        } else if(bit_pos == 16) {
          rx_crc.b[1] = NN_rx_buf.b[0];
          Serial.println("Got Second CRC Byte");
        } else if(bit_pos == 24 && NN_rx_buf.b[0] == NN_EOT) {
          if(NN_crc.getCrc() != rx_crc.s) {
            Serial.println("Bad data");
          } else {
            Serial.println(buf);
            break;
          }
        }
      }
      if(NN_rx_buf.s == NN_SOT) {
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
