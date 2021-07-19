/* ------------------- General config ------------------- */
// WiFi
const char* ssid          = "SSID";     // Your WiFi SSID
const char* password      = "PASSWORD";         // Your WiFi password


// MQTT
const char* mqtt_broker   = "IPBROKER";     // IP address of your MQTT broker
int port                  = 1883;               // Port connection
const char* mqtt_username = "USER";    // Your MQTT username
const char* mqtt_password = "PASS";    // Your MQTT password
#define REPORT_MQTT_SEPARATE true               // Report each valoue to its own topic
#define REPORT_MQTT_JSON true                   // Report all values in a JSON message
const char* status_topic = "events";            // MQTT topic to report startup

// Serial
#define SERIAL_BAUD_RATE 115200                 // Speed for USB serial console