// One Arduino controls transmitters on pins 3 and 11, alternately
// sending out pings from the two of them, simulating two synchronized
// transmitter stations sending out pings in turn.

enum {
  TX_PIN_1 = 3,
  TX_PIN_2 = 11,
  FREQ = 25000u,  // Hz
  DURATION = 5u,  // ms
};

void setup() {
  pinMode(TX_PIN_1, OUTPUT);
  pinMode(TX_PIN_2, OUTPUT);
}

void loop() {
  tone(TX_PIN_1, FREQ, DURATION);
  delay(100u);
  tone(TX_PIN_2, FREQ, DURATION);
  delay(1000u);
}
