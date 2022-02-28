/*
 * Autor: Sergio Pons Villanueva
 * Ultima fecha de modificacion: 20/08/2021
 */


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

int g_data_buffer[100];                       // Vector de datos

bool g_GSR_readings_taken     = false;      // true/fasle: whether any readings have been taken
int g_GSR_value               = 0;          // Value for sensor GSR
int g_pin_GSR                 = A0;         // Analog pin on NN33IoT
float g_GSR_time_sep          = 0.0;        // Tiempo entre muestra y muestra


// MQTT
# define g_topicSize 100                    // Tamaño del string de los topics. Peso en bytes del paquete. 
char g_mqtt_message_buffer[g_topicSize*5];        // General purpose buffer for MQTT messages
char g_command_topic[g_topicSize];                // MQTT topic for receiving commands

#if REPORT_MQTT_SEPARATE
char g_GSR_mqtt_topic[g_topicSize];               // MQTT topic for reporting GSR values
char g_npaquete_mqtt_topic[g_topicSize];          // MQTT topic for reporting GRS time value
#endif

#if REPORT_MQTT_JSON
char g_mqtt_json_topic[g_topicSize];              // MQTT topic for reporting all values using JSON
#endif

unsigned long g_npaquete       = 0;               // Value for number of pacakge sent
//int g_npaquete       = 0;               // Value for number of pacakge sent

// WiFi
#define WIFI_CONNECT_INTERVAL       500     // Wait 500ms intervals for wifi connection
#define WIFI_CONNECT_MAX_ATTEMPTS   10      // Number of attempts/intervals to wait

// Time
unsigned long g_segundo          = 0;        // Variable tiempo
unsigned long g_tiempo_inicio    = 0;
unsigned long g_tiempo_actual    = 0;


//General
char* g_cliente_ID;                        // Simulate chipID. *: NO CAMBIA 

unsigned long g_tStart_loop  = 0;          // Initial time for measuring time spent
unsigned long g_tEnd_loop    = 0;          // End time for measuring time spent

// Contador
unsigned int g_contador = 0;               // Contar el numero de paquetes enviados (en la variable time) 

// Nuevas var 11.08.2021
uint32_t g_tsLastReport       = 0;                  // ts: timeStamp?
#define MUESTRAS_SEGUNDO 360.0             
float g_reporting_period      = 1000000 / MUESTRAS_SEGUNDO; // 10^6 por ser micros. Frec muestreo
int g_cantidad_datos_leidos   = 0;

// Nuevas var max y min frec muestro. Inicialiacion.
unsigned int g_maxfrec = 0;
unsigned int g_minfrec = 10000;
unsigned int g_auxfrec = 0;
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
void calcularFrecuencias();                                         // Done
/* ------------------ Program -------------------------------------*/
/*
 * SETUP
 */


void setup()
{         
  // Initialize connection
  Serial.begin(SERIAL_BAUD_RATE);

   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println();


  // We need a unique device ID for our MQTT client connection.
  // Normalmente utilizariamos el ID del dispositivo (los de arduino no los tienen)
  g_cliente_ID = "IDSPV001";       // Get ID. Revisar TODO.
  

  /*------------------TOPICS-------------------------------------------------------------------------------*/
  // Set up the topics for publishing sensor readings. By inserting the unique ID,
  // the result is of the form: "arduino_1/GSR/valor" etc
  sprintf(g_command_topic, "cmnd/%s/COMMAND", g_cliente_ID);                     // For receiving commands

  #if REPORT_MQTT_SEPARATE
    sprintf(g_GSR_mqtt_topic, "arduino_1/%s/GSR/valor", g_cliente_ID);                // Data form AN33IoT
    sprintf(g_npaquete_mqtt_topic, "arduino_1/%s/GSR/numPaquete", g_cliente_ID);      // Data form AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    sprintf(g_mqtt_json_topic, "arduino_1/%s/SENSOR", g_cliente_ID);                  // Data from AN33IoT
  #endif

  // Report the MQTT topics to the serial console
  Serial.println(g_command_topic);                   // For receiving messages
  
  #if REPORT_MQTT_SEPARATE
    Serial.println("MQTT topics: ");
    Serial.println(g_GSR_mqtt_topic);                // From AN33IoT
    Serial.println(g_npaquete_mqtt_topic);           // From AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    Serial.println("MQTT topics en JSON: ");
    Serial.println(g_mqtt_json_topic);               // From AN33IoT
  #endif

  // Connect to WiFi
  if (initWiFi())
  {
    Serial.print("WiFi connected");
    Serial.println(ssid);
    Serial.print("IP local address: ");
    Serial.println(WiFi.localIP());


     /* Set up the MQTT client */
    mqttClient.setServer(mqtt_broker, port);
    mqttClient.setCallback(mqttCallback);
    // Tamaño del paquete que puede enviar MQTT por defecto 128 (recomendado 255)
    mqttClient.setBufferSize(2048);              

  } 
  else
  {
    Serial.println("WiFi FAILED");
    Serial.println("CHECK WIFI CONNEXION");       
    while(true){                                 
      
       
    }
    
  }
  Serial.println(g_reporting_period);
  g_tiempo_inicio = micros();  
}

/*
 * ------------------- MAIN LOOP --------------------------------------------------------------------------- 
 */

