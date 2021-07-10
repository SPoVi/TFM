/*
 * DEMO 1: HELLO ARDUINO BLE
 * 
 * If the BLE reader is connected, we can turn on LED. Off when disconnected.
 */

// Libraries
#include <ArduinoBLE.h>

// Constan var
int led1 = 4;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led1, OUTPUT);

  // Begin initialization
  if (!BLE.begin()){
    Serial.println("Starting BLE failed!");
    while(1);
  }

  // Set our BLE radio name
  BLE.setLocalName("HelloBLE");
  BLE.setAdvertisedServiceUuid("1fea24c9-8672-4873-847e-fa990e1fb792"); // Set BLE UUID
  // You can generate UUID using this online tool,
  // https://www.guidgenerator.com/online-guid-generator.aspx

  // Start Advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");

  /*
   * Make sure your BLE UUID complies with standard BLE SIG. Some BLE
   * UIDs are reserved by their services. You can check these services at this
   * nk, https://www.bluetooth.com/specifications/assigned-numbers/
   * ervice-discovery/.
   */
}

void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();

  if (central){
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    //digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(led1, HIGH);

    while(central.connected()){
      // do nothing
    }

    //digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(led1, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

}
