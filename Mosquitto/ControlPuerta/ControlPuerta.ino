/*
  Arduino MQTT Publisher - Puerta de casa abierta

  This example connects to a MQTT broker and publishes a message to
  a topic once a second.

  The circuit:
  - Arduino MKR 1000, MKR 1010, nano 33 IoT or Uno WiFi Rev.2 board

  This example code is in the public domain.
*/
  
// Decomentar para mostrar debug monitor serie
#define DEBUG_ALARMA
 
//Librerías
#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#endif

#include "arduino_secrets.h"

// Pines
const byte intPuerta = 21;  // Pin
 
// Variables interrupción de interruptor
volatile boolean isrAlarmaCentinela = false; // Detectar interrupción
 
// Configuración WiFi
char ssid[] = SECRET_SSID; // SSID de la WiFi
char pass[] = SECRET_PASS; // Constraseña de la WiFi
int status = WL_IDLE_STATUS;
WiFiClient  clienteWifi;
 
// Configuración MQTT
MqttClient clientMqtt(clienteWifi);
const char* servidorMqtt = "192.168.1.45"; //"IP-BROKER-MQTT"
const char* topicAlarma  = "/casa/puerta/alarma";
int         port         = 1883;
 
void setup() {
 
#ifdef DEBUG_ALARMA
  Serial.begin(9600);
  delay(500);
  Serial.println("[INI]Comienzo del programa sistema alarma con Arduino MKR1000");
 
  // Señal arrancando
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
#endif
 
  // Conexión con la red WiFi
  int intentosWiFi = 0;
  while (status != WL_CONNECTED || intentosWiFi > 5) {
#ifdef DEBUG_ALARMA
    Serial.print("[WIFI]Intentando conectar a: ");
    Serial.println(ssid);
#endif
 
    // Conectando a la red WiFi WPA/WPA2
    status = WiFi.begin(ssid, pass);
 
    // Esperamos 10 segundos para conectarnos
    delay(10000);
 
    intentosWiFi++;
  }
 
  // Si no ha conetado mostramos error
  if (status != WL_CONNECTED)
  {
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }
 
#ifdef DEBUG_ALARMA
  Serial.println("[WIFI]Conectado a la WiFi");
#endif
 
  // Configuración MQTT (broker)
  if (!clientMqtt.connect(servidorMqtt, port)) {  
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(clientMqtt.connectError());

    while (1);
  }
#ifdef DEBUG_ALARMA
  Serial.println("[MQTT]Conectado al servidor MQTT");
#endif
 
  // Configuración de interrupciones
  pinMode(intPuerta, INPUT_PULLUP); // Interrupción interruptor magnético
  attachInterrupt(intPuerta, isrAlarma, FALLING); //Interrupción interruptor magnético
 
  // Apagado de LED
  digitalWrite(LED_BUILTIN, LOW);
}
 
void loop()
{
  // Comprobamos conexión con broker MQTT
  if (!clientMqtt.connected()) {
    reconnectMqtt();
  }
  clientMqtt.poll();
 
  // Si detecta interrupción alarma
  if (isrAlarmaCentinela) {
 
    // Marcamos para que no vuelva a pasar
    isrAlarmaCentinela = false;
 
    // Enviamos mensaje
    clientMqtt.beginMessage(topicAlarma);
    clientMqtt.print("1");
    clientMqtt.endMessage();
 
#ifdef DEBUG_ALARMA
    Serial.println("[MQTT]Publicando mensaje alarma: Puerta abierta");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
#endif
  }
}
 
void isrAlarma() {
  // Marca ha ejecutado interrupción
  isrAlarmaCentinela = true;
}
 
void reconnectMqtt() {
  // Repetimos hasta conectar
  while (!clientMqtt.connected()) {
#ifdef DEBUG_ALARMA
    Serial.println("[MQTT]Esperando conexión con MQTT...");
#endif
    // Intentamos conectar
    if (clientMqtt.connect("ALARMA-PUERTA-MKR1000")) {
#ifdef DEBUG_ALARMA
      Serial.println("[MQTT]Conectado");
#endif
    } else {
#ifdef DEBUG_ALARMA
      Serial.print("[MQTT]Fallo, rc=");
      //Serial.print(clientMqtt.state());
      Serial.println(" se intentará o travez tras 5 segundos");
#endif
      // Esperamos 5 segundos
      delay(5000);
    }
  }
}
