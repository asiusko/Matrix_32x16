void buildEqualizer(byte led_strip_number, CRGB* ledsConfiguration, FastLED_NeoMatrix* matrix) {
  // settings
  byte peaksMode = LED_SETTINGS_VALUE[led_strip_number][2];
  byte equalizerMode = LED_SETTINGS_VALUE[led_strip_number][3];
  byte noiseLevel = LED_SETTINGS_VALUE[led_strip_number][5];

  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  if (equalizerMode != 6) FastLED.clear();

  // Reset bandValues[]
  for (int i = 0; i < LED_MAX_BANDS; i++) {
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < FFT_SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN);  // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */
    }
  }

  // Fast Fourier Transformation
  FFT.dcRemoval();
  FFT.windowing(vReal, FFT_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, FFT_SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, FFT_SAMPLES);

  for (int i = 2; i < (FFT_SAMPLES / 2); i++) {  // Don't use sample 0 and only first FFT_SAMPLES/2 are usable.
    if (vReal[i] > FFT_FILTERED_NOISE) {         // Add a crude noise filter

      // 16 bands, rate 24KHz, bands 200Hz-12kHz
      if (i <= 5) bandValues[0] += (int)vReal[i];
      if (i > 5 && i <= 6) bandValues[1] += (int)vReal[i];
      if (i > 6 && i <= 9) bandValues[2] += (int)vReal[i];
      if (i > 9 && i <= 11) bandValues[3] += (int)vReal[i];
      if (i > 11 && i <= 15) bandValues[4] += (int)vReal[i];
      if (i > 15 && i <= 19) bandValues[5] += (int)vReal[i];
      if (i > 19 && i <= 25) bandValues[6] += (int)vReal[i];
      if (i > 25 && i <= 33) bandValues[7] += (int)vReal[i];
      if (i > 33 && i <= 44) bandValues[8] += (int)vReal[i];
      if (i > 44 && i <= 58) bandValues[9] += (int)vReal[i];
      if (i > 58 && i <= 76) bandValues[10] += (int)vReal[i];
      if (i > 76 && i <= 99) bandValues[11] += (int)vReal[i];
      if (i > 99 && i <= 131) bandValues[12] += (int)vReal[i];
      if (i > 131 && i <= 172) bandValues[13] += (int)vReal[i];
      if (i > 172 && i <= 225) bandValues[14] += (int)vReal[i];
      if (i > 225) bandValues[15] += (int)vReal[i];
    }
  }


  // Process the FFT data into bar heights
  for (byte band = 0; band < LED_MAX_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = bandValues[band] / (FFT_AMPLITUDE * noiseLevel);
    if (barHeight > LED_TOP_ELEMENTS[led_strip_number]) barHeight = LED_TOP_ELEMENTS[led_strip_number];

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(LED_TOP_ELEMENTS[led_strip_number], barHeight);
    }

    // Draw bars
    switch (equalizerMode) {
      case 1:
        rainbowBars(led_strip_number, band, barHeight, matrix);
        break;
      case 2:
        // No bars on this one
        break;
      case 3:
        purpleBars(led_strip_number, band, barHeight, matrix);
        break;
      case 4:
        centerBars(led_strip_number, band, barHeight, matrix);
        break;
      case 5:
        changingBars(led_strip_number, band, barHeight, matrix);
        break;
      case 6:
        waterfall(led_strip_number, band, ledsConfiguration, matrix);
        break;
    }

    // Draw peaks
    if (equalizerMode != 4 && equalizerMode != 6) {
      switch (peaksMode) {
        case 1:
          whitePeak(led_strip_number, band, matrix);
          break;
        case 2:
          outrunPeak(led_strip_number, band, matrix);
          break;
        case 3:
          whitePeak(led_strip_number, band, matrix);
          break;
      }
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
}

// PATTERNS
void rainbowBars(byte led_strip_number, int band, int barHeight, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    for (int y = LED_TOP_ELEMENTS[led_strip_number]; y >= LED_TOP_ELEMENTS[led_strip_number] - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV((x / LED_BAR_WIDTHS[led_strip_number]) * (255 / LED_MAX_BANDS), 255, 255));
    }
  }
}

void purpleBars(byte led_strip_number, int band, int barHeight, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    for (int y = LED_TOP_ELEMENTS[led_strip_number]; y >= LED_TOP_ELEMENTS[led_strip_number] - barHeight; y--) {
      matrix->drawPixel(x, y, ColorFromPalette(purplePal, y * (255 / (barHeight + 1))));
    }
  }
}

void changingBars(byte led_strip_number, int band, int barHeight, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    for (int y = LED_TOP_ELEMENTS[led_strip_number]; y >= LED_TOP_ELEMENTS[led_strip_number] - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV(y * (255 / LED_MATRIX_HEIGHTS[led_strip_number]) + colorTimer, 255, 255));
    }
  }
}

void centerBars(byte led_strip_number, int band, int barHeight, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((LED_MATRIX_HEIGHTS[led_strip_number] - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix->drawPixel(x, y, ColorFromPalette(heatPal, colorIndex));
    }
  }
}

void whitePeak(byte led_strip_number, int band, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  int peakHeight = LED_TOP_ELEMENTS[led_strip_number] - peak[band] - 1;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    matrix->drawPixel(x, peakHeight, CHSV(0, 0, 255));
  }
}

void outrunPeak(byte led_strip_number, int band, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  int peakHeight = LED_TOP_ELEMENTS[led_strip_number] - peak[band] - 1;
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    matrix->drawPixel(x, peakHeight, ColorFromPalette(outrunPal, peakHeight * (255 / LED_MATRIX_HEIGHTS[led_strip_number])));
  }
}

void waterfall(byte led_strip_number, int band, CRGB* ledsConfiguration, FastLED_NeoMatrix* matrix) {
  int xStart = LED_BAR_WIDTHS[led_strip_number] * band;
  double highestBandValue = 60000;  // to calibrate the waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + LED_BAR_WIDTHS[led_strip_number]; x++) {
    matrix->drawPixel(x, 0, CHSV(constrain(map(bandValues[band], 0, highestBandValue, 160, 0), 0, 160), 255, 255));
  }

  // Move screen up starting at 2nd row from top
  if (band == LED_MAX_BANDS - 1) {
    for (int y = LED_MATRIX_HEIGHTS[led_strip_number] - 2; y >= 0; y--) {
      for (int x = 0; x < LED_MATRIX_WIDTHS[led_strip_number]; x++) {
        int pixelIndexY = matrix->XY(x, y + 1);
        int pixelIndex = matrix->XY(x, y);
        ledsConfiguration[pixelIndexY] = ledsConfiguration[pixelIndex];
      }
    }
  }
}
