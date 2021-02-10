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
#include <Scheduler.h>
#include "NanoNet.h"
#include <EEPROM.h>

char *message1 = "Debug Message";
char *message2 = "Lol im a leonardo get owned";
NanoNet nanonet = NanoNet();
NanoNet nanonet2 = NanoNet();
byte MAC;

void setup() {
  Serial.begin(9600);
  MAC = EEPROM.read(0);
  nanonet = NanoNet(MAC, false);
  if(MAC == 0x20) {
    nanonet2 = NanoNet(MAC, true);
  }
  pinMode(13, OUTPUT);
  Serial.print("I'm 0x");
  Serial.println(MAC, HEX);
}

void loop() {
  if(MAC == 0x01) {
  } else if(MAC == 0x02) {
  } else if(MAC == 0x11) {
  } else if(MAC == 0x12) {
  } else if(MAC == 0x20) {
  }
}
