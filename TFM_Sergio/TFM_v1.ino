// Librerias para garantizar el funcionamiento del Wifi y el
// funcionamiento del servidor MQTT
#include <WiFiNINA.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); // Instancia de Cliente parcialmente inicializado

// Archivo contenedor de los datos de la red wifi
#include "arduino_secrets.h"

//Definición de la red Wifi

const char* ssid = SECRET_SSID;                   // Nombre de la red WiFi
const char* password = SECRET_PASS;               // Contraseña de la red WiFi
const char* mqttServer = "test.mosquitto.org";    // Dirección ip del servidor
//const char* mqttServer = "192.168.85.128";

// Definir topics
#define topic1 "t1"
#define topic2 "t2"

//Definir las variables que vamos a utilizar.
int sensorGSR;

long velocidad_deseada = 115200; // baudios
int port = 1883;
int pinGSR = A0;                 // Pin de conexion GSR (A0) D14

#define mysize 10                // tamaño del vector de datos (paquete)
byte data[mysize];

// Medir tiempos
unsigned long tStart;
unsigned long tEnd;

// Paquete datos MQTT
String paquete = "";
unsigned long tpaquete;

// Tiempo real
// time_t Tstart = now();

// Procedure - Establecemos la conexión Wifi
void setup_wifi()
{
   delay(10);

   Serial.println();
   Serial.print("Connecting to wifi ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED)
   {
     delay(500);
     Serial.print(".");
   }

   randomSeed(micros());   // Semilla aleatoria para micros()

   Serial.println("");
   Serial.print("WiFi connected: ");
   Serial.println(ssid);
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
}

// Funcion String - Crear nuevo cliente
String NewClientId(){
     String clientId = "identificacóndelcliente";
     clientId += String(random(0xffff), HEX);
     return clientId;
}

// En caso de perder la conexion reconectar
void reconnect() {
// Bucle hasta que nos conectamos
   while (!mqttClient.connected()) {
     Serial.println("Attempting MQTT connection...");

     // New random client ID
     String clientId = NewClientId();
     Serial.print("Client id : ");
     Serial.println(clientId);


     // Intento de conexión del clienteId creado aleatoriamente
     if (mqttClient.connect(clientId.c_str())) { // Convierte el contenido de una cadena en una cadena terminada en cero al estilo de C
       Serial.println("conectado");

       // Una vez conectado envíamos un mensaje de que nos hemos conectado a los temas en cuestion
       // bolean publish (topic, payload, [length],[reatined]
       mqttClient.publish("t1", "CONECTADO:");

       //Tema al que nos vamos a subscribir para enviar la información
       mqttClient.subscribe("t1");

     } else {
       Serial.print("failed, rc=");
       Serial.print(mqttClient.state());
       Serial.println(" vuelve a intentar en 0.5s");
       // Esperar 0.5 segundos antes de volver a intentar conectar
       delay(500);
     }
   }
}


// Procedure - PUBBLICACION al topic
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

// Procedure - Callback : respuesta a la SUBSCRUPCION de un topic
void callback(char* topic, byte* payload, unsigned int len) {
   Serial.print("Message arrived (topic)[");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < len; i++) {
     Serial.print((char)payload[i]);  // convierte a character el mensaje pasado como int
   }
   Serial.println();
}

//  ************************************ MAIN SETUP *****************************************
void setup() {

   //Velocidad de los baudios del puerto serial
   Serial.begin(velocidad_deseada); //baudios

   //Inicializamos la conexion wifi y la conexion con el servidor
   setup_wifi();

   /*
    * Los metodos de mqttClient es como su hubieramos puesto al principio
    * PubSubClient client(server, 1883, callback, ethClient);
    * PubSubClient client(mqttServer, port, callback, wifiClient);
    */
   mqttClient.setServer(mqttServer, port);
   mqttClient.setCallback(callback);
   // El cliente está ahora listo para su uso


}


// ********************************** MAIN LOOP ********************************************
void loop() {

   // Probar conexion a servidor. Si se produce un error en la conexion llamar a reconnect
   if (!mqttClient.connected()){
     reconnect();
   }

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

  //Publish Values to MQTT broker
  tpaquete = millis();
  pubMQTTString(topic1,paquete); // Prublicacion String


  pubMQTT(topic2,tEnd);          // Publicacion long

  paquete = ""; // Vacía el paquete

  mqttClient.loop();
}