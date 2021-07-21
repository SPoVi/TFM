/*
 * SUPERHOUSE AUTOMATION (http://www.superhouse.tv)
 * https://github.com/SuperHouse/AQS/blob/main/Firmware/AirQualitySensorD1Mini/AirQualitySensorD1Mini.ino
 * 
 * Ultima fecha de modificacion: 21/07/2021
 */

 /*
  * TODO:
  *       Introducir libraria time.h .Da igual que el setTime no sea exacto, lo quieres para medir tiempos.
  *       Revisar si se envian todos los paquetes. Elevar a QoS 2.
  *       
  *       Probar a cambiar a float las var que se quieren enviar a grafana. Sino cambiar mean to count.
  *       
  *       Ver si hay alguna forma de convertir decima a HEX u otro codig que reduzca el peso del paquete.
  */

  /*
   * MODIFICADO:
   * 
   *      Añadido g_topicSize
   */
   
   /*
    * OBSERVACIONES: 
    * 
    *     El TAMAÑO máximo de la cadena sera de 256 bytes (0-255). Esto implica que el tamaño de mysize maximo es de 255.
    */
#define VERSION "2.0"

/*------------------- Configuration -------------------------*/
// Configuration should be done in the included file:
#include "configuration.h";

/*------------------- Libraries -----------------------------*/
#include <WiFiNINA.h>
#include <PubSubClient.h>
/* 
 *  TODO : Buscar una forma de incluir libreria de tiempo.
 *  Necesita que lepases la hora. Utilizar datos subMqtt y  Node-red ??
 */

/*------------------ Global Variables -----------------------*/
// g_: for global

// SENSORS

#define g_mysize 360                        // Tamaño del vector de datos
byte g_data[g_mysize];                      // Vector de datos

bool g_GSR_readings_taken     = false;      // true/fasle: whether any readings have been taken
String g_GSR_value            = "";         // Value for sensor GSR
unsigned long g_GSR_time       = 0;         // Value for sensor GSR time
int g_pin_GSR                 = A0;         // Analog pin on NN33IoT
bool g_GSR_state_start        = 0;          // Timestamp when GSR state last changed


// MQTT
# define g_topicSize 255                          // Tamaño del string de los topics. Peso en bytes del paquete.
char g_mqtt_message_buffer[255];            // General purpose buffer for MQTT messages
char g_command_topic[g_topicSize];                   // MQTT topic for receiving commands

#if REPORT_MQTT_SEPARATE
char g_GSR_mqtt_topic[g_topicSize];                  // MQTT topic for reporting GSR values
char g_time_mqtt_topic[g_topicSize];                 // MQTT topic for reporting GRS time value
#endif

#if REPORT_MQTT_JSON
char g_mqtt_json_topic[g_topicSize];                 // MQTT topic for reporting all values using JSON
#endif

// WiFi
#define WIFI_CONNECT_INTERVAL       500     // Wait 500ms intervals for wifi connection
#define WIFI_CONNECT_MAX_ATTEMPTS   10      // Number of attempts/intervals to wait

// Time
//time_t g_time;                              // Variable tiempo

//General
char* g_cliente_ID;                        // Simulate chipID

unsigned long g_tStart;                     // Initial time for measuring time spent
unsigned long g_tEnd;                       // End time for measuring time spent


/*------------------- Instantiate Global Objects ----------------*/
// MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/*-------------------- Function Signatures -----------------------*/
void mqttCallback(char* topic, byte* payload, unsigned int len);    // Done
bool initWiFi();                                                    // Done
void reconnectMqtt();                                               // Done
void updateReadings();                                              // Done
void reportToMqtt();                                                // Done
void reportToSerial();                                              // Done
String NewClientID();                                               // Done
void showTimeNeeded();                                              // Done

/* ------------------ Program -------------------------------------*/
/*
 * SETUP
 */

void setup()
{
  delay(1000); // quitar
  
  // Initialize connection
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  Serial.print("Sensor GSR starting up, v");
  Serial.println(VERSION);

  // We need a unique device ID for our MQTT client connection
  // g_cliente_ID = NewClientID();
  g_cliente_ID = "IDSPV001";       // Get ID. Revisar TODO.
  

  /*------------------TOPICS-----------------------------------------------------------------------------------------*/
  // Set up the topics for publishing sensor readings. By inserting the unique ID,
  // the result is of the form: "arduino_1/GSR/valor" etc
  sprintf(g_command_topic, "cmnd/%s/COMMAND", g_cliente_ID);     // For receiving commands

  #if REPORT_MQTT_SEPARATE
    sprintf(g_GSR_mqtt_topic, "arduino_1/%s/GSR/valor", g_cliente_ID);    // Data form AN33IoT
    sprintf(g_time_mqtt_topic, "arduino_1/%s/GSR/tiempo", g_cliente_ID);    // Data form AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    sprintf(g_mqtt_json_topic, "arduino_1/%s/SENSOR", g_cliente_ID);   // Data from AN33IoT
  #endif

  // Report the MQTT topics to the serial console
  Serial.println(g_command_topic);        // For receiving messages
  #if REPORT_MQTT_SEPARATE
    Serial.println("MQTT topics: ");
    Serial.println(g_GSR_mqtt_topic);      // From AN33IoT
    Serial.println(g_time_mqtt_topic);     // From AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    Serial.println(g_mqtt_json_topic);    // From AN33IoT
  #endif

  // Connect to WiFi
  if (initWiFi())
  {
    Serial.print("WiFi connected");
    Serial.println(ssid);
    Serial.print("IP local address: ");
    Serial.println(WiFi.localIP());
  } 
  else
  {
    Serial.print("WiFi FAILED");
  }
  delay(100);

  /* Set up the MQTT client */
  mqttClient.setServer(mqtt_broker, port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(255);              // Tamaño del paquete que puede enviar MQTT por defecto 128

  delay(1000); // quitar
}

/*
 * ------------------- MAIN LOOP ---------------------
 */

void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttClient.connected())
    {
      reconnectMqtt();
    }
  }

  mqttClient.loop();           // Proces any outstanding MQTT messages

  updateReadings();
  

}

