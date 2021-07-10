/* WORKING WITH GYROSCOPE
 *  Measure orientation and angular velocity
 */


// Libraries
#include <Arduino_LSM6DS3.h>

void setup() {
  
  Serial.begin(115200);
  while(!Serial);

  if (!IMU.begin()){
    Serial.println("Failed to initialize IMU!");

    while(1);
  }

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.print("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");
}


void loop() {
  float x, y, z;

  if (IMU.gyroscopeAvailable()){
    IMU.readGyroscope(x, y, z);

    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }

}