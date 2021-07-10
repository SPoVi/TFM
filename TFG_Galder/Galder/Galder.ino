//Librerias para garantizar el funcionamiento del Wifi y el 
funcionamiento del servidor MQTT
#include <WiFiNINA.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

//Definición de la red Wifi

const char* ssid = "Nombre de la red";
const char* password = "Contraseña";
const char* mqttServer = ""; // Dirección ip del servidor

//Definir las variables que vamos a utilizar.
int sensorGSR;
char v[10];//Vector para hacer el cambio de variable


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
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < length; i++) {
     Serial.print((char)payload[i]);
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
     if (client.connect(clientId.c_str())) {
       Serial.println("conectado");
       // Una vez conectado envíamos un mensaje de que nos hemos 
conectado a los 3 temas en cuestion
       client.publish("tema", "CONECTADO:");
       // ... and resubscribe
       client.subscribe("tema");//Tema al que nos vamos a subscribir 
para enviar la información
     } else {
       Serial.print("failed, rc=");
       Serial.print(client.state());
       Serial.println(" vuelve a intentar en 0.5s");
       // Esperar 0.5 segundos antes de volver a intentar conectar
       delay(500);
     }
   }
}

void setup() {
   // put your setup code here, to run once:
   //Velocidad de los baudios del puerto serial
   Serial.begin(velocidad deseada);

   //Inicializamos la conexion wifi y la conexion con el servidor
   setup_wifi();
   client.setServer(mqttServer, 1883);
   client.setCallback(callback);


}


void loop() {
   // put your main code here, to run repeatedly:
   // Comprobar la conexion al servidor. EN caso de no estar conectado 
ir a la funcion reconnect y hasta que no se conecta no volver al 
codigo principal
   if (!client.connected())
   {
     reconnect();
   }
   client.loop();

   //Leemos los valores del sensor GSR

    sensorGSR=analogRead(pin al que se ha conectado);

//Pasamos los numeros a un vector de char para poder enviarlos al 
servidor para esto se usa el comando itoa();


client.publish("nombredeltema",vectordondeestanlosdatos);// Enviamos 
el dato al tema en cuestión
