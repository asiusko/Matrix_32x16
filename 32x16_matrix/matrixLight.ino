void buildLight(int led_strip_number, FastLED_NeoMatrix* matrix) {
  for (int y = 0; y < LED_MATRIX_HEIGHTS[led_strip_number]; y++) {
    for (int x = 0; x < LED_MATRIX_WIDTHS[led_strip_number]; x++) {
      matrix->drawPixel(x, y, CHSV(0, 0, 255));
    }
  }
}
