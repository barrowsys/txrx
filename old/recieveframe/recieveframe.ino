/**
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
#include "NanoNet.h"

char rx_buf[1024];
NanoNet nanonet = NanoNet(0x01);

void setup() {
  Serial.begin(9600);
  Serial.println("I'm 0x01 (Recieve Only)");
}

void loop() {
  byte r = nanonet.recieveFrame(rx_buf);
  if(r != 0) {
    Serial.print("Message from 0x");
    Serial.print(r, HEX);
    Serial.print(": ");
    Serial.println(rx_buf);
  }
}
