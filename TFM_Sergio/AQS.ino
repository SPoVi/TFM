/*
 * SUPERHOUSE AUTOMATION (http://www.superhouse.tv)
 * https://github.com/SuperHouse/AQS/blob/main/Firmware/AirQualitySensorD1Mini/AirQualitySensorD1Mini.ino
 *
 */
#define VERSION "1.0"

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
bool g_GSR_readings_taken     = false;      // true/fasle: whether any readings have been taken
String g_GSR_value            = "";         // Value for sensor GSR
unsigned long g_GSR_time      = 0;          // Value for sensor GSR time
int g_pin_GSR                 = A0;         // Analog pin on NN33IoT
bool g_GSR_state_start        = 0;          // Timestamp when GSR state last changed


// MQTT
char g_mqtt_message_buffer[255];            // General purpose buffer for MQTT messages
char g_command_topic[50];                   // MQTT topic for receiving commands

#if REPORT_MQTT_SEPARATE
char g_GSR_mqtt_topic[50];                  // MQTT topic for reporting GSR values
char g_time_mqtt_topic[50];                 // MQTT topic for reporting GRS time value
#endif

#if REPORT_MQTT_JSON
char g_mqtt_json_topic[50];                 // MQTT topic for reporting all values using JSON
#endif

// WiFi
#define WIFI_CONNECT_INTERVAL       500     // Wait 500ms intervals for wifi connection
#define WIFI_CONNECT_MAX_ATTEMPTS   10      // Number of attempts/intervals to wait

// Time
//time_t g_time;                              // Variable tiempo

//General
String g_cliente_ID;                        // Simulate chipID

unsigned long g_tStart;                     // Initial time for measuring time spent
unsigned long g_tEnd;                       // End time for measuring time spent

String g_paquete = "";                      // Paquete de datos MQTT
#define g_mysize 10                         // Tamaño del vector de datos
byte g_data[g_mysize];                      // Vector de datos

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
  // Initialize connection
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  Serial.print("Sensor GSR starting up, v");
  Serial.println(VERSION);

  // We need a unique device ID for our MQTT client connection
  g_cliente_ID = NewClientID();       // Get ID. Revisar TODO.

  // Set up the topics for publishing sensor readings. By inserting the unique ID,
  // the result is of the form: "arduino_1/GSR/valor" etc
  sprintf(g_command_topic, "cmnd/%/COMMAND", g_cliente_ID);     // For receiving commands

  #if REPORT_MQTT_SEPARATE
    sprintf(g_GSR_mqtt_topic, "arduino_1/GSR/valor", g_cliente_ID);    // Data form AN33IoT
    sprintf(g_time_mqtt_topic, "arduino_1/GSR/tiempo", g_cliente_ID);    // Data form AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    sprintf(g_mqtt_json_topic, "arduino_1/%x/SENSOR", g_cliente_ID);   // Data from AN33IoT
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
  mqttClient.setBufferSize(255);              // ???????????????

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

// FUNCTION - Generate client ID (chipID)
/*
 *  TODO: Que las iniciales del paciente formen parte del ID. Que se intrduzca por teclado.
 *  Compatibilidad con base de datoso para la numeracion y que sea un ID único.
 *
 *  Idea: utilizar fecha y hora de creacion como parametro ID
 */
String NewClientID()
{
  String clientID = "IDSPV001";                // ID + Iniciales + XXX
  //clientID += String(random(0xffff), HEX);   // Generate random ID. No queremos un random.
  return clientID;

  // Mostrar por pantalla
  Serial.println();
  Serial.print("ID del cliente: ");
  Serial.println(clientID);
}

// FUNCTION - Connect to WiFi
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
  sprintf(mqtt_client_id, "Paciente-%x", g_cliente_ID); // Añade a mqtt_client_id donde %x lee valor dado g_cliente_ID

  // Loop until we're reconnected
  while(!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      // Once connected, publish announcement
      sprintf(g_mqtt_message_buffer, "Device %s starting up", mqtt_client_id);
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
    g_paquete.concat(g_data[i]);          // Creacion del paquete de datos
    //g_paquete.concat(" ");
  }

  g_GSR_value = g_paquete;

  // Report the new values
   reportToMqtt();
   reportToSerial();

   // Vaciar paquete
   g_paquete = "";
}

// PROCEDURE - Report the latest values to MQTT -------------------------------------------------------------
void reportToMqtt()
{
  String message_string;
  unsigned long int gsr_time;

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
         sprintf(g_mqtt_message_buffer,  "{\"IDSPV001\":{\"GSR\":%i,\"GSR Time\":%i,\"CF10\":%i}}",
              g_GSR_value, String(g_GSR_time));
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
    /* Report GSR value*/
    Serial.print("GSR: ");
    Serial.println(String(g_paquete));

    /* Report GSR time value*/
    Serial.print("Time: ");
    Serial.print(String(g_GSR_time));

  }
}

