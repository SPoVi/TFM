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

 /* READING SENSOR-BASED I2C ADDRESS
  *  We read sensor data from the I2C device
  *  Each sensor has a channel address on 0xXY.
  */

// Access sensor devices over the I2C interface
#include "Wire.h"

#define PCF8591 0x48 // I2C bus address
#define PCF8591_ADC_CH0 0x00 //thermistor
#define PCF8591_ADC_CH1 0x01 //photo-voltaic cell
#define PCF8591_ADC_CH2 0x02 
#define PCF8591_ADC_CH3 0x03 //potentiometer

byte ADC1, ADC2, ADC3;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);

}

void loop() {
  // Read thermistor
  Wire.beginTransmission(PCF8591);
  Wire.write((byte)PCF8591_ADC_CH0);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(PCF8591,2);
  delay(100);
  ADC1=Wire.read();
  ADC1=Wire.read();

  Serial.print("Thermistor= ");
  Serial.println(ADC1);

  // Lo mismo para los otros sensores cambiando su canal

}
