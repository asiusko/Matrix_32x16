void buildTime(byte led_strip_number, String time_string) {
  matrix->setTextWrap(false);
  matrix->setFont(&TomThumb);
  matrix->setTextColor(matrix->Color(255, 255, 255));
  matrix->fillScreen(0);
  matrix->setCursor(3, 10);  // Start X and Y position
  matrix->print(time_string);
}
