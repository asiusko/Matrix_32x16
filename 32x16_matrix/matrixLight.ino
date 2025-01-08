void buildLight(byte led_strip_number) {
  for (int y = 0; y < LED_MATRIX_HEIGHT; y++) {
    for (int x = 0; x < LED_MATRIX_WIDTH; x++) {
      matrix->drawPixel(x, y, CHSV(0, 0, 255));
    }
  }
}
