#include "NanoNet.h"

char *message = "Goodbye Non-world!!!";
//char *message = "Hello World!!!";
NanoNet nanonet = NanoNet(20);

void setup() {
  Serial.begin(9600);
//  delay(1000);
  if(nanonet.sendFrame(message)) {
    Serial.println("Successfully sent");
  } else {
    Serial.println("Try again");
  }
}

void loop() {
}
