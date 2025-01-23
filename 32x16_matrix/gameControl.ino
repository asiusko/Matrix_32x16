void startGameTetris() {
  startGameControl();
  LED_SETTINGS_VALUE[0][0] = 3;
}

void startGameControl() {
  server.send(200, "text/html", wifiJoystickPage);
}

void gameActions(String action) {
  gameBtnAction = action;
  blinkLed(1, 100);
  logLn(action);
}
