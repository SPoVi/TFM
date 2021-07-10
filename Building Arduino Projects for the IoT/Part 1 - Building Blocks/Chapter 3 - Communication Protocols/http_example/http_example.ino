// =============== External Libraries =================
#include <SPI.h>
#include <WiFi.h>

#include "arduino_secrets.h"

// =============== Internet Connectivity (Wireless) ===
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int keyIndec = 0;
int status = WL_IDLE_STATUS;

WiFiClient client;

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

// ================ Data Publish ==========================
// - Variables to Define de HTTP Server
char server = {"www.httpbin.org"};
int port = 80;

// - HTTP GET Request
// This code has length limitation, if you want to avoid this limitation use HTTP POST instead.
void doHttpGet()
{
  // Prepare data or parameters that need to be posted to server
  String requestData = "requestVar = test";

  // Check if a connection to server:port was made
  if (client.connect(server, port))
  {
    Serial.println("[INFO] Server Coneected - HTTP GET Started");

    // Make HTTP GET request
    client.println("GET /get?" + requestData + "HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();
    Serial.println("[INFO] HTTP GET Completed");
  }
  else
  {
    Serial.println("[ERROR] Connection Failed");
  }

  Serial.println("--------------------------------------------------------------------");
}


// HTTP POST Request
void doHttpPost()
{
  // Prepare data or parameters that need to be posted to server
  String requestData = "requestData={\"requestVar:test\"}";

  // Check if a connection to a server:port was made
  if (client.connect(server, port))
  {
    Serial.println("[INFO] Server Connected - HTTP POST Started");

    // Make HTTP POST request
    client.println("POST /post HTTP/1.1");
    client.println("Host: " + String(server);
    client.println("User-Agent: Arduino nano 33 IoT");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.println("Content-Length: ");
    client.println(requestData.lenght());
    client.println();
    client.println(requestData);

    Serial.println("[INFO] HTTP POST Completed");
  }
  else
  {
    // Connection to server:port failed
    Serial.println("[ERROR] Connection Failed");
  }

  Serial.println("--------------------------------------------------------------");
}

// ================= Standar Functions ========================

void setup() {
  // Initialize serial port
  Serial.begin(9600);

  // Connect Arduino to internet
  connectToInternet();

  // Make HTTP GET request
  doHttpGet();

  // Maje HTTP POST request

}

void loop() {
  // put your main code here, to run repeatedly:

}
