#include "Crc16.h"
#include "NanoNet.h"

char rx_buf[1024];
NanoNet nanonet;

void setup() {
  Serial.begin(9600);
}

void loop() {
  nanonet.recieveFrame(rx_buf);
}
