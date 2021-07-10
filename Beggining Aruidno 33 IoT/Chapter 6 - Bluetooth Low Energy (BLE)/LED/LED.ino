/*
 * DEMO 2: CONTROLLING LED WITH BLE
 * 
 * LED controller over BLE radio. Turn on/off LED using mobile application.
 */

 // Library
 #include <ArduinoBLE.h>

 // Initizalize the BLE Service with BLERead and BLEWrite
 // You can generate UUID using this online tool,
 // https://www.guidgenerator.com/online-guid-generator.aspx
 BLEService ledService("1fea24c9-8672-4873-847e-fa990e1fb792");
 BLEByteCharacteristic switchCharacteristic("1fea24c9-8672-4873-847e-fa990e1fb792", BLERead | BLEWrite);
 //const int ledPin = LED_BUILTIN;
 const int ledPin = 3;

 // Initialize serial communication and digital OUTPUT mode
void setup() {
  Serial.begin(9600);
  while(!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()){
    Serial.println("starting BLE failed!");

    while(1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // see the initial value for the chracteristic
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");

}

void loop() {
  BLEDevice central = BLE.central();

  if (central){
    Serial.print("Connected to central: ");
    // print the centnra's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()){
      // if the remote device wrote to the characteristic, use the value to conrol de LED
      if (switchCharacteristic.written()){
        int val = switchCharacteristic.value();
        Serial.println(val);
        if (val>0){ // any value ohter than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH); // Will turn the LED on
        }
        else{
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);  // Will turn the LED off
        }
      }
    }

    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

}