void loop() {
       

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttClient.connected())
    {
      reconnectMqtt();
    }
  }
  // Proces any outstanding MQTT messages. Tiene que ir después de la comprobación del Wifi.
  mqttClient.loop();                     

  Serial.print("Tiempo entre muestra y muestra: ");
  Serial.println(micros() - g_tsLastReport);

  // Calcular aproximadamente la frecuencia max y min
  //calcularFrecuencias();

  
  //g_reporting_period = 100000;
  // Control de frecuencia de muestreo
  if (micros() - g_tsLastReport >= g_reporting_period){
    calcularFrecuencias();
    
    g_tsLastReport = micros();
    
    updateReadings();
    reportToMqtt();
    
    // Vaciar paquete
    g_GSR_value = 0;
    
    
  }

    // Report the new values
   
   //reportToSerial();

   

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

  // Semilla aleatoria para valores random
  //randomSeed(micros());  
  
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
   Serial.println();
   Serial.print("Connecting to wifi ");
   Serial.println(ssid);

   // Conenecto to WiFi
   WiFi.begin(ssid, password);
   

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
     Serial.print("Falied connexion to Wifi. Attempts: ");
     Serial.println(num_attempts);
     return false;
   } 
   else
   {
     return true;
   }   
}

// PROCEDURE - Reconnect to MQTT broker, and publish a notifiacion to the status topic ----------------------
void reconnectMqtt(){
  char mqtt_client_id[20];
   // Añade a mqtt_client_id donde %s lee valor dado g_cliente_ID
  sprintf(mqtt_client_id, "Paciente-%s", g_cliente_ID);    

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

  g_GSR_value                 = analogRead(g_pin_GSR);
  g_GSR_readings_taken        = true;
  g_cantidad_datos_leidos++;
  //Serial.print("Datos leidos (GSR, HR, SpO2): ");
  Serial.print("Datos leidos (GSR): ");
  Serial.println(g_GSR_value);
  //Serial.print(", 0.00, 0.00");
}

// PROCEDURE - Report the latest values to MQTT -------------------------------------------------------------
void reportToMqtt()
{
  String message_string;
  Serial.println();
  Serial.println("###### ENVIANDO MENSAJES MQTT ######");
  

  #if REPORT_MQTT_SEPARATE
    if (true == g_GSR_readings_taken)
    {
      /* Report GSR value */
      message_string = String(g_GSR_value);
      message_string.toCharArray(g_mqtt_message_buffer, message_string.length() + 1);
      mqttClient.publish(g_GSR_mqtt_topic, g_mqtt_message_buffer);
      // TODO: Coger tiempo ???
      
      Serial.println("Topic GSR Value - Enviado");

      /* Report GSR time value */
      g_npaquete++;                                                               
      message_string = String(g_npaquete);
      message_string.toCharArray(g_mqtt_message_buffer, message_string.length() + 1);
      mqttClient.publish(g_npaquete_mqtt_topic, g_mqtt_message_buffer);
      Serial.println("Topic GSR Paquete - Enviado");
    }
   #endif

   #if REPORT_MQTT_JSON
    /* Report all values combined into one JSON message */

    // Note: The PubSubClient library limits MQTT message size to 128 bytes. The long format
    // message below only works because the message buffer size has been increased to 255 bytes
    // in setup.

      if (true == g_GSR_readings_taken)
      {
         
         // Es necesario poner %s entre "\%s"\ por ser un string o char 
         // Si pones \" %i\" estas pasando a string un entero
         sprintf(g_mqtt_message_buffer,  "{\"ID:%s\":{\"GSR\":%i,\"GSR Paquete\":%i}}",  
              g_cliente_ID, g_GSR_value, g_npaquete);
      }
      mqttClient.publish(g_mqtt_json_topic, g_mqtt_message_buffer);
      Serial.println("Topic JSON - Enviado");
    #endif
}


// PROCEDURE - Mostar por pantalla --------------------------------------------------------------------------
void reportToSerial()

{
  if (true == g_GSR_readings_taken)
  {

    int peso = 0;

    Serial.println("");
    Serial.println("###### MOSTRANDO DATOS ######");

    /* Paciente*/
    Serial.print("Paciente: ");
    Serial.println(g_cliente_ID);
    /* Report GSR value*/
    
    Serial.print(g_GSR_mqtt_topic);
    Serial.print(" topic is GSR: ");
    Serial.println(String(g_GSR_value));

    /* Report package number value*/
    Serial.print("Numero del paquete: ");
    Serial.println(String(g_npaquete));

    
     // Mostrar tamaño del buffer
    Serial.print("TAMAÑO DEL BUFFER: ");
    Serial.println(MQTT_MAX_PACKET_SIZE);

    /* Peso del paquete*/
    Serial.print("Peso del paquete GSR value: ");
    peso = sizeof(g_GSR_mqtt_topic);
    Serial.print(peso);
    Serial.println(" bytes.");

  }
}

// PROCEDURE - Calcular frecuencias max y min 
void calcularFrecuencias(){
  g_auxfrec = micros() - g_tsLastReport;
  
  if (g_auxfrec > g_maxfrec && g_auxfrec < 20000)
  {
     g_maxfrec = g_auxfrec;
  }
  if (g_auxfrec < g_minfrec)
  {
    g_minfrec = g_auxfrec;
  }

   Serial.print("Frecuencia max: ");
   Serial.println(g_maxfrec);
   Serial.print("Frecuencia min: ");
   Serial.println(g_minfrec);

   Serial.print("Numero del paquete: ");
   Serial.println(String(g_npaquete));
}
