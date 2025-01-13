void defineWiFi() {
  logLn("load initial page...");
  server.send(200, "text/html", wifiConfigPage);
}

void saveDataAndConnectToWifi() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Write data to a JSON file
  JSONVar jsonDoc;
  jsonDoc["ssid"] = ssid;
  jsonDoc["pass"] = password;
  // write to file
  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    configFile.println(jsonDoc);
    configFile.close();
    logLn("Config saved.");
  } else {
    logLn("Failed to open config file for writing");
  }

  // Connect to Wi-Fi
  if (connectToWifi(ssid, password)) {
    server.send(200, "text/html", "Connected to Wi-Fi.");
  } else {
    server.send(200, "text/html", "Cannot connect to Wi-Fi, reset config.");
  }
}

bool connectToWifi(String ssid, String password) {
  logLn("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);
  int8_t connections = 0;

  while (WiFi.status() != WL_CONNECTED && connections < 30) {
    blinkLed(1, 500);
    log(".");
    connections++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    logLn("Connected to Wi-Fi");

    return true;
  } else {
    logLn("Cannot connect to Wi-Fi, reset config.");

    return false;
  }
}

void resetSavedData(){
  // Delete the file if it exists
  logLn("Deleting config file...");
  if (SPIFFS.exists("/config.json")) {
    SPIFFS.remove("/config.json");
    logLn("Config deleted");
  } else {
    logLn("Config does not exist");
  }
}


