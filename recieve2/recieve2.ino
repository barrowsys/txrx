#define CLOCK 2
#define DATA 3
#define DATA2 4

volatile byte rx_byte = 0;
volatile int bit_position = 0;

void setup() {
  pinMode(CLOCK, INPUT);
  pinMode(DATA, INPUT);
  pinMode(DATA2, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(CLOCK), onClockPulse, RISING);
}

void onClockPulse() {
  bool rx_bit = digitalRead(DATA);
  bool rx_bit2 = digitalRead(DATA2);
  if(bit_position == 8) {
    rx_byte = 0;
    bit_position = 0;
  }
  
  if(rx_bit) {
    rx_byte |= (0x80 >> bit_position);
  }
  if(rx_bit2) {
    rx_byte |= (0x80 >> (bit_position+1));
  }
  bit_position += 2;

  if(bit_position == 8) {
    Serial.write(rx_byte);
  }
}

void loop() {
}
