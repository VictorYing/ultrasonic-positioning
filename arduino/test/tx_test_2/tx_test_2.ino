// One Arduino controls two transmitters, alternately sending
// out pings from the two of them, simulating two synchronized
// transmitter stations sending out pings in turn.

#define TX_1_PORT PORTD
#define TX_2_PORT PORTB

enum {
  TX_1_PIN_1 = 2,
  TX_1_PIN_2 = 3,
  TX_2_PIN_1 = 10,
  TX_2_PIN_2 = 11,
  TX_1_PIN_1_MASK = 1u << (TX_1_PIN_1 % 8u),
  TX_1_PIN_2_MASK = 1u << (TX_1_PIN_2 % 8u),
  TX_2_PIN_1_MASK = 1u << (TX_2_PIN_1 % 8u),
  TX_2_PIN_2_MASK = 1u << (TX_2_PIN_2 % 8u),
  FREQ = 25000u,  // Hz
  PERIOD = 1000000u / FREQ,  // μs
  HALF_PERIOD = PERIOD / 2u,  // μs
  DURATION = 5000u,  // μs
  SPACING = 100000u,  // μs
  CYCLE = 1000000u,  // μs
};

void setup() {
  pinMode(TX_1_PIN_1, OUTPUT);
  pinMode(TX_1_PIN_2, OUTPUT);
  pinMode(TX_2_PIN_1, OUTPUT);
  pinMode(TX_2_PIN_2, OUTPUT);
}

void loop() {
  unsigned long beginning = micros();

  // Send out ping from transmitter 1
  unsigned long time = micros() - beginning;
  unsigned long next = time;
  while (time < DURATION) {
    // Turn off TX_1_PIN_2 and turn on TX_1_PIN_1
    TX_1_PORT = TX_1_PORT & (~TX_1_PIN_2_MASK) | TX_1_PIN_1_MASK;
    next += HALF_PERIOD;
    while (micros() - beginning < next)
      ;

    // Turn off TX_1_PIN_1 and turn on TX_1_PIN_2
    TX_1_PORT = TX_1_PORT & (~TX_1_PIN_1_MASK) | TX_1_PIN_2_MASK;
    next += HALF_PERIOD;
    while ((time = micros() - beginning) < next)
      ;
  }

  // Send out a ping from transmitter 2
  while ((time = micros() - beginning) < SPACING)
    ;
  next = time;
  while (time < SPACING + DURATION) {
    // Turn off TX_2_PIN_2 and turn on TX_2_PIN_1
    TX_2_PORT = TX_2_PORT & (~TX_2_PIN_2_MASK) | TX_2_PIN_1_MASK;
    next += HALF_PERIOD;
    while (micros() - beginning < next)
      ;

    // Turn off TX_2_PIN_1 and turn on TX_2_PIN_2
    TX_2_PORT = TX_2_PORT & (~TX_2_PIN_1_MASK) | TX_2_PIN_2_MASK;
    next += HALF_PERIOD;
    while ((time = micros() - beginning) < next)
      ;
  }

  // Wait for end of cycle
  while (micros() - beginning < CYCLE)
    ;
}
