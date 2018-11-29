#include <WiFi.h>
#include <Adafruit_ADS1015.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

Adafruit_ADS1115 ads;  // Declare an instance of the ADS1115

int16_t rawADCvalue;  // The is where we store the value we receive from the ADS1115
float scalefactor = 0.1875F; // This is the scale factor for the default +/- 6.144 Volt Range we will use
float volts = 0.0; // The result of applying the scale factor to the raw value

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "mikrotik"
#define WLAN_PASS       "baliteam888"
/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "202.179.136.148"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish voltage = Adafruit_MQTT_Publish(&mqtt,"/volt");

void MQTT_connect();
void WiFi_connect();
int lastSend = 0;
void setup(void)
{
  Serial.begin(115200); 
  ads.begin();
  
 // init WiFi
    Serial.println("Connecting to WiFi");
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop(void)
{  
  MQTT_connect();
  
  rawADCvalue = ads.readADC_Differential_0_1(); 
  volts = (rawADCvalue * scalefactor)/1000.0;
  
//  Serial.print("Raw ADC = "); 
//  Serial.print(rawADCvalue); 
//  Serial.print("\tVoltage = ");
 // Serial.println(volts,6);
//  Serial.println();

  voltage.publish(volts);
  lastSend = millis();
  delay(5000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
 
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

   Serial.println("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
