/* SCANNING WIFI HOTSPOT
 *  We can access a WiFi hotspot if we know the WiFi SSID name.
 *  We'll scan an existing WiFi SSID and then print the list on the serial terminal
 */


// Libraries
#include <SPI.h>
#include <WiFiNINA.h>
//#include <ESP8266WiFi.h>

#include "arduino_secrets.h"

//SSID and Password of your WiFi router
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

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

  // WiFi
  scanWiFi();
  connectToWiFi();

}

void loop() {

}

/*
 * PROCEDURES AND FUNCTIONS
 */

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

// PROCEDURE - CONNECT TO WIFI
void connectToWiFi(){
  // Connect to WiFi Network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to Wifi");
  Serial.println("...");

  WiFi.begin(ssid, password);
  int retries = 0;

  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)){
    retries++;
    delay(500);
    Serial.print(".");
  }

  if (retries > 14){
    Serial.println(F("WiFi connection FAILED"));
  }

  if (WiFi.status() == WL_CONNECTED){
    Serial.println(F("WiFi connected!"));
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println(F("Setup ready"));
}