void buildGame(int led_strip_number, CRGB* ledsConfiguration, FastLED_NeoMatrix* matrix, String action) {
  String localIP = WiFi.localIP().toString();

  matrix->setTextWrap(false);
  matrix->setFont(&TomThumb);
  matrix->setTextColor(matrix->Color(255, 255, 255));
  matrix->fillScreen(0);
  // Start X and Y position

  if (action.length() > 0) {
    matrix->setCursor(4, 10);
    matrix->print(action);
    delay(500);
  } else {
    matrix->setCursor(2, 8);
    matrix->print(localIP.substring(0, 8));
    matrix->setCursor(4, 15);
    matrix->print(localIP.substring(8));
  }
}
