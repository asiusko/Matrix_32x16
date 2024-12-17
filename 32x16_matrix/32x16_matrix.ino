#include <FastLED_NeoMatrix.h>
#include <Fonts/TomThumb.h>
#include <arduinoFFT.h>
#include <AiEsp32RotaryEncoder.h>
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#define AUDIO_IN_PIN 35               // Audio signal
#define SENSOR_BTN_PIN 32             // Touch btn
#define FFT_SAMPLES 512               // Must be a power of 2
#define FFT_SAMPLING_FREQ 24000       // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define FFT_AMPLITUDE 10000           // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define FFT_FILTERED_NOISE 1000       // Used as a crude noise filter, values below this are ignored

#define LED_STRIP_NUMBERS 4           // Amount of strips or matrix
#define LED_PIN_1 33                  // LED strip data
#define LED_PIN_2 25                  // LED strip data
#define LED_PIN_3 26                  // LED strip data
#define LED_PIN_4 27                  // LED strip data
#define LED_COLOR_ORDER GRB           // If colours look wrong, play with this
#define LED_CHIPSET WS2812B           // LED strip type
#define LED_MAX_MILLI_AMPS 9000       // Be careful with the amount of power here if running from USB port
#define LED_VOLTS 5                   // Usually 5 or 12
#define LED_NUM_BANDS 16              // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define LED_MATRIX_HEIGHT 16
#define LED_MATRIX_WIDTH 32
#define LED_QUANTITY (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT)     // Total number of LEDs
#define LED_BAR_WIDTH (LED_MATRIX_WIDTH / (LED_NUM_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define LED_TOP_ELEMENT (LED_MATRIX_HEIGHT - 0)                 // Don't allow the bars to go offscreen
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define TIMER_AUTO_CHANGE (60 * 1000)  // 60 second
#define TIMER_COLOR_CHANGE 10          // 10 millisecond
#define TIMER_DECAY_PEAK 60            // 60 millisecond
#define ROTARY_ENCODER_A_PIN 18        // CLK (A pin)
#define ROTARY_ENCODER_B_PIN 19        // DT (B pin)
#define ROTARY_ENCODER_BUTTON_PIN 5    // SW (button pin)
#define ROTARY_ENCODER_VCC_PIN -1      // VCC if microcontroller VCC (then set ROTARY_ENCODER_VCC_PIN -1)
#define ROTARY_ENCODER_STEPS 4         // depending on your encoder - try 1,2 or 4 to get expected behaviour
#define ROTARY_ENCODER_NOISE 10
#define ROTARY_ENCODER_MIN_VALUE 0
#define ROTARY_ENCODER_MAX_VALUE 15

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
byte encoderValue = 1;

// Settings
const byte BRIGHTNESS_SETTINGS[(ROTARY_ENCODER_MAX_VALUE + 1)] = { 0, 1, 2, 5, 10, 20, 40, 60, 80, 100, 125, 150, 175, 200, 225, 250 };
const String ENCODER_MODE[3] = { "DISPLAY", "SCROLL", "CHANGE VALUE" };
byte encoderMode = 0;
const String ENCODER_BTN_MODE[3] = {"SELECT LED", "SELECT", "CONFIRM" };
byte encoderBtnMode = 0;
const String SETTINGS[LED_STRIP_NUMBERS + 1] = { "LED 1", "LED 2", "LED 3" ,"LED 4", "LOGS" }; // LED_STRIP_NUMBERS + 1(Logs)
const byte settingsPropsLength = 6;
const String LED_SETTINGS[settingsPropsLength + 1] = { "MODE", "BRIGHTNESS", "PEAKS", "EQUALIZER MODE", "AUTO CHANGE", "NOISE LEVEL", "EXIT" };  // settingsPropsLength + 1(Exit)
const String LED_MODE[4] = { "EQUALIZER", "TIME", "LIGHT" ,"GAME" };

byte LED_SETTINGS_VALUE[LED_STRIP_NUMBERS][settingsPropsLength] = {
  { 0, 10, 1, 1, 0, 1 },
  { 0, 10, 1, 1, 0, 1 },
  { 0, 10, 1, 1, 0, 1 },
  { 0, 10, 1, 1, 0, 1 },
};
byte currentSetting = 0;
byte ledSettingProperty = 0;
byte newEncoderValue = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty];


