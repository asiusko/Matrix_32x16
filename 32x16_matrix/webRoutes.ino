void startServerListener() {
  // Route for the setup WiFi page
  server.on("/", HTTP_GET, defineWiFi);
  // Route for getting data
  server.on("/configure", HTTP_POST, saveDataAndConnectToWifi);
  // Route to restart
  server.on("/restart", HTTP_GET, restartESP);
  // Games routes
  server.on("/games/tetris", HTTP_GET, startGameTetris);
  server.on("/games/control", HTTP_GET, []() {
    String action = server.arg("action");
    gameActions(action);
    server.send(200);
  });

  server.begin();
}
