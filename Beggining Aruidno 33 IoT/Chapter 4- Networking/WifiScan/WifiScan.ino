/* SCANNING WIFI HOTSPOT
 *  We can access a WiFi hotspot if we know the WiFi SSID name.
 *  We'll scan an existing WiFi SSID and then print the list on the serial terminal
 */


// Libraries
#include <SPI.h>
#include <WiFiNINA.h>

// Global var
int led = 13;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  // Check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE){
    Serial.println("Communication with WiFi module failed !");
    // don't continue
    while(true);
  }

}

void loop() {
  digitalWrite(led, HIGH);
  scanWiFi();   // procedure
  digitalWrite(led, LOW);
  delay(15000); // 15 seg
}

// PROCEDURE - SCANWIFI
void scanWiFi(){
  Serial.print("Scanning...");
  byte ssid = WiFi.scanNetworks();

  Serial.print("found ");
  Serial.println(ssid);

  for (int i = 0; i<ssid; i++){
    Serial.print(">> ");
    Serial.print(WiFi.SSID(i));       // Nombre de la red WiFi
    Serial.print("\t\tRSSI: ");
    Serial.print(WiFi.RSSI(i));       // Fuerza de la red
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");   // Tipo de encriptacion
    Serial.println(WiFi.encryptionType(i));
  }

  Serial.println("");
  Serial.println("");
}
