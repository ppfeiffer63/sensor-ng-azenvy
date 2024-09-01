bool loadConfig() {
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    JsonDocument doc;
    auto error = deserializeJson(doc, configFile);
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    strlcpy(config.host, doc["host"] | "haus_75", sizeof(config.host));    
    strlcpy(config.ssid, doc["ssid"] | "Sensor", sizeof(config.ssid));  
    strlcpy(config.wifipassword, doc["wifipass"] | "sensorgarten", sizeof(config.wifipassword));  
    strlcpy(config.adminuser, doc["adminuser"] | "admin", sizeof(config.adminuser));  
    strlcpy(config.adminpassword, doc["adminpass"] | "admin", sizeof(config.adminpassword));  
    strlcpy(config.webuser, doc["webuser"] | "admin", sizeof(config.webuser));  
    strlcpy(config.webpassword, doc["webpass"] | "admin", sizeof(config.webpassword));  
    strlcpy(config.mqttserver,doc["mqttserver"]["ip"] | "192.168.12.2", sizeof(config.mqttserver));
    config.webserverport = doc["port"] | 80;

    // Real world application would store these values in some variables for
    // later use.

    configFile.close();
    return true;
}

bool defaultConfig(){
    JsonDocument doc;
    strlcpy(config.host, "AZ_envy", sizeof(config.host));    
    strlcpy(config.ssid, "Sensor", sizeof(config.ssid));  
    strlcpy(config.wifipassword, "sensorgarten", sizeof(config.wifipassword));  
    strlcpy(config.adminuser, "admin", sizeof(config.adminuser));  
    strlcpy(config.adminpassword, "admin", sizeof(config.adminpassword));  
    strlcpy(config.webuser, "admin", sizeof(config.webuser));  
    strlcpy(config.webpassword, "admin", sizeof(config.webpassword));  
    strlcpy(config.mqttserver,"192.168.12.2", sizeof(config.mqttserver));
    config.webserverport =  80;

    doc["ssid"] = config.ssid;
    doc["host"] = config.host;
    doc["wifipass"] = config.wifipassword;
    doc["adminuser"] = config.adminuser;
    doc["adminpass"] = config.adminpassword;
    doc["webuser"] = config.webuser;
    doc["webpass"] = config.webpassword;
    doc["mqttserver"]["ip"] = config.mqttserver;
    doc["port"] = config.webserverport;
    // Real world application would store these values in some variables for
    // later use.
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    // Serialize JSON to file
    if (serializeJsonPretty(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
    }
    configFile.close();
    return true;
}

bool saveConfig() {
    JsonDocument doc;

    doc["ssid"] = config.ssid;
    doc["host"] = config.host;
    doc["wifipass"] = config.wifipassword;
    doc["adminuser"] = config.adminuser;
    doc["adminpass"] = config.adminpassword;
    doc["webuser"] = config.webuser;
    doc["webpass"] = config.webpassword;
    doc["mqttserver"]["ip"] = config.mqttserver;
    doc["port"] = config.webserverport;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    // Serialize JSON to file
    if (serializeJsonPretty(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    configFile.close();
    return true;
}


void getSensorReadings() {
  //temperature =  sht1x.readTemperatureC();
  //humidity =  sht1x.readHumidity();
  sht30.get();
  dewpoint = 243.04*(log(sht30.humidity/100)+((17.625*sht30.cTemp)/(243.04+sht30.cTemp)))/(17.625-log(sht30.humidity/100)-((17.625*sht30.cTemp)/(243.04+sht30.cTemp)));
  humidity = sht30.humidity;
  temperature = sht30.cTemp;
  float T = temperature;
  float RHx = humidity;
  heatindex =(-42.379+(2.04901523*T)+(10.14333127*RHx)-(0.22475541*T*RHx)-(0.00683783*T*T)-(0.05481717*RHx*RHx)+(0.00122874*T*T*RHx)+(0.00085282*T*RHx*RHx)-(0.00000199*T*T*RHx*RHx)-32)*5/9;
  if ((sht30.cTemp <= 26.66) || (sht30.humidity <= 40)) heatindex = sht30.cTemp;
  //dewpoint = (243.5*(log(humidity/100)+((17.67*temperature)/(243.5+temperature)))/(17.67-log(humidity/100)-((17.67*temperature)/(243.5+temperature))));
  //float* values= mq2.read(true);

  lpg=mq2.readLPG();
  co = mq2.readCO();
  smoke = mq2.readSmoke();
  //float ADWert = analogRead(A0);
  Serial.println(lpg); 
  Serial.println(co);
  Serial.println(smoke);
  Serial.println(dewpoint);
  Serial.println(temperature);
}

String processor(const String& var) {
  Serial.println(var);
  if (var == "TEMPERATURE") {
    return String(temperature);
  }
  else if (var == "DEWPOINT") {
    return String(dewpoint);
  }
  else if (var == "HUMIDITY") {
    return String(humidity);
  }
  else if (var == "HEATINDEX") {
    return String(heatindex);
  }
  else if (var == "LPG") {
    return String(lpg);
  }
  else if (var == "CO") {
    return String(co);
  }
  else if (var == "SMOKE") {
    return String(smoke);
  }
  //---
  else if (var == "BUILD_TIMESTAMP"){
    return String(__DATE__) + " " + String(__TIME__);
  }
  else if (var == "IP"){
    return WiFi.localIP().toString();
  }
  //----
  else
    return "?";
}


void initServer(){
    // Handle Web Server
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);
    // Server send JSON-DATA
    server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      JsonDocument json;
      json["temperatur"] = String(temperature);
      json["feuchte"] = String(humidity);
      json["taupunkt"] = String(dewpoint);
      json["heatindex"] = String(heatindex);
      json["lpg"] = String(lpg);
      json["co"] = String(co);
      json["smoke"] = String(smoke);
      serializeJson(json, *response);
      request->send(response);
    });
    server.on("/api/wifi-info", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      JsonDocument json;
      json["status"] = "ok";
      json["ssid"] = WiFi.SSID();
      json["ip"] = WiFi.localIP().toString();
      serializeJson(json, *response);
      request->send(response);
    });
    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient * client) {
        if (client->lastId()) {
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);
    server.addHandler(new FSEditor("admin","admin"));



    // MQTT Server Verbindung
    client.setServer(config.mqttserver,1883);
    // Start AsyncElegantOTA
    AsyncElegantOTA.begin(&server);    
    // Start Server
    server.begin();

}



// Initialize WiFi
bool initWiFi() {
  unsigned long timer;
  WiFi.setHostname(config.host);
  WiFi.mode(WIFI_STA);
  int wifi_timeout = 10; // Sekunden
  timer = millis() / 1000;
  WiFi.begin(config.ssid, config.wifipassword);
  Serial.printf("Connecting to WiFi .. %s ",config.ssid);
  
  while (WiFi.status() != WL_CONNECTED && (millis() / 1000) < timer + wifi_timeout) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("\nTimeout beim Verbindungsaufbau");
    return false;
  } else {
    Serial.print("\nIP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
}

void initPortal(){
  Serial.printf("Connecting to Manager ..");
  WiFi.softAP("Sensor-Manager");
  server.on("/", HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(LittleFS,"/wifimanager.html","text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.begin();
}



void mqtt_reconnect(){
  while (!client.connected()){
    Serial.println("starte MQTT Verbindung....");
    String clientId = (config.host);
    clientId += "-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())){
      Serial.println("verbunden");
      client.publish(config.host , "online");
    } else {
      Serial.print("failed, rc= ");
      Serial.println(client.state());
      delay(5000);
    }




  }

}