// Sampling and FFT
unsigned int sampling_period_us;
byte peak[LED_NUM_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int oldBarHeights[LED_NUM_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int bandValues[LED_NUM_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
unsigned long newTime;
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFT_SAMPLES, FFT_SAMPLING_FREQ);

// Create OLED display object
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// FastLED
CRGB leds[LED_QUANTITY];
DEFINE_GRADIENT_PALETTE(purple_gp){
  0, 0, 212, 255,       //blue
  255, 179, 0, 255      //purple
};
DEFINE_GRADIENT_PALETTE(outrun_gp){
  0, 141, 0, 100,       //purple
  127, 255, 192, 0,     //yellow
  255, 0, 5, 255        //blue
};
DEFINE_GRADIENT_PALETTE(greenblue_gp){
  0, 0, 255, 60,        //green
  64, 0, 236, 255,      //cyan
  128, 0, 5, 255,       //blue
  192, 0, 236, 255,     //cyan
  255, 0, 255, 60       //green
};
DEFINE_GRADIENT_PALETTE(redyellow_gp){
  0, 200, 200, 200,     //white
  64, 255, 218, 0,      //yellow
  128, 231, 0, 0,       //red
  192, 255, 218, 0,     //yellow
  255, 200, 200, 200    //white
};
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

// FastLED_NeoMatrix - see https://github.com/marcmerlin/FastLED_NeoMatrix
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT,
                                                  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  delay(100);
  Serial.begin(115200);

  // Initialize OLED display (with I2C address 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  display.clearDisplay();
  renderOLED();

  //rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(ROTARY_ENCODER_MIN_VALUE, ROTARY_ENCODER_MAX_VALUE, true);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

  /*Rotary acceleration introduced 25.2.2021.
   * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
   * without acceleration you need long time to get to that number
   * Using acceleration, faster you turn, faster will the value raise.
   * For fine tuning slow down.
   */
  rotaryEncoder.disableAcceleration();  //acceleration is now enabled by default - disable if you dont need it
                                        //rotaryEncoder.setAcceleration(250); //or set the value - larger number = more acceleration; 0 or 1 means disabled acceleration


  // 32x16 WS2812 LED
  FastLED.addLeds<LED_CHIPSET, LED_PIN_1, LED_COLOR_ORDER>(leds, LED_QUANTITY).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, LED_MAX_MILLI_AMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[LED_SETTINGS_VALUE[currentSetting][1]]);
  FastLED.clear();

  sampling_period_us = round(1000000 * (1.0 / FFT_SAMPLING_FREQ));
}

void loop() {
  rotaryLoop();

  // for (byte ledStripNumber = 0; ledStripNumber < LED_STRIP_NUMBERS; ledStripNumber++) {};
  byte ledStripNumber = 0; // only the one led matrix

  switch(LED_SETTINGS_VALUE[ledStripNumber][0]) {
    case(0): // Equalizer
      buildEqualizer(ledStripNumber);
      break;
    case(1): // Time
      buildTime(ledStripNumber);
      break;
    case(2): // Light
      buildLight(ledStripNumber);
      break;
    case(3): // Game
      // buildGame(ledStripNumber);
      break;
  }

  EVERY_N_MILLISECONDS(TIMER_DECAY_PEAK) {
    for (byte band = 0; band < LED_NUM_BANDS; band++) if (peak[band] > 0) peak[band] -= 1;
    colorTimer++;
  }

  EVERY_N_MILLISECONDS(TIMER_COLOR_CHANGE) {
    colorTimer++;
  }

  // autochange
  EVERY_N_MILLISECONDS(TIMER_AUTO_CHANGE) {
    for (byte led = 0; led < LED_STRIP_NUMBERS; led++) {
      if (LED_SETTINGS_VALUE[led][4]) {
        // peaks
        LED_SETTINGS_VALUE[led][2] = random(1, 3);
        // equalizerMode
        LED_SETTINGS_VALUE[led][3] = random(1, 6);
      }
    }
  }

  FastLED.show();
}

void buildTime(byte led_strip_number) {
  const String timeString = "14:55:44"; // TODO get time from web

  matrix->setTextWrap(false);
  matrix->setFont(&TomThumb);
  matrix->setTextColor(matrix->Color(255, 255, 255));
  matrix->fillScreen(0);
  matrix->setCursor(3, 10);  // Start X and Y position
  matrix->print(timeString);
}

void buildLight(byte led_strip_number) {
  for (int y = 0; y < LED_MATRIX_HEIGHT; y++) {
    for (int x = 0; x < LED_MATRIX_WIDTH; x++) {
      matrix->drawPixel(x, y, CHSV(0, 0, 255));
    }
  }
}

