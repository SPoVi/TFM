/*
 * WIFI WEB CLIENT
 */

// Libraries
#include <SPI.h>
#include <WiFiNINA.h>

// Other doc
#include "arduino_secrets.h"

// Static Var
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "www.google.com";    // name address for Google (using DNS)

WiFiClient client;



void setup() {
  Serial.begin(115200);
  while(!Serial){
    ;
  }

  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE){
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while(true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  
  // Attempt to connect to WiFi network:
  while(status != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    // Wait 10 seconds fot connection
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  // HTTP GET
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)){
    Serial.println("connected to server");
    // Make a HTTP request
    client.println("GET /search?q=arduino HTTP/1.1·");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()){
    char c = client.read();
    Serial.write(c);
  }

  // If the server's disconnected, stop the client
  if (!client.connected()){
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore
    while(true);
  }
}


// PROCEDURE
void printWiFiStatus(){
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}
