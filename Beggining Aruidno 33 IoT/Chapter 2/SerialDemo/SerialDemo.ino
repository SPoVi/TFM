// Serial communication is the process of sending data one bit a time, sequentially, over a 
// communication chanel.

// In Arduino Nano 33 IoT, we can implement serial communication using a Serial object.
// Further information: https://www.arduino.cc/reference/en/language/functions/communication/serial/.


int led = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial){
    ;
  }

}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("LED : HIGH");
  digitalWrite(led, HIGH);
  delay(1000);
  Serial.println("LED: LOW");
  digitalWrite(led, LOW);
  delay(1000);
  

}
