// ARDUINO PROGRAM FOR DATA TRANSFERING

const int analogIn = A0; // Analog input pin from pot
int Value = 0; // Value read from the pot


void setup() {

  Serial.begin(9600); // Initialice serial communication at 9600 bps
}

void loop() {

  Value = analogRead(analogIn); // read the analog in value

  Serial.write(Value); // print the results to the serail monitor

  delay(2); // wait 2 milliseconds before next loop

}
