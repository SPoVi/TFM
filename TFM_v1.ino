// Librerias para garantizar el funcionamiento del Wifi y el 
// funcionamiento del servidor MQTT
#include <WiFiNINA.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Archivo contenedor de los datos de la red wifi 
#include "arduino_secrets.h"

//Definición de la red Wifi

const char* ssid = SECRET_SSID;                   // Nombre de la red WiFi
const char* password = SECRET_PASS;               // Contraseña de la red WiFi
const char* mqttServer = "test.mosquitto.org";    // Dirección ip del servidor

#define topic1 "t1"
#define topic2 "t2"

//Definir las variables que vamos a utilizar.
int sensorGSR;

long velocidad_deseada = 115200; // baudios
int port = 1883;
int pinGSR = A0; // Pin de conexion GSR (A0) D14

#define mysize 50

unsigned long tStart;
unsigned long tEnd;
byte data[mysize];

// Establecemos la conexión Wifi
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

   randomSeed(micros());

   Serial.println("");
   Serial.print("WiFi connected: ");
   Serial.println(ssid);
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
}

// Mensaje de respuesta de Node - RED ?? SUBSCRUPCION AL TOPIC ??
void subMQTT(char* topic, byte* payload, unsigned int len) {
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
   while (!client.connected()) {
     Serial.print("Attempting MQTT connection...");
     
     // Create a random client ID
     String clientId = "identificacóndelcliente";
     clientId += String(random(0xffff), HEX);

     
     // Intento de conexión
     if (client.connect(clientId.c_str())) { // Convierte el contenido de una cadena en una cadena terminada en cero al estilo de C
       Serial.println("conectado");
       // Una vez conectado envíamos un mensaje de que nos hemos conectado a los 3 temas en cuestion
       client.publish("tema", "CONECTADO:");
       // ... and resubscribe
       client.subscribe("t1");//Tema al que nos vamos a subscribir para enviar la información
     } else {
       Serial.print("failed, rc=");
       Serial.print(client.state());
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
  client.publish(topic.c_str(), String(topic_val).c_str(), true);
 }

 void pubMQTTString(String topic,String topic_val){
  Serial.print("Newest topic " + topic + " value:");
  Serial.println(String(topic_val).c_str());
  client.publish(topic.c_str(), String(topic_val).c_str(), true);
 }



void setup() {
   
   //Velocidad de los baudios del puerto serial
   Serial.begin(velocidad_deseada); //baudios

   //Inicializamos la conexion wifi y la conexion con el servidor
   setup_wifi();
   client.setServer(mqttServer, port);
   client.setCallback(subMQTT);  // ????? Subscripcion al topic
}

String paquete = "";
unsigned long tpaquete;

void loop() {
   
   // Comprobar la conexion al servidor. 
   // En el caso de no estar conectado ir a la funcion reconnect y hasta que no se conecta no volver 
   // al codigo principal
   if (!client.connected()){
     reconnect();
   }
   client.loop();

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


 
  // Pasamos los numeros a un vector de char para poder enviarlos al servidor para esto se usa el 
  // comando itoa();
  //Publish Values to MQTT broker
  //Serial.println(paquete);
  //paquete.toFloat();
  //pubMQTT(topic1,paquete);
  tpaquete = millis();
  pubMQTTString(topic1,paquete);
  pubMQTTString(topic2,tEnd);

  paquete = ""; // Vacía el paquete
}
