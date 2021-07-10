/* WORKING WITH AN ACCELERATOR */

// Libraries
#include <Arduino_LSM6DS3.h>

void setup() {
  
  Serial.begin(115200);
  while(!Serial);

  if (!IMU.begin()){
    Serial.println("Failed to initialize IMU!");

    while(1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.print("Acceleration in G's");
  Serial.println("X\tY\tZ");
}

void loop() {
  float x, y, z;

  if (IMU.accelerationAvailable()){
    IMU.readAcceleration(x, y, z);

    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }

}
