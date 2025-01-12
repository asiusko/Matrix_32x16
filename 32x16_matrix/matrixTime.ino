void buildTime(byte led_strip_number, String time_string, FastLED_NeoMatrix* matrix) {
  matrix->setTextWrap(false);
  matrix->setFont(&TomThumb);
  matrix->setTextColor(matrix->Color(255, 255, 255));
  matrix->fillScreen(0);
  // Start X and Y position
  matrix->setCursor(3, 10); 
  matrix->print(time_string);
}
