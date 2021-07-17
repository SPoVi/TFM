/ Librerias para garantizar el funcionamiento del Wifi y el
// funcionamiento del servidor MQTT
#include <WiFiNINA.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Archivo contenedor de los datos de la red wifi
#include "arduino_secrets.h"

//Definición de la red Wifi


const char* ssid = SECRET_SSID;                   // Nombre de la red WiFi
const char* password = SECRET_PASS;               // Contraseña de la red WiFi
const char* mqttServer = RPI_1;    // Dirección ip del servidor ej: test.mosquitto.org

#define topic1 "t1"
#define topic2 "t2"

//Definir las variables que vamos a utilizar.
int sensorGSR;

long velocidad_deseada = 115200; // baudios
int port = 1883;
int pinGSR = A0; // Pin de conexion GSR (A0) D14

#define mysize 50 // tamaño del vector de datos (paquete)
byte data[mysize];

// Medir tiempos
unsigned long tStart;
unsigned long tEnd;


// Establecemos la conexión Wifi
void connectToWiFi(){
  // Connect to WiFi Network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to Wifi");
  Serial.println("...");
  Serial.println(ssid);

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
    Serial.println("WiFi connected!");
    Serial.println("Local IP address: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("Setup ready");
}

// Mensaje de SUBSCRUPCION AL TOPIC
void callback(char* topic, byte* payload, unsigned int len) {
   Serial.print("Message arrived (topic)[");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < len; i++) {
     Serial.print((char)payload[i]);  // convierte a character el mensaje pasado como int
   }
   Serial.println();
}

// En caso de perder la conexion reconectar
void reconnect() {
// Bucle hasta que nos conectamos
   while (!mqttClient.connected()) {
     Serial.print("Attempting MQTT connection...");

     // Create a random client ID
     String clientId = "identificacóndelcliente";
     clientId += String(random(0xffff), HEX);


     // Intento de conexión
     if (mqttClient.connect(clientId.c_str())) { // Convierte el contenido de una cadena en una cadena terminada en cero al estilo de C
       Serial.println("conectado");
       // Una vez conectado envíamos un mensaje de que nos hemos conectado a los 3 temas en cuestion
       mqttClient.publish("tema", "CONECTADO:");
       // ... and resubscribe
       mqttClient.subscribe("t1");//Tema al que nos vamos a subscribir para enviar la información
     } else {
       Serial.print("failed, rc=");
       Serial.print(mqttClient.state());
       Serial.println(" vuelve a intentar en 0.5s");
       // Esperar 0.5 segundos antes de volver a intentar conectar
       delay(500);
     }
   }
}


// Procedure - Publicar mensaje
void pubMQTT(String topic,float topic_val){
  Serial.print("Newest topic " + topic + " value:");
  Serial.println(String(topic_val).c_str());
  mqttClient.publish(topic.c_str(), String(topic_val).c_str(), true);
 }

 void pubMQTTString(String topic,String topic_val){
  Serial.print("Newest topic " + topic + " value:");
  Serial.println(String(topic_val).c_str());
  mqttClient.publish(topic.c_str(), String(topic_val).c_str(), true);
 }



void setup() {

   //Velocidad de los baudios del puerto serial
   Serial.begin(velocidad_deseada); //baudios

   //Inicializamos la conexion wifi y la conexion con el servidor
   connectToWiFi();

   mqttClient.setServer(mqttServer, port);
   mqttClient.setCallback(callback);  // ????? Subscripcion al topic
}

String paquete = "";
unsigned long tpaquete;

void loop() {

   // Comprobar la conexion al servidor.
   // En el caso de no estar conectado ir a la funcion reconnect y hasta que no se conecta no volver
   // al codigo principal
   if (!mqttClient.connected()){
     reconnect();
   }
   mqttClient.loop();

   //Leemos los valores del sensor GSR
   tStart=micros();             // Inicio del tiempo para coger muestras. OJO MICROSEGUNDOS 10^-6
   for (int i=0; i<mysize;i++){
    data[i]=analogRead(pinGSR);  // recogo 'mysize' numero muestras
     //time[i]=micros();
   tEnd=micros();              // Fin del tiempo para coger muestras
   }

  // Mostrar por pantalla
  Serial.println();
  Serial.println("NEW ACQUISITION");
  Serial.print("Inicio del tiempo (tStart) = ");
  Serial.println(tStart);  // Incio del tiempo
  Serial.print("Fin del tiempo (tEnd) = ");
  Serial.println(tEnd);    // Fin del tiempo
  Serial.print("Tarda (tEnd-tStart) = ");
  Serial.println(tEnd-tStart); // Lo que ha tardado en recoger la informacion
  Serial.print("Tamaño del vector de datos = ");
  Serial.println(mysize);     // Tamaño de la muestra
  Serial.println("Datos enviados: ");

  // Creacion del paquete y Mostrar datos del vector enviados
  for (int i=0; i<mysize;i++){
    Serial.println(data[i],DEC);
    paquete.concat(data[i]);          // Creacion del paquete de datos
    //paquete.concat(" ");
  }

  pubMQTTString(topic1,paquete);
  pubMQTT(topic2,tEnd);

  paquete = ""; // Vacía el paquete
}