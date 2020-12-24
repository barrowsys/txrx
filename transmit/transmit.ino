#define CLOCK 2
#define DATA 3
#define TX_RATE 10

const char *message = "Hello, World!\r\n";

void setup() {
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  delay(1000);
  for(int byte_idx = 0; byte_idx < strlen(message); byte_idx++) {
    char tx_byte = message[byte_idx];
    for(int bit_idx = 0; bit_idx < 8; bit_idx++) {
      bool tx_bit = tx_byte & (0x80>>bit_idx);
      digitalWrite(DATA, tx_bit);
      delay((1000 / TX_RATE) / 2);
      digitalWrite(CLOCK, HIGH);
      delay((1000 / TX_RATE) / 2);
      digitalWrite(CLOCK, LOW);
    }
  }
  digitalWrite(DATA, LOW);
  digitalWrite(CLOCK, LOW);
}

void loop() {
}
