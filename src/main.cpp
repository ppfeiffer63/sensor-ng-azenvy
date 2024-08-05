#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>


#include <ESPAsyncWebServer.h>
#include <FSEditor.h>
#include <AsyncElegantOTA.h>

//#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>




const char* ssid = "devnet-34";
const char* password = "testerwlan";




   
//
WiFiClient espClient;
PubSubClient client(espClient);
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");

#include <WEMOS_SHT3X.h>  //https://github.com/wemos/WEMOS_SHT3x_Arduino_Library
#include <MQ2.h>

#include <variables.h>
#include <function_subs.h>

/*-----------------------------Programm Setup---------------------------------------------*/

void setup(void) {
  Serial.begin(115200);
  delay(300);
  Serial.println("Mounting LittleFS ...");
	if (!LittleFS.begin()) {
		// SPIFFS will be configured on reboot
		Serial.println("ERROR: Cannot mount LittleFS, Rebooting");
		delay(1000);
		ESP.restart();
		}
  
  status = loadConfig();
  defaultConfig();
  if (!status){
    Serial.println("default config...");
    defaultConfig();
  }
  delay(1000);

  if (initWiFi()){
      initServer();
      client.setServer(config.mqttserver, 1883);

    } else {
      initPortal();
    };
    status = saveConfig();
    pinMode(A0, INPUT);
    mq2.begin();
    delay(2000);
}

void loop(void) {
  if ((millis() - lastTime) > time_meas) {
    if (running) {
      if (!client.connected()){
        mqtt_reconnect();
      }
      client.loop();
    }
    getSensorReadings();
    lastTime = millis();
  }
  if (IsRebootRequired) {
		Serial.println("Rebooting ESP32: "); 
		delay(1000); // give time for reboot page to load
		ESP.restart();
	}
}