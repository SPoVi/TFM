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
void reconnect(clientId, pubTopic, subTopic) {
// Bucle hasta que nos conectamos
   while (!mqttClient.connected()) {
     Serial.println("Attempting MQTT connection...");


     // Intento de conexión del clienteId creado aleatoriamente
     if (mqttClient.connect(clientId.c_str())) { // Convierte el contenido de una cadena en una cadena terminada en cero al estilo de C
       Serial.println("conectado");
       // Una vez conectado envíamos un mensaje de que nos hemos conectado a los 3 temas en cuestion
       mqttClient.publish(pubTopic, "CONECTADO:");

       // ... and resubscribe
       mqttClient.subscribe(subTopic);                  //Tema al que nos vamos a subscribir para enviar la información
       //mqttClient.subscribe("t2");
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

// procedure - SUBSCRIPCION a topic
void subMQTTT(String clienteId, String topic, int qos){

  // Si la conexion es exitosa
  if(mqttClient.connect(clientId.c_str())){
      Serial.print("Cliente (ID): ");
      Serial.print(clienteId);
      Serial.println(" CONECTADO");

      boolean ret = mqttClient.subscribe(topic,qos); // return 0 ó 1
      if (ret == 0){
        Serial.println("La solicitud de subscripcion ha fallado la conexion se ha perdido o el mensaje es demasiado largo");
      } else if(ret == 1){
        Serial.println("Peticion de subscripcion realidada con exito");
      } else {
        Serial.println("Return no valido. Revisa codigo !!");
      }
  } else {
    Serial.println("Conexion FALLIDA");
    // mqttClient.stat() will provide more information
    server_state();
  }
}


// Procedure - Callback : recivir mensajes de un topic
void callback_function(char* topic, byte* payload, unsigned int len) {
   Serial.print("Message arrived (topic)[");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < len; i++) {
     Serial.print((char)payload[i]);  // convierte a character el mensaje pasado como int
   }
   Serial.println();
}


// FUNCION - ESTADO DEL SERVIDOR
void server_state(){
  int estado = mqttClient.state();

  switch (estado) {
    case -4:
      Serial.println("El servidor no ha respondido dentro del tiempo mantenervivo");
      break;

    case -3:
      Serial.println("La conexion con la red estaba rota");
      break;

    case -2:
      Serial.println("La conexion con al red ha fallado");
      break;

    case -1:
      Serial.println("El cliente se ha desconectado limpiamente");
      break;

    case 0:
      Serial.println("El cliente está conectado");
      break;

    case 1:
      Serial.println("El servidor no soporta la versión solicidata de MQTT");
      break;

    case 2:
      Serial.println("El servidor ha rechazado el identificador de cliente");
      break;

    case 3:
      Serial.println("El servidor no fue capaz de aceptar la conexion");
      break;

    case 4:
      Serial.println("El nombre de usuario/contraseña  fueron rechazados");
      break;

    case 5:
      Serial.println("El cliente no fue autorizado a conectarse");
      break;
  }


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
   mqttClient.setCallback(callback_function);
   // El cliente está ahora listo para su uso

   // New random client ID
     String clientId = NewClientId();
     Serial.print("Client id : ");
     Serial.println(clientId);
}


// ********************************** MAIN LOOP ********************************************
void loop() {

   // Si se produce un error en la conexion llamar a reconnect
   if (!mqttClient.connected()){
     reconnect(clientId);
   }

   //Leemos los valores del sensor GSR
   tStart=micros();             // Inicio del tiempo para coger muestras. OJO MICROSEGUNDOS 10^-6
   for (int i=0; i<mysize;i++){
    data[i]=analogRead(pinGSR);  // recogo 'mysize' numero muestras
     //time[i]=micros();
   tEnd=micros();              // Fin del tiempo para coger muestras
   }


  // Creacion del paquete y Mostrar datos del vector enviados
  for (int i=0; i<mysize;i++){
    Serial.println(data[i],DEC);
    paquete.concat(data[i]);          // Creacion del paquete de datos
    //paquete.concat(" ");
  }

  pubMQTTString(topic1,paquete); // Prublicacion String
  pubMQTT(topic2,tEnd);          // Publicacion long

  paquete = ""; // Vacía el paquete

  /*
   * This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
   * Returns
   *
   *    false - the client is no longer connected
   *    true - the client is still connected
   */
  mqttClient.loop();
}