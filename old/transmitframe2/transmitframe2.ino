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
#include <Scheduler.h>
#include "NanoNet.h"

char *message1 = "Debug Message";
char *message2 = "Lol im a leonardo get owned";
NanoNet nanonet = NanoNet(0x03);

void setup() {
  Serial.begin(9600);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  Serial.println("I'm 0x03");
}

void loop() {
  if(digitalRead(12)) {
    delay(500);
    Serial.println("Sending to 0x01 ^^");
    while(!nanonet.sendFrame(message2, 0x01)) { delay(100); }
    Serial.println("Successfully sent to 0x01 ^^");
    Serial.println("Sending to 0x02 //");
    while(!nanonet.sendFrame(message1, 0x02)) { delay(100); }
    Serial.println("Successfully sent to 0x02 //");
    delay(1000);
  }
}
