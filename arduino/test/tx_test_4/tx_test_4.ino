// One Arduino controling one transmitters, simulates four transmitters

#define TX_PORT PORTD

enum {
  TX_PIN_1 = 2,
  TX_PIN_2 = 3,
  TX_PIN_1_MASK = 1u << (TX_PIN_1 % 8u),
  TX_PIN_2_MASK = 1u << (TX_PIN_2 % 8u),
  FREQ = 25000u,  // Hz
  PERIOD = 1000000u / FREQ,  // μs
  HALF_PERIOD = PERIOD / 2u,  // μs
  NUM_TRANSMITTERS = 4u,
  DURATION = 5000u,  // μs
};

unsigned const long SPACINGS[NUM_TRANSMITTERS] =
    {117766u, 95420u, 77814u, 709001u};

void setup() {
  pinMode(TX_PIN_1, OUTPUT);
  pinMode(TX_PIN_2, OUTPUT);
}

void loop() {
  static unsigned i = 0;
  unsigned long beginning = micros();
  
  // Send out ping from transmitter
  unsigned long time = micros() - beginning;
  unsigned long next = time;
  while (time < DURATION) {
    // Turn off TX_PIN_2 and turn on TX_PIN_1
    TX_PORT = TX_PORT & (~TX_PIN_2_MASK) | TX_PIN_1_MASK;
    next += HALF_PERIOD;
    while (micros() - beginning < next)
      ;

    // Turn off TX_PIN_1 and turn on TX_PIN_2
    TX_PORT = TX_PORT & (~TX_PIN_1_MASK) | TX_PIN_2_MASK;
    next += HALF_PERIOD;
    while ((time = micros() - beginning) < next)
      ;
  }

  // wait until time for next ping
  unsigned long spacing = SPACINGS[i];
  i = (i + 1) % NUM_TRANSMITTERS;
  while (micros() - beginning < spacing)
    ;
}
