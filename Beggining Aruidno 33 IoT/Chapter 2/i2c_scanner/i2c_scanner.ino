/* INTERINTEGRATED CIRCUIT (I2C)
 * Is a protocol intended to allow multiple "slave" module/decive (chip) to communicate with one or 
 * more "master" chips.
 * 
 * Works with asynchronous mode.
 * 
 * To communicate with other devices/modules, it defines the I2C address for all "slaves" devices.
 * 
 * The I2C interface has two pins: serial data (SDA) and serial clock (SLC)
 * SDA on Analog pin 4
 * SLC on Analog pin 5
 */

 /* SCANNING I2C ADDRESS
  *  We want to scan all devices that are attached on Arduino.
  *  To access I2C on the Arduino board, we can use the Wire library.
  */

 // Libraries
 #include <Wire.h>
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  Serial.println("\nI2C Scanner");

}

void loop() {
  // put your main code here, to run repeatedly:
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;

  for (address = 1; address < 127; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0){
      Serial.print("I2C device found at address 0x");
      if (address < 16){
        Serial.print("0");
      }
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4){
      Serial.print("Unknown error at address 0x");
      if (address < 16){
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found");
  }
  else{
    Serial.println("done");
  }
  delay(5000);

}
