/*****************************************************************
slave_transmitter.ino

Responds to messages from the master and sends out ultrasonic pings
at the appropriate time.

Hardware Hookup:
  The XBee Shield makes all of the connections you'll need
  between Arduino and XBee. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION. That will connect
  the XBee's DOUT and DIN pins to Arduino pins 2 and 3.
*****************************************************************/
// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>
#define TX_PORT PORTB
#define TRANSMITTER_NUMBER 1

enum {
  TX_PIN_1 = 12,
  TX_PIN_2 = 13,
  TX_PIN_1_MASK = 1u << (TX_PIN_1 % 8u),
  TX_PIN_2_MASK = 1u << (TX_PIN_2 % 8u),
  FREQ = 25000u,  // Hz
  PERIOD = 1000000u / FREQ,  // μs
  HALF_PERIOD = PERIOD / 2u,  // μs
  DURATION = 5000u,  // μs
  TIMEOUT = 50000u, // μs
  BAUD_RATE = 9600u, // bps
  MAX_LAT_TIME = 20000u, // μs
  SOFTWARE_SERIAL_DELAY = 2000u, // μs
};

// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX
unsigned long latTime;

void setup()
{
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  XBee.begin(BAUD_RATE);
  //Serial.begin(BAUD_RATE); // Serial for debugging
  
  pinMode(TX_PIN_1, OUTPUT);
  pinMode(TX_PIN_2, OUTPUT);
}

void loop()
{
  int i;
  char c;
  byte b[4];
  
  if (XBee.available())
  { // If data comes in from XBee, send it out to serial monitor
    c = XBee.read();
    if (c == (char)('a' + TRANSMITTER_NUMBER - 1)) {
      XBee.write(c);
    }
    else if (c == (char)('A' + TRANSMITTER_NUMBER - 1)) {
      for (i = 0; i < 4; i++) {
        unsigned long beginning = micros();
        while (!XBee.available() && (micros()-beginning < TIMEOUT));
        if (XBee.available()) {
          b[i] = XBee.read();
        }
        else {
          break;
        }
      }
      if (i == 4) {
       unsigned long temp = BytesToLong(b);
       if (temp < MAX_LAT_TIME) {
         latTime = temp;
       }
      }
      XBee.write(c);
    }
    else if (c == 'p') {
      delayMicroseconds(SOFTWARE_SERIAL_DELAY);
      sendPing();
    }
  }
}

unsigned long BytesToLong(byte b[4]) {
  unsigned long ret = 0u;
  for (int i = 0; i < 4; i++) {
    ret += (unsigned long)(b[i] & 0xFF) << 8*(3-i);
  }
  return ret;
}

void sendPing() {
  unsigned long beginning = micros();
  //Serial.println("Sending ping");
  while(micros() - beginning < (100000u*TRANSMITTER_NUMBER)-latTime)
    ;
  
  // Send out ping from transmitter
  beginning = micros();
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
  
  //Serial.println("Ping sent");
}


