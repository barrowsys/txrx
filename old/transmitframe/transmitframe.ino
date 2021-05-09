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

char *message = "Debug Message";
char *message1 = "Goodbye Non-world!!!";
char *message2 = "Hello World!!!";
char message_buf[64] = "Message relayed from 0x0: \0";
NanoNet nanonet = NanoNet(0x02);

void setup() {
  Serial.begin(9600);
  Serial.println("I'm 0x02");
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);
}

void loop() {
  if(digitalRead(12)) {
    Serial.println("Sending to 0x01... <<");
    while(!nanonet.sendFrame(message1, 0x01)) { delay(100); }
    Serial.println("Successfully sent to 0x01 <<");
    Serial.println("Recieving from 0x03... //");
    byte r;
    do {
      r = nanonet.recieveFrame(message_buf+26);
    } while(r == 0);
    Serial.println("Successfully recieved from 0x03 //");
    char shortbuf[2];
    itoa(r, shortbuf, 16);
    message_buf[23] = shortbuf[0];
    Serial.println("Relaying to 0x01... <<");
    while(!nanonet.sendFrame(message_buf, 0x01)) {delay(100);}
    Serial.println("Successfully relayed to 0x01 <<");
    Serial.println("Sending to 0x04... vv");
    while(!nanonet.sendFrame(message2, 0x04)) { delay(100); }
    Serial.println("Successfully sent to 0x04 vv");
    delay(1000);
  }
}