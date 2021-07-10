
  
// Decomentar para mostrar debug monitor serie
#define DEBUG_ALARMA
 
//Librerías
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
 
// Pines
const byte intPuerta = 2;
 
// Variables interrupción de interruptor
volatile boolean isrAlarmaCentinela = false; // Detectar interrupción
 
// Configuración WiFi
char* ssid = "SSID-WIFI"; // SSID de la WiFi
char* pass = "PASS-WIFI"; // Constraseña de la WiFi
WiFiClient  clienteWifi;
 
// Configuración MQTT
PubSubClient clientMqtt(clienteWifi);
const char* servidorMqtt = "IP-BROKER-MQTT";
const char* topicAlarma = "/casa/puerta/alarma";
 
void setup() {
 
#ifdef DEBUG_ALARMA
  Serial.begin(115200);
  delay(500);
  Serial.println("[INI]Comienzo del programa sistema alarma con ESP8266");
 
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
  WiFi.begin(ssid, pass);
 
#ifdef DEBUG_ALARMA
    Serial.print("[WIFI]Intentando conectar a: ");
    Serial.println(ssid);
#endif
 
  while (WiFi.status() != WL_CONNECTED || intentosWiFi > 30) {
#ifdef DEBUG_ALARMA
    Serial.print(".");
#endif
 
    // Esperamos
    delay(500);
 
    intentosWiFi++;
  }
 
#ifdef DEBUG_ALARMA
    Serial.println(".");
#endif
 
  // Si no ha conetado mostramos error
 if (WiFi.status() != WL_CONNECTED)
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
 
  // Configuración MQTT
  clientMqtt.setServer(servidorMqtt, 1883);
#ifdef DEBUG_ALARMA
  Serial.println("[MQTT]Conectado al servidor MQTT");
#endif
 
  // Configuración de interrupciones
  pinMode(intPuerta, INPUT_PULLUP); // Interrupción interruptor magnético
  attachInterrupt(intPuerta, isrAlarma, FALLING); //Interrupción interruptor magnético
 
  // Apagado de LED
  digitalWrite(LED_BUILTIN, HIGH);
}
 
void loop()
{
  // Comprobamos conexión con broker MQTT
  if (!clientMqtt.connected()) {
    reconnectMqtt();
  }
  clientMqtt.loop();
 
  // Si detecta interrupción alarma
  if (isrAlarmaCentinela) {
 
    // Marcamos para que no vuelva a pasar
    isrAlarmaCentinela = false;
 
    // Enviamos mensaje
    clientMqtt.publish(topicAlarma, "1");
 
#ifdef DEBUG_ALARMA
    Serial.println("[MQTT]Publicando mensaje alarma: Puerta abierta");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
#endif
  }
}
 
ICACHE_RAM_ATTR void isrAlarma() {
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
      Serial.print(clientMqtt.state());
      Serial.println(" se intentará o travez tras 5 segundos");
#endif
      // Esperamos 5 segundos
      delay(5000);
    }
  }
}
