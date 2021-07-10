/*
 * DEMO 3: SENSOR REAL-TIME MONITORING
 * 
 * Make a BLE service that provides Gyroscope sensor data to the BLE reader.
 * The BLE reader will obtain notification if the sensor data changes.
 */

 // Libraries
 #include <ArduinoBLE.h>
 #include <Arduino_LSM6DS3.h>  //Gyroscope

 /* 
  *  Define BLE service an three BLE characteristics.
  *  We need different UUIDs to apply these features.
  *  We define three variables to hold sensor data.
  */

  // You can generate UUID using this online tool,
  // https://www.guidgenerator.com/online-guid-generator.aspx

  
  BLEService sensorService("7dcacd35-b87a-4931-9a7d-290bf9440824");
  // X
  BLEStringCharacteristic xSensorLevel("4500aa8a-570a-4c89-a014-bac08be0bd7c", BLERead | BLENotify,15);
  // Y
  BLEStringCharacteristic ySensorLevel("f8ec7c20-39d2-4ccb-88d6-5f863431a674", BLERead | BLENotify,15);
  // Z
  BLEStringCharacteristic zSensorLevel("b33aa585-98ea-43f6-9f8a-015c8e3a53cf", BLERead | BLENotify,15);
  
  // last sensor data
  float oldXLevel = 0;
  float oldYLevel = 0;
  float oldZLevel = 0;
  long previousMillis = 0;

  // LED
  int led1 = 6; // Blue
  int led2 = 4; // Red
  int led3 = 3; // Orange 
  

void setup() {
  Serial.begin(115200);  
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // Define BLE service name and add to he advertised service
  BLE.setLocalName("Gyroscope");  // Service name
  BLE.setAdvertisedService(sensorService);  // Advertised service

  // add the BLE characteristics into the BLE service
  sensorService.addCharacteristic(xSensorLevel);
  sensorService.addCharacteristic(ySensorLevel);
  sensorService.addCharacteristic(zSensorLevel);
  BLE.addService(sensorService);

  // ser initial default data on all BLE charcteristics
  xSensorLevel.writeValue(String(0));
  ySensorLevel.writeValue(String(0));
  zSensorLevel.writeValue(String(0));

  // Start to advertise de BLE service by calling the BLE
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");  
}

/*
 * Wait for the incoming BLE reader.
 * Once is connected, print thr MAC address ont the BLE reader.
 * Then, turn on the LED.
 */
void loop() {
  BLEDevice central = BLE.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(led1, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);

    // If the BLE reader is connected, we have the BLEDevice object.
    // Perform looping funcion until BLE reader is disconnected
    while (central.connected()){
      //long currentMillis = millis();
      updateGyroscopeLevel();
      delay(300);
    }

    // Turn off the LED after BLE reader disconnected
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(led1, LOW);
    Serial.print("Disconnected from central: ");
    Serial.print(central.address());
  }
}

/*
 * FUNCTIONS
 */

 void updateGyroscopeLevel(){
  float x, y, z;

  if (IMU.gyroscopeAvailable()){
    IMU.readGyroscope(x, y, z);

    // Send Gyroscope data to the BLE service
    if (x != oldXLevel){
      xSensorLevel.writeValue(String(x));
      oldXLevel = x;
    }
    if (y != oldYLevel){
      ySensorLevel.writeValue(String(y));
      oldYLevel = y;
    }
    if (z != oldZLevel){
      zSensorLevel.writeValue(String(z));
      oldZLevel = z;
    }
    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
    
  }
 }
