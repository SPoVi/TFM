/*
 * SUPERHOUSE AUTOMATION (http://www.superhouse.tv)
 * https://github.com/SuperHouse/AQS/blob/main/Firmware/AirQualitySensorD1Mini/AirQualitySensorD1Mini.ino
 * 
 * Ultima fecha de modificacion: 22/07/2021 06/08/2021
 */

 /*
  * TODO:
  *       Introducir libraria time.h .Da igual que el setTime no sea exacto, lo quieres para medir tiempos.
  *       Revisar si se envian todos los paquetes. Elevar a QoS 2.
  *       
  *       Probar a cambiar a float las var que se quieren enviar a grafana. Sino cambiar mean to count.
  *       
  *       Ver si hay alguna forma de convertir decima a HEX u otro codig que reduzca el peso del paquete.
  *       
  *       Ver si se pudede eliminar g_data o g_GSR_value y utulizar solo uno.
  */

  /*
   * MODIFICADO:
   * 
   *      Buffer size, message_buffer - > Falta entender funcionamiento. Optimizarlo.
   *      
   *      Introduciendo visualicaciones de tiempos. tStart y tEnd 's
   */
   
   /*
    * OBSERVACIONES: 
    * 
    *     El tamaño del buffer tiene que ser superior al tamaño de los paquetes. Cuanto no se sabe, lo he multiplicado por 5.
    *     
    *     Puede que el tamño del paquete tenga que ser el doble mas 1 de la cantidad de muestras que se quiera recoger.
    *     Para asegurar valores de 3 digitos 
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

#define g_mysize 360                        // Tamaño del vector de datos
int g_data[g_mysize];                       // Vector de datos

bool g_GSR_readings_taken     = false;      // true/fasle: whether any readings have been taken
String g_GSR_value            = "";         // Value for sensor GSR
int g_pin_GSR                 = A0;         // Analog pin on NN33IoT
bool g_GSR_state_start        = 0;          // Timestamp when GSR state last changed


// MQTT
# define g_topicSize 360                          // Tamaño del string de los topics. Peso en bytes del paquete. 
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

// WiFi
#define WIFI_CONNECT_INTERVAL       500     // Wait 500ms intervals for wifi connection
#define WIFI_CONNECT_MAX_ATTEMPTS   10      // Number of attempts/intervals to wait

// Time
unsigned long g_segudo          = 0;        // Variable tiempo
unsigned long g_tiempo_inicio   = 0;
unsigned long g_tiempo_actual = 0;


//General
char* g_cliente_ID;                        // Simulate chipID

unsigned long g_tStart_loop  = 0;          // Initial time for measuring time spent
unsigned long g_tEnd_loop    = 0;          // End time for measuring time spent

// Contador
unsigned int g_contador = 0;               // Contar el numero de paquetes enviados (en la variable time) -----CAMBIAR NOMBRE ¡¡¡¡¡


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
//void mostrarTiempos();

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


  // We need a unique device ID for our MQTT client connection
  // g_cliente_ID = NewClientID();
  g_cliente_ID = "IDSPV001";       // Get ID. Revisar TODO.
  

  /*------------------TOPICS-----------------------------------------------------------------------------------------*/
  // Set up the topics for publishing sensor readings. By inserting the unique ID,
  // the result is of the form: "arduino_1/GSR/valor" etc
  sprintf(g_command_topic, "cmnd/%s/COMMAND", g_cliente_ID);                  // For receiving commands

  #if REPORT_MQTT_SEPARATE
    sprintf(g_GSR_mqtt_topic, "arduino_1/%s/GSR/valor", g_cliente_ID);        // Data form AN33IoT
    sprintf(g_npaquete_mqtt_topic, "arduino_1/%s/GSR/numPaquete", g_cliente_ID);      // Data form AN33IoT
  #endif

  #if REPORT_MQTT_JSON
    sprintf(g_mqtt_json_topic, "arduino_1/%s/SENSOR", g_cliente_ID);          // Data from AN33IoT
  #endif

  // Report the MQTT topics to the serial console
  Serial.println(g_command_topic);        // For receiving messages
  #if REPORT_MQTT_SEPARATE
    Serial.println("MQTT topics: ");
    Serial.println(g_GSR_mqtt_topic);      // From AN33IoT
    Serial.println(g_npaquete_mqtt_topic);     // From AN33IoT
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


     /* Set up the MQTT client */
    mqttClient.setServer(mqtt_broker, port);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(2048);              // Tamaño del paquete que puede enviar MQTT por defecto 128 (recomendado 255)

  } 
  else
  {
    Serial.println("WiFi FAILED");
    Serial.println("CHECK WIFI CONNEXION");
    while(true){
      
    }
    
  }
  

  g_tiempo_inicio = micros();
}

