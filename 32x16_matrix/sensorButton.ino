void sensorButtonSingleClick() {
  // cancel actions - single click
  if (isReadyToResetWiFi) {
    isReadyToResetWiFi = false;
    logLn("Reset canceled");
    blinkLed();
    return;
  }
}

void sensorButtonDoubleClick() {
  // confirm actions - double click
  if (isReadyToResetWiFi) {
    isReadyToResetWiFi = false;

    logLn("Resetting WiFi data...");
    resetSavedData();
    blinkLed(2);
    logLn("Restart ESP");

    return;
  }
}

void sensorButtonLongPress() {
  isReadyToResetWiFi = true;
  resetCounterMillis = millis();
  logLn("Resetting WiFi data");
  logLn("Double click - Yes, Single Click - No");
}