// Rotary encoder
void rotaryOnButtonClick() {
  static unsigned long lastTimePressed = 0;
  //ignore multiple press in that time milliseconds
  if (millis() - lastTimePressed < ROTARY_ENCODER_NOISE) {
    return;
  }

  lastTimePressed = millis();

  if (ledSettingProperty == 6) {
    // exit to change led level
    encoderMode = 0;
    encoderBtnMode = 1;
    ledSettingProperty = 0;
    rotaryEncoder.setEncoderValue(currentSetting);
  } else if (encoderBtnMode) {
      // CONFIRM changed value
      rotaryEncoder.setEncoderValue(ledSettingProperty);
      encoderMode = 1;
      encoderBtnMode = 0;
  } else {
      // SELECT
      rotaryEncoder.setEncoderValue(LED_SETTINGS_VALUE[currentSetting][ledSettingProperty]);
      encoderMode = 2;
      encoderBtnMode = 1;
  }

  renderOLED();
}

void changeLed(byte value) {
  currentSetting = value;

  currentSetting = currentSetting > LED_STRIP_NUMBERS ? LED_STRIP_NUMBERS : currentSetting;
  currentSetting = currentSetting < 0 ? 0 : currentSetting;
}

void changeMode(byte value) {
  ledSettingProperty = value;

  ledSettingProperty = ledSettingProperty > settingsPropsLength ? settingsPropsLength : ledSettingProperty;
  ledSettingProperty = ledSettingProperty < 0 ? 0 : ledSettingProperty;

  newEncoderValue = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty];
}

byte changeModeValue(byte value) {
    switch (ledSettingProperty) {
      case 0:   // MODE
        value = value > 3 ? 3 : value;
        value = value < 0 ? 0 : value;
        break;
      case 1:   // BRIGHTNESS
        FastLED.setBrightness(BRIGHTNESS_SETTINGS[value]);
        break;
      case 2:   // PEAKS
        value = value > 3 ? 3 : value;
        value = value < 1 ? 1 : value;
        break;
      case 3:   // EQUALIZER_MODE
        value = value > 6 ? 6 : value;
        value = value < 1 ? 1 : value;
        break;
      case 4:   // AUTO_CHANGE
        value = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty] == 1 ? 0 : 1;
        break;
      case 5:   // NOISE_LEVEL
        value = value > 0 ? value : 1;
        break;
      case 6:   // EXIT
        break;
    }

    if (ledSettingProperty != 6) LED_SETTINGS_VALUE[currentSetting][ledSettingProperty] = value;

    return value;
}

void rotaryLoop() {
  if (rotaryEncoder.isEncoderButtonClicked()) {
    rotaryOnButtonClick();
  } else if (rotaryEncoder.encoderChanged()) {
    byte encoderValue = rotaryEncoder.readEncoder();

    switch(encoderMode) {
      // Select LED
      case(0) :
        changeLed(encoderValue);
        encoderBtnMode = 1;
        break;
      // Scroll Settings
      case(1) :
        changeMode(encoderValue);
        encoderBtnMode = 0;
        break;
      // Change value
      case(2) :
        newEncoderValue = changeModeValue(encoderValue);
        encoderBtnMode = 1;
        break;
    }

    renderOLED();
  }
}
// Rotary encoder END

void renderOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(SETTINGS[currentSetting]);

  if (currentSetting == 4) {
    // Logs mode
    display.setTextSize(1);
    display.println("logs will be here");
  } else if (encoderMode == 0) {
    // LED info
    display.setTextSize(1);
    display.println();
    display.println("------------------");
    display.println(LED_MODE[LED_SETTINGS_VALUE[currentSetting][0]]);
  } else if (encoderMode == 1) {
    // LED settings menu
    display.setTextSize(1);
    display.println();
    String prevMenuItem = ledSettingProperty > 0 ? ("   " + String(ledSettingProperty) + " " + LED_SETTINGS[(ledSettingProperty - 1)]) : "------------------";
    String nextMenuItem = ledSettingProperty < settingsPropsLength ? ("   " + String(ledSettingProperty + 2) + " " + LED_SETTINGS[(ledSettingProperty + 1)]) : "-----------------";
    prevMenuItem.toLowerCase();
    nextMenuItem.toLowerCase();
    display.println(prevMenuItem);
    display.println();
    display.println(">" + String(ledSettingProperty + 1) + " " + LED_SETTINGS[ledSettingProperty] + ": " + String(LED_SETTINGS_VALUE[currentSetting][ledSettingProperty]));
    display.println();
    display.println(nextMenuItem);
  } else if (encoderMode == 2) {
    // Change value
    display.setTextSize(1);
    display.println();
    display.println();
    display.print(LED_SETTINGS[ledSettingProperty] + ": ");
    if (ledSettingProperty == 0) {
      display.println(newEncoderValue);
      display.setTextSize(2);
      display.println(LED_MODE[newEncoderValue]);
    } else {
      display.setTextSize(2);
      display.println(newEncoderValue);
    }
  }

  display.display();
}

