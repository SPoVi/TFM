/* SERIAL PERIPHERAL INTERFACE 
// Serial communication works with asynchronous mode so there is no control on serial communication.
//
// This means we cannot guarantee the data that is sent will be received by receiver*/
//
// Ther serial peripheral interface (SPI) is a synchronous serial communication interface specification,
// but SPI has four wires to control data such as master out/slave in (MOSI), master in/slave out (MISO),
// serial clock signal (SLCK),and slave select (SS)
//
// MOSI on Digital pin 11
// MISO on DIgital pin 12
// SCLK on Digital pin 13 */

// Library
#include <SPI.h>

// Global vars
byte sendData, recvData;

void setup() {
  // put your setup code here, to run once:
  SPI.begin();
  Serial.begin(9600);
  randomSeed(80);

}

void loop() {
  // put your main code here, to run repeatedly:
  sendData = random(50,100);
  recvData = SPI.transfer(sendData);

  Serial.print("Send = ");
  Serial.println(sendData, DEC);
  Serial.print("Recv= ");
  Serial.println(recvData, DEC);
  delay(800);

}
