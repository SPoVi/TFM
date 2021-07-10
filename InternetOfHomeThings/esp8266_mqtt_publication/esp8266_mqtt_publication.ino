/*
 * ESP8266 MQTT Publication
 * 
 * https://internetofhomethings.com/homethings/?p=1121
 */
// Archivo contenedor de los datos de la red wifi 
#include "arduino_secrets.h"

//#include <ESP8266WiFi.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>


const char* wifi_ssid = SECRET_SSID;        // Nombre de la red WiFi
const char* wifi_password = SECRET_PASS;    // Contraseña de la red WiFi

#define mqtt_server "test.mosquitto.org"
#define mqtt_user "your_username"
#define mqtt_password "your_password"

#define topic1 "t1"
#define topic2 "t2"

WiFiClient espClient;            // Creacion de objeto 
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);                
  setup_wifi();                        // Conectar a wifi
  client.setServer(mqtt_server, 1883); // Servidor y puerto 
 }

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
   
  Serial.println("");
  Serial.print("WiFi connected:");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 }


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("TestMQTT")) { //* See //NOTE below
      Serial.println("connected");
      
    }else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 1 seconds before retrying
      delay(2000);
     }
   }
 }


//NOTE: if a user/password is used for MQTT connection use:
//if(client.connect("TestMQTT", mqtt_user, mqtt_password)) {

// Procedure - Publicar mensaje
void pubMQTT(String topic,float topic_val){
  Serial.print("Newest topic " + topic + " value:");
  Serial.println(String(topic_val).c_str());
  client.publish(topic.c_str(), String(topic_val).c_str(), true);
 }

//Variables used in loop()
long lastMsg = 0;
float t1 = 75.5;
float t2 = 50.5;

void loop() {

  // Comprobar conexion al servidor.
  if (!client.connected()) {
    reconnect();
    }
  
  client.loop();
  //2 seconds minimum between Read Sensors and Publish
  long now = millis();
  if (now - lastMsg > 2000){
    lastMsg = now;
    //Read Sensors (simulate by increasing the values, range:0-90)
    if (t1 > 90){
     t1 = 0;
   }else{
      t1++;
    }
    
    if(t2 > 90){
     t2 = 0;
   }else{
     t2++;
    }
    //Publish Values to MQTT broker
    pubMQTT(topic1,t1);
    pubMQTT(topic2,t2);
  }
 }