/*
 * ------------------- MAIN LOOP --------------------------------------------------------------------------------------------------------------------- LOOP
 */

void loop() {

  g_tiempo_actual = micros();             //TODO : Dejar solo uno de los temporizadores
  
  unsigned long tStart_loop, tEnd_loop, tStart_mqtt, tEnd_mqtt, tStart_update, tEnd_update, tStart_wifi, tEnd_wifi;

  tStart_loop = micros();                 // Comienza a contar el tiempo del loop  


  tStart_wifi = micros();                 // Comienza a contar el tiempo del proceso de reconexion al wifi
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttClient.connected())
    {
      reconnectMqtt();
    }
  }
  tEnd_wifi = micros();
  
  tStart_mqtt = micros();                 // Comienda a contar el tiempo del proceso mqtt
  mqttClient.loop();                      // Proces any outstanding MQTT messages
  tEnd_mqtt = micros();

  tStart_update = micros();               // Comienza a contar el tiempo del proceso updateReadings
  updateReadings();
  tEnd_update = micros();
  
  tEnd_loop = micros();                        // Finaliza el loop


  Serial.println("");
  Serial.println("##### MOSTRANDO TIEMPOS #####");
  Serial.println("-- Tiempo desde que se finalizó el setup--");
  showTimeNeeded(g_tiempo_inicio, g_tiempo_actual);  // tStart_loop = g_tiempo_actual
  Serial.println("-- Tiempo entre loops --");
  showTimeNeeded(tEnd_loop, tStart_loop);
  Serial.println("--Tiempo del loop--");
  showTimeNeeded(tStart_loop, tEnd_loop);
  Serial.println("--Tiempo de reconx Wifi--");
  showTimeNeeded(tStart_wifi, tEnd_wifi);
  Serial.println("--Tiempo de MQTT--");
  showTimeNeeded(tStart_mqtt, tEnd_mqtt);
  Serial.println("--Tiempo de updateReadings");
  
  
  Serial.println("----------------------------------------------------------------------------------------------------------");
  Serial.println();
  Serial.println();

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

// PROCEDURE - Reconnect to MQTT broker, and publish a notifiacion to the status topic -------------------------------
void reconnectMqtt(){
  char mqtt_client_id[20];
  sprintf(mqtt_client_id, "Paciente-%s", g_cliente_ID);     // Añade a mqtt_client_id donde %s lee valor dado g_cliente_ID

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

     // Para ver cuantos datos metemos
     //g_data[i] = i; 

     g_GSR_value.concat(g_data[i]);          // Creacion del paquete de datos 
     g_GSR_value.concat(" ");                // Añade espacio
   }
   GSR_tEnd=micros();              // Fin del tiempo para coger muestras

   
  // Mostrar tiempo necesitado para leer los datos por pantalla
  Serial.print("");
  Serial.println("----TIEMPO PARA LEER LOS DATOS ----");
  showTimeNeeded(GSR_tStart,GSR_tEnd);    
  g_GSR_readings_taken = true; 

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
         // Convertir a array
         String str = g_GSR_value;
         char char_array[g_GSR_value.length()];
         str.toCharArray(char_array, g_GSR_value.length());  // Eliminado el +1 para elimnar el ultimo caracter separador 

         sprintf(g_mqtt_message_buffer,  "{\"ID:%s\":{\"GSR\":\"%s\",\"GSR Paquete\":%i}}",  // Es necesario poner %s entre "\%s"\ por ser un string o char 
              g_cliente_ID, char_array, g_npaquete);
      }
      mqttClient.publish(g_mqtt_json_topic, g_mqtt_message_buffer);
      Serial.println("Topic JSON - Enviado");
    #endif
}

// PROCEDURE - Mostar tiempos por pantalla -------------------------------------------------------------------
/*
 * TODO : 
 */
void showTimeNeeded(unsigned long t_start, unsigned long t_end)
{
  float tiempo = 0.0;
  Serial.print("Tarda:  ");
  Serial.print(t_end-t_start);
  tiempo = float(t_end-t_start) / (1000000.0);
  Serial.print(" microsegundos. Que son: ");
  Serial.print(tiempo,6); // Lo que ha tardado en recoger la informacion
  Serial.println(" segundos.");
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

     // Ver el tamaño de g_GSR_value
     Serial.print("Tamaño de g_GSR_value (bytes): ");
     Serial.println(sizeof(g_GSR_value));               // Es un string, por lo que su peso es de 12 bytes. 

     // Ver el tamaño de g_data
     Serial.print("Tamaño de g_Data (bytes): ");
     Serial.println(sizeof(g_data));
   
  }
}
