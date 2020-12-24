#define CLOCK 2
#define DATA 3

volatile byte rx_byte = 0;
volatile int bit_position = 0;

void setup() {
  pinMode(CLOCK, INPUT);
  pinMode(DATA, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(CLOCK), onClockPulse, RISING);
}

void onClockPulse() {
  bool rx_bit = digitalRead(DATA);
  if(bit_position == 8) {
    rx_byte = 0;
    bit_position = 0;
  }
  
  if(rx_bit) {
    rx_byte |= (0x80 >> bit_position);
  }
  bit_position += 1;

  if(bit_position == 8) {
    Serial.write(rx_byte);
  }
}

void loop() {
}
