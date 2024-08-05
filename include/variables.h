typedef struct WIFI_CONFIG {
  char ssid[20];
  char host[20];
  char wifipassword[20];
  char webuser[20];
  char webpassword[20];
  char adminuser[20];
  char adminpassword[20];
  char mqttserver[20];
  int webserverport;
} WIFI_CONFIG;

WIFI_CONFIG config;

SHT3X sht30(0x44);
MQ2 mq2(A0);

// Timer variables
unsigned long lastTime = 0;
unsigned long lastMeas = 0;
unsigned long timerDelay = 15000;
unsigned long time_meas = 60000;
// dummy status
bool status = false;
bool running = false;
bool IsRebootRequired = false;


// variable holding BME readout
float temperature, humidity, dewpoint, heatindex;
float lpg, co, smoke;
float temp_array[48];
float humi_array[48];



