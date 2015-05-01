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

#define NUM_TRANSMITTERS 2
#define NUM_TESTS 5
// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

void setup()
{
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  XBee.begin(9600);
  Serial.begin(9600);
}

void loop()
{
  int i, j;
  unsigned long startTime, totalLatTime, averageLatTime;
  byte b[4];
  unsigned long startTotTime = micros();
    
  for (i = 0; i < NUM_TRANSMITTERS; i++) {
    totalLatTime = 0u;
    for (j = 0; j < NUM_TESTS; j++) {
      startTime = micros();
      XBee.write((char)('a' + i));
      while (!XBee.available());
      XBee.read();
      totalLatTime += (micros()-startTime);
      //Serial.println(totalLatTime);
    }
    averageLatTime = totalLatTime/(2*NUM_TESTS);
    Serial.print("Average Time: ");
    Serial.println(averageLatTime);
    XBee.write((char)('A' + i));
    LongToBytes(averageLatTime, b);
    for (j = 0; j < 4; j++) {
      XBee.write(b[j]); 
    } 
    while (!XBee.available());
    XBee.read();
  }
  XBee.write('p');
  Serial.print("Total Time: ");
  Serial.println(micros()-startTotTime);
  delay(1000);
}

void LongToBytes(unsigned long val, byte b[4]) {
  b[0] = (byte )((val >> 24) & 0xff);
  b[1] = (byte )((val >> 16) & 0xff);
  b[2] = (byte )((val >> 8) & 0xff);
  b[3] = (byte )(val & 0xff);
}

unsigned long BytesToLong(byte b[4]) {
  return ((unsigned long)(b[0] & 0xFF) << 24) + ((unsigned long)(b[1] & 0xFF) << 16) 
          + ((unsigned long)(b[2] & 0xFF) << 8) + (unsigned long)(b[3] & 0xFF);
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

