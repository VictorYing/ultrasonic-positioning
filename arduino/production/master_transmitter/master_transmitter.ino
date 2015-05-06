/*****************************************************************
XBee_Serial_Passthrough.ino

Set up a software serial port to pass data between an XBee Shield
and the serial monitor.

Hardware Hookup:
  The XBee Shield makes all of the connections you'll need
  between Arduino and XBee. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION. That will connect
  the XBee's DOUT and DIN pins to Arduino pins 2 and 3.
*****************************************************************/
// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>

#define NUM_TRANSMITTERS 3
#define NUM_TESTS 5
#define TX_PORT PORTB

enum {
  TX_PIN_1 = 12,
  TX_PIN_2 = 13,
  TX_PIN_1_MASK = 1u << (TX_PIN_1 % 8u),
  TX_PIN_2_MASK = 1u << (TX_PIN_2 % 8u),
  FREQ = 25000u,  // Hz
  PERIOD = 1000000u / FREQ,  // μs
  HALF_PERIOD = PERIOD / 2u,  // μs
  TIMEOUT = 50000u, // µs
  DURATION = 5000u,  // μs
  BAUD_RATE = 9600u, // bps
};


// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

void setup()
{
  pinMode(TX_PIN_1, OUTPUT);
  pinMode(TX_PIN_2, OUTPUT);
  
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  XBee.begin(BAUD_RATE);
  Serial.begin(BAUD_RATE);
}

void loop()
{
  int i, j;
  unsigned long startTime, totalLatTime, averageLatTime, beginning;
  byte b[4];
  unsigned long startTotTime = micros();
    
  for (i = 0; i < NUM_TRANSMITTERS; i++) {
    totalLatTime = 0u;
    int successCount = 0;
    for (j = 0; j < NUM_TESTS; j++) {
      startTime = micros();
      XBee.write((char)('a' + i));
      while (!XBee.available() && (micros() - startTime < TIMEOUT));
      if (XBee.available()) {
        XBee.read();
        totalLatTime += (micros()-startTime);
        successCount++;
      }
      //Serial.println(totalLatTime);
    }
    if (successCount > 0) {
      averageLatTime = totalLatTime/(2*successCount);
      Serial.print("Average Time: ");
      Serial.println(averageLatTime);
      XBee.write((char)('A' + i));
      LongToBytes(averageLatTime, b);
      for (j = 0; j < 4; j++) {
        XBee.write(b[j]); 
      } 
      beginning = micros();
      while (!XBee.available() && (micros()-beginning < TIMEOUT));
      Serial.print("Acknowledgement: ");
      Serial.write(XBee.read());
      Serial.println();
    }
  }
  XBee.write('p');
  sendPing();
  Serial.print("Total Time: ");
  Serial.println(micros()-startTotTime);
  delay(300);
}

void LongToBytes(unsigned long val, byte b[4]) {
  b[0] = (byte )((val >> 24) & 0xff);
  b[1] = (byte )((val >> 16) & 0xff);
  b[2] = (byte )((val >> 8) & 0xff);
  b[3] = (byte )(val & 0xff);
}

/*
unsigned long BytesToLong(byte b[4]) {
  return ((unsigned long)(b[0] & 0xFF) << 24) + ((unsigned long)(b[1] & 0xFF) << 16) 
          + ((unsigned long)(b[2] & 0xFF) << 8) + (unsigned long)(b[3] & 0xFF);
}
*/

void sendPing() {
  
  //Serial.println("Sending ping");
  
  // Send out ping from transmitter
  unsigned long beginning = micros();
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
  Serial.println("Ping sent");
}
/*
// ASCIItoInt
// Helper function to turn an ASCII hex value into a 0-15 byte val
int ASCIItoInt(char c)
{
  if ((c >= '0') && (c <= '9'))
    return c - 0x30; // Minus 0x30
  else if ((c >= 'A') && (c <= 'F'))
    return c - 0x37; // Minus 0x41 plus 0x0A
  else if ((c >= 'a') && (c <= 'f'))
    return c - 0x57; // Minus 0x61 plus 0x0A
  else
    return -1;
}
*/