// FUNCTION - Generate client ID (chipID) ------------------------------------------------
/*
 *  TODO: Que las iniciales del paciente formen parte del ID. Que se intrduzca por teclado.
 *  Compatibilidad con base de datoso para la numeracion y que sea un ID único.
 *  
 *  Idea: utilizar fecha y hora de creacion como parametro ID
 */
String NewClientID()
{
  String clientID = "SPV001";                  // Iniciales + XXX
  //clientID += String(random(0xffff), HEX);   // Generate random ID. No queremos un random.
  return clientID;

  // Mostrar por pantalla
  Serial.println();
  Serial.print("ID del cliente: ");
  Serial.println(clientID);
}

// FUNCTION - Connect to WiFi ---------------------------------------------------------------
bool initWiFi()
{
   delay(10);

   Serial.println();
   Serial.print("Connecting to wifi ");
   Serial.println(ssid);

   // Conenecto to WiFi
   WiFi.begin(ssid, password);
   randomSeed(micros());              // Semilla aleatoria para micros()

   // Wait for connection ser amount of intervals
   int num_attempts = 0;
   while (WiFi.status() != WL_CONNECTED && num_attempts <= WIFI_CONNECT_MAX_ATTEMPTS)
   {
    delay(WIFI_CONNECT_INTERVAL);
    Serial.print(".");
    num_attempts++;
   }

   if (WiFi.status() != WL_CONNECTED)
   {
     return false;
   } 
   else
   {
     return true;
   }   
}

// PROCEDURE - Reconnect to MQTT broker, and publish a notifiacion to the status topic -------------------------------
void reconnectMqtt(){
  char mqtt_client_id[20];
  sprintf(mqtt_client_id, "Paciente-%s", g_cliente_ID); // Añade a mqtt_client_id donde %x lee valor dado g_cliente_ID

  // Loop until we're reconnected
  while(!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      // Once connected, publish announcement
      sprintf(g_mqtt_message_buffer, "Device %s starting up..", mqtt_client_id);
      mqttClient.publish(status_topic, g_mqtt_message_buffer);
      // Resubscribe
      // mqttclient.subscribe(g_command_topic);     // Activar cuando se requiera leer topic   
    }
    else // Omitir Serial para acelerar código
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrtying
      delay(5000);
    }
  }
  
}
// PROCEDURE - Callback : recibir mensaje de un topic ------------------------------------------------------
/*
 * This callback is invoked when a MQTT message is received. It's not important tright now
 * for  this project becasue we don't receive commands via MQTT. You can modify this function
 * to maje the device act on commands that you send it
 */
void mqttCallback(char* topic, byte* payload, unsigned int len) {
   Serial.print("Message arrived (topic)[");
   Serial.print(topic);
   Serial.print("] ");
   for (int i = 0; i < len; i++) {
     Serial.print((char)payload[i]);  // convierte a character el mensaje pasado como int
   }
   Serial.println();
}

// PROCEDURE - Update Readings ------------------------------------------------------------------------------
/*
 * TODO: Cambiar var tiempo a t_time
 */
void updateReadings(){
  unsigned long GSR_tStart;                       // Initial time for measuring time spent
  unsigned long GSR_tEnd;                       // Initial time for measuring time spent

   //Leemos los valores del sensor GSR
   GSR_tStart=micros();             // Inicio del tiempo para coger muestras. OJO MICROSEGUNDOS 10^-6
   for (int i=0; i<g_mysize;i++){
    g_data[i]=analogRead(g_pin_GSR);  // recogo 'mysize' numero muestras
     //time[i]=micros();       
   GSR_tEnd=micros();              // Fin del tiempo para coger muestras
   }

  // Mostrar tiempo necesitado para leer los datos por pantalla
  showTimeNeeded(GSR_tStart,GSR_tEnd);    
  g_GSR_readings_taken = true;


  // Creacion del paquete y Mostrar datos del vector enviados
  for (int i=0; i<g_mysize;i++){
    Serial.println(g_data[i],DEC);
    g_GSR_value.concat(g_data[i]);          // Creacion del paquete de datos 
    g_GSR_value.concat(" ");                // Añade espacio
  }

  // Report the new values
   reportToMqtt();
   reportToSerial();

   // Vaciar paquete
   g_GSR_value = "";
}

