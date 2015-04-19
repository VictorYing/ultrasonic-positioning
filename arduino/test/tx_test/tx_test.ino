// One Arduino controls transmitters on pins 3 and 11, alternately
// sending out pings from the two of them, simulating two synchronized
// transmitter stations sending out pings in turn.

enum {
  tx_pin_1 = 3,
  tx_pin_2 = 11,
};

void setup() {
  pinMode(tx_pin_1, OUTPUT);
  pinMode(tx_pin_2, OUTPUT);
}

void loop() {
  tone(tx_pin_1, 25000u, 1u);
  delay(100u);
  tone(tx_pin_2, 25000u, 1u);
  delay(1000u);
}
