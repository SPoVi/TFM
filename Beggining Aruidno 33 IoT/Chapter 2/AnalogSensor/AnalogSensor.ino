void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial){
    ;
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  int reading = analogRead(A0);

  float voltage = reading * 3.3;
  voltage /= 1024.0;

  Serial.print(voltage); Serial.print(" volts");
  
  float tempC = (voltage - 0.5) * 100;

  Serial.print(tempC);
  Serial.println(" degrees C");
  delay(3000);

}
