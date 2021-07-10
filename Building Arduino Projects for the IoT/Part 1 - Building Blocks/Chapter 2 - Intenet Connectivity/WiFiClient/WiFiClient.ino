/*
 * INTERNET CONNECTIVITY (WIRELESS)
 */

 // Libraries
 #include <SPI.h>
 #include <WiFi.h>

 // Other doc
 #include "arduino_secrets.h"

 // Static Var
 char ssid[] = SECRET_SSID;
 char pass[] = SECRET_PASS;

 int keyIndex = 0;
 int status = WL_IDLE_STATUS;

 WiFiClient client;

/*
 * Functions & Procedures
 */

void connectToInternet(){
  status = WiFi.status();

  // Check for the presence of the shield
  if (status == WL_NO_SHIELD){
    Serial.println("[ERROR] WiFi Shiel Not Present");
    // Do nothing
    while (true);
  }

  // Attemp to connect to WPA/WPA2 WiFi network
  while (status != WL_CONNECTED){
    Serial.print("[INFO] Attempting Connection - WPA SSID: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
  }

  // Connection successful
  Serial.print("[INFO] Connection Successful");
  Serial.print("");
  printConnectionInformation();
  Serial.println("--------------------------------------------------------------");
  Serial.println("");
}

void printConnectionInformation(){
  // Print Network SSID
  Serial.print("[INFO] SSID: ");
  Serial.print(WiFi.SSID());

  // Print Router's MAC address
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("[INFO] BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);
  

  // Print received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("[INFO] Signal Strength (RSSI): ");
  Serial.println(rssi);

  // Print encryption type
  byte encryption = WiFi.encryptionType();
  Serial.print("[INFO] encryption Type: ");
  Serial.print(encryption, HEX);

  // Print WiFi Shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("[INFO] IP Address: ");
  Serial.println(ip);

  // Print MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("[INFO] MAC Address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

 
/*
 * SETUP & LOOP
 */
 
 
void setup() {
  Serial.begin(115200);
  while(!Serial){
    ;
  }

  connectToInternet();

}

void loop() {
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