void buildEqualizer(byte led_strip_number) {
  // settings
  byte peaksMode = LED_SETTINGS_VALUE[led_strip_number][2];
  byte equalizerMode = LED_SETTINGS_VALUE[led_strip_number][3];
  byte noiseLevel = LED_SETTINGS_VALUE[led_strip_number][5];

  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  if (equalizerMode != 6) FastLED.clear();

  // Reset bandValues[]
  for (int i = 0; i < LED_NUM_BANDS; i++) {
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
  for (byte band = 0; band < LED_NUM_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = bandValues[band] / (FFT_AMPLITUDE * noiseLevel);
    if (barHeight > LED_TOP_ELEMENT) barHeight = LED_TOP_ELEMENT;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(LED_TOP_ELEMENT, barHeight);
    }

    // Draw bars
    switch (equalizerMode) {
      case 1:
        rainbowBars(band, barHeight);
        break;
      case 2:
        // No bars on this one
        break;
      case 3:
        purpleBars(band, barHeight);
        break;
      case 4:
        centerBars(band, barHeight);
        break;
      case 5:
        changingBars(band, barHeight);
        break;
      case 6:
        waterfall(band);
        break;
    }

    // Draw peaks
    if (equalizerMode != 4 && equalizerMode != 6) {
      switch (peaksMode) {
        case 1:
          whitePeak(band);
          break;
        case 2:
          outrunPeak(band);
          break;
        case 3:
          whitePeak(band);
          break;
      }
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
}

// PATTERNS
void rainbowBars(int band, int barHeight) {
  int xStart = LED_BAR_WIDTH * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    for (int y = LED_TOP_ELEMENT; y >= LED_TOP_ELEMENT - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV((x / LED_BAR_WIDTH) * (255 / LED_NUM_BANDS), 255, 255));
    }
  }
}

void purpleBars(int band, int barHeight) {
  int xStart = LED_BAR_WIDTH * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    for (int y = LED_TOP_ELEMENT; y >= LED_TOP_ELEMENT - barHeight; y--) {
      matrix->drawPixel(x, y, ColorFromPalette(purplePal, y * (255 / (barHeight + 1))));
    }
  }
}

void changingBars(int band, int barHeight) {
  int xStart = LED_BAR_WIDTH * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    for (int y = LED_TOP_ELEMENT; y >= LED_TOP_ELEMENT - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV(y * (255 / LED_MATRIX_HEIGHT) + colorTimer, 255, 255));
    }
  }
}

void centerBars(int band, int barHeight) {
  int xStart = LED_BAR_WIDTH * band;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((LED_MATRIX_HEIGHT - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix->drawPixel(x, y, ColorFromPalette(heatPal, colorIndex));
    }
  }
}

void whitePeak(int band) {
  int xStart = LED_BAR_WIDTH * band;
  int peakHeight = LED_TOP_ELEMENT - peak[band] - 1;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    matrix->drawPixel(x, peakHeight, CHSV(0, 0, 255));
  }
}

void outrunPeak(int band) {
  int xStart = LED_BAR_WIDTH * band;
  int peakHeight = LED_TOP_ELEMENT - peak[band] - 1;
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    matrix->drawPixel(x, peakHeight, ColorFromPalette(outrunPal, peakHeight * (255 / LED_MATRIX_HEIGHT)));
  }
}

void waterfall(int band) {
  int xStart = LED_BAR_WIDTH * band;
  double highestBandValue = 60000;  // to calibrate the waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + LED_BAR_WIDTH; x++) {
    matrix->drawPixel(x, 0, CHSV(constrain(map(bandValues[band], 0, highestBandValue, 160, 0), 0, 160), 255, 255));
  }

  // Move screen up starting at 2nd row from top
  if (band == LED_NUM_BANDS - 1) {
    for (int y = LED_MATRIX_HEIGHT - 2; y >= 0; y--) {
      for (int x = 0; x < LED_MATRIX_WIDTH; x++) {
        int pixelIndexY = matrix->XY(x, y + 1);
        int pixelIndex = matrix->XY(x, y);
        leds[pixelIndexY] = leds[pixelIndex];
      }
    }
  }
}