// PROCEDURE - Report the latest values to MQTT -------------------------------------------------------------
void reportToMqtt()
{
  String message_string;
  

  #if REPORT_MQTT_SEPARATE
    if (true == g_GSR_readings_taken)
    {
      /* Report GSR value */
      message_string = String(g_GSR_value);
      message_string.toCharArray(g_mqtt_message_buffer, message_string.length() + 1);
      mqttClient.publish(g_GSR_mqtt_topic, g_mqtt_message_buffer);
      // TODO: Coger tiempo ???
      g_GSR_time = millis();

      /* Report GSR time value */
      message_string = String(g_GSR_time);
      message_string.toCharArray(g_mqtt_message_buffer, message_string.length() + 1);
      mqttClient.publish(g_time_mqtt_topic, g_mqtt_message_buffer);
    }
   #endif

   #if REPORT_MQTT_JSON
    /* Report all values combined into one JSON message */
    // This is an example message generated by Tasmota, to match the format:
    // {"Time":"2020-02-27T03:27:22","PMS5003":{"CF1":0,"CF2.5":1,"CF10":1,"PM1":0,"PM2.5":1,"PM10":1,"PB0.3":0,"PB0.5":0,"PB1":0,"PB2.5":0,"PB5":0,"PB10":0}}
    //
    // This is the source code from Tasmota:
    //ResponseAppend_P(PSTR(",\"PMS5003\":{\"CF1\":%d,\"CF2.5\":%d,\"CF10\":%d,\"PM1\":%d,\"PM2.5\":%d,\"PM10\":%d,\"PB0.3\":%d,\"PB0.5\":%d,\"PB1\":%d,\"PB2.5\":%d,\"PB5\":%d,\"PB10\":%d}"),
    //    pms_g_data.pm10_standard, pms_data.pm25_standard, pms_data.pm100_standard,
    //    pms_data.pm10_env, pms_data.pm25_env, pms_data.pm100_env,
    //    pms_data.particles_03um, pms_data.particles_05um, pms_data.particles_10um, pms_data.particles_25um, pms_data.particles_50um, pms_data.particles_100um);

    // Note: The PubSubClient library limits MQTT message size to 128 bytes. The long format
    // message below only works because the message buffer size has been increased to 255 bytes
    // in setup.

      if (true == g_GSR_readings_taken)
      {
         // Convertir a array
         String str = g_GSR_value;
         char char_array[g_GSR_value.length()];
         str.toCharArray(char_array, g_GSR_value.length());  // Eliminado el +1 para elimnar el ultimo caracter separador 

         /* BORRRAAARRRR ----------------------------------------------------------"""""""""""""!!!!!!!!!!!!!
         
         Serial.println();
         Serial.print("VALOR DE CHAR ARRAY: ");
         Serial.println(char_array);
         Serial.print("VALOR DE g_GSR_value: ");
         Serial.println(g_GSR_value.length());
         Serial.println();
         
         delay(5000);

         //--------------------------------------------------------------------------------------------------
         */
         sprintf(g_mqtt_message_buffer,  "{\"ID:%s\":{\"GSR\":\"%s\",\"GSR Time\":%i}}",  // Es necesario poner %s entre "\%s"\ por ser un string o char 
              g_cliente_ID, char_array, g_GSR_time);
      }
      mqttClient.publish(g_mqtt_json_topic, g_mqtt_message_buffer);
    #endif
}

// PROCEDURE - Mostar tiempos por pantalla -------------------------------------------------------------------
/*
 * TODO : Cambiar a tipo t_time
 */
void showTimeNeeded(unsigned long t_start, unsigned long t_end)
{
  Serial.println("---TIEMPO NECESITADO--");
  Serial.print("Inicio del tiempo (tStart) = ");
  Serial.println(t_start);  // Incio del tiempo
  Serial.print("Fin del tiempo (tEnd) = ");
  Serial.println(t_end);    // Fin del tiempo
  Serial.print("Tarda (tEnd-tStart) = ");
  Serial.println(t_end-t_start); // Lo que ha tardado en recoger la informacion 
}

// PROCEDURE - Mostar por pantalla --------------------------------------------------------------------------
void reportToSerial()


{
  if (true == g_GSR_readings_taken)
  {

    int peso = 0;



    /* Paciente*/
    Serial.print("Paciente: ");
    Serial.println(g_cliente_ID);
    /* Report GSR value*/
    
    Serial.print(g_GSR_mqtt_topic);
    Serial.print(" topic is GSR: ");
    Serial.println(String(g_GSR_value));

    /* Report GSR time value*/
    Serial.print("Time: ");
    Serial.println(String(g_GSR_time));

    /* Peso del paquete*/
    Serial.print("Peso del paquete GSR value: ");
    peso = sizeof(g_GSR_mqtt_topic);
    Serial.print(peso);
    Serial.println(" bytes.");
    
    delay(5000);
   
  }
  Serial.println();
}
