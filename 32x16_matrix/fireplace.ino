#define COOLING 55
#define SPARKING 120
#define FIRE_HEIGHT 15

void fireplace(int led_strip_number, CRGB* ledsConfiguration, FastLED_NeoMatrix* matrix) {
  // TODO rework
  static int heat[FIRE_HEIGHT][32];

  int matrixWidth = matrix->width();
  int matrixHeight = matrix->height();

  // Step 1: Cool down every cell in the heat array
  for (int col = 0; col < matrixWidth; col++) {
    for (int row = 0; row < matrixHeight; row++) {
      heat[row][col] = qsub8(heat[row][col], random8(0, ((COOLING * 10) / matrixHeight) + 2));
    }
  }

  // Step 2: Heat from the bottom (sparks)
  for (int col = 0; col < matrixWidth; col++) {
    if (random8() < SPARKING) {
      int y = matrixHeight - 1;
      heat[y][col] = qadd8(heat[y][col], random8(160, 255));
    }
  }

  // Step 3: Propagate heat upwards
  for (int col = 0; col < matrixWidth; col++) {
    for (int row = 1; row < matrixHeight; row++) {
      heat[row - 1][col] = (heat[row][col] + heat[max(row - 1, 0)][col] + heat[min(row + 1, matrixHeight - 1)][col]) / 3;
    }
  }

  // Step 4: Map heat to LED colours
  for (int col = 0; col < matrixWidth; col++) {
    for (int row = 0; row < matrixHeight; row++) {
      int colourIndex = scale8(heat[row][col], 240);
      matrix->drawPixel(col, row, HeatColor(colourIndex));
    }
  }
}

// Firepower pattern
const int patternPowerMask[16][32] = {
    {0  , 4  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 4  , 0  , 0  , 0  },
    {0  , 4  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 16 , 8  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 4  , 0  , 0  , 0  },
    {0  , 4  , 8  , 0  , 0  , 0  , 8  , 0  , 16 , 0  , 0  , 0  , 16 , 0  , 0  , 32 , 16 , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 16 , 0  , 0  , 0  , 4  , 0  , 0  , 0  },
    {0  , 4  , 8  , 0  , 0  , 0  , 8  , 0  , 16 , 0  , 0  , 0  , 32 , 64 , 0  , 32 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 4  , 0  , 0  , 0  },
    {0  , 4  , 0  , 0  , 16 , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 0  , 0  , 4  , 0  , 0  , 0  },
    {0  , 8  , 16 , 8  , 0  , 32 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 128, 64 , 128, 64 , 128, 128, 0  , 0  , 0  , 16 , 32 , 0  , 8  , 32 , 8  , 64 , 0  , 16 , 0  },
    {4  , 0  , 16 , 2  , 4  , 32 , 32 , 64 , 128, 32 , 16 , 64 , 64 , 128, 128, 64 , 128, 64 , 128, 64 , 0  , 32 , 32 , 16 , 0  , 8  , 16 , 8  , 0  , 8  , 16 , 32 },
    {0  , 4  , 8  , 16 , 32 , 0  , 32 , 32 , 128, 64 , 128, 64 , 128, 128, 128, 64 , 128, 128, 32 , 32 , 64 , 64 , 32 , 64 , 0  , 0  , 16 , 4  , 8  , 32 , 0  , 32 },
    {32 , 8  , 16 , 2  , 32 , 128, 128, 64 , 128, 128, 64 , 128, 128 ,64 , 64 , 128, 64 , 64 , 64 , 128, 64 , 128, 128, 64 , 16 , 64 , 32 , 0  , 8  , 0  , 64 , 16 },
    {4  , 32 , 8  , 16 , 32 , 128, 128, 64 , 128, 64 , 128, 128, 64  ,128, 64 , 64 , 128, 128, 64 , 128, 64 , 32 , 128, 64 , 32 , 0  , 32 , 8  , 16 , 64 , 0  , 32 },
    {64 , 16 , 0  , 8  , 128, 255, 128, 64 , 128, 64 , 128, 128, 64 , 128, 128, 64 , 128, 128, 32 , 128, 64 , 128, 32 , 128, 16 , 128, 64 , 128, 64 , 128, 16 , 32 },
    {4  , 0  , 64 , 128, 255, 128, 255, 128, 255, 128, 64 , 128, 16 , 64 , 128, 128, 255, 128, 255, 255, 128, 128, 255, 64 , 128, 128, 32 , 64 , 16 , 128, 8  , 64 },
    {0  , 32 , 128, 255, 32 , 128, 128, 128, 128, 128, 64 , 128, 255, 128, 64 , 128, 64 , 128, 128, 255, 128, 64 , 255, 128, 64 , 32 , 128, 64 , 8  , 32 , 16 , 128},
    {128, 255, 128, 64 , 128, 255, 128, 128, 64 , 128, 64 , 32 , 64 , 128, 32 , 128, 64 , 128, 128, 64 , 128, 128, 64 , 128, 128, 128, 255, 64 , 32 , 128, 64 , 128},
    {64 , 128, 128, 128, 64 , 128, 128, 64 , 128, 64 , 128, 128, 255, 128, 128, 64 , 128, 128, 64 , 128, 255, 128, 128, 128, 255, 128, 64 , 128, 64 , 32 , 32 , 128},
    {128, 128, 128, 64 , 128, 255, 128, 128, 128, 128, 64 , 128, 64 , 128, 128, 64 , 128, 128, 128, 128, 255, 128, 128, 128, 128, 64 , 128, 128, 128, 64 , 32 , 64 },
};

void drawCampfire(int led_strip_number, CRGB* ledsConfiguration, FastLED_NeoMatrix* matrix) {
  static uint8_t heat[16][32];

  // Step 1: Cool down every cell a little
  for (int col = 0; col < 32; col++) {
    for (int row = 0; row < 16; row++) {
      heat[row][col] = qsub8(heat[row][col], random8(0, ((COOLING * 10) / 16) + 2));
    }
  }

  // Step 2: Add heat based on the patternPowerMask
  for (int col = 0; col < 32; col++) {
    for (int row = 0; row < 16; row++) {
      if (random8() < patternPowerMask[row][col]) {
        heat[row][col] = qadd8(
          heat[row][col],
          patternPowerMask[row][col] < 64
            ? random8(2, patternPowerMask[row][col])
            : random8(64, patternPowerMask[row][col]));
      }
    }
  }

  // Step 3: Propagate heat upwards
  for (int col = 0; col < 32; col++) {
    for (int row = 15; row > 0; row--) {
      heat[row - 1][col] = (heat[row][col] + heat[row - 1][col] + heat[max(row - 2, 0)][col]) / 3;
    }
  }

  // Step 4: Map heat to LED colours
  for (int col = 0; col < 32; col++) {
    for (int row = 0; row < 16; row++) {
      uint8_t colourIndex = scale8(heat[row][col], 240);
      matrix->drawPixel(col, row, HeatColor(colourIndex));
    }
  }
}
