#include <FastLED_NeoMatrix.h>
#include <Fonts/TomThumb.h>
#include <arduinoFFT.h>
#include <AiEsp32RotaryEncoder.h>
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <FS.h>
#include <SPIFFS.h>
#include <OneButton.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

#include <wifiConfigPage.h>
#include <wifiJoystickPage.h>

#define BOARD_LED_PIN 2    // Built-in LED on most ESP32 boards 2 pin
#define AUDIO_IN_PIN 35    // Audio signal
#define SENSOR_BTN_PIN 32  // Touch btn

#define FFT_SAMPLES 512          // Must be a power of 2
#define FFT_SAMPLING_FREQ 24000  // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define FFT_AMPLITUDE 10000      // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define FFT_FILTERED_NOISE 1000  // Used as a crude noise filter, values below this are ignored

#define LED_MAX_BANDS 16         // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define LED_STRIP_NUMBERS 4      // Amount of strips or matrix
#define LED_MAX_MILLI_AMPS 9000  // Be careful with the amount of power here if running from USB port
#define LED_VOLTS 5              // Usually 5 or 12

#define LED_1_PIN 33           // LED strip data
#define LED_1_COLOR_ORDER GRB  // If colours look wrong, play with this
#define LED_1_CHIPSET WS2812B  // LED strip type
#define LED_1_MATRIX_HEIGHT 16
#define LED_1_MATRIX_WIDTH 32
#define LED_1_QUANTITY (LED_1_MATRIX_WIDTH * LED_1_MATRIX_HEIGHT)   // Total number of LEDs
#define LED_1_BAR_WIDTH (LED_1_MATRIX_WIDTH / (LED_MAX_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define LED_1_TOP_ELEMENT (LED_1_MATRIX_HEIGHT - 0)                 // Don't allow the bars to go offscreen

#define LED_2_PIN 25           // LED strip data
#define LED_2_COLOR_ORDER GRB  // If colours look wrong, play with this
#define LED_2_CHIPSET WS2812B  // LED strip type
#define LED_2_MATRIX_HEIGHT 14
#define LED_2_MATRIX_WIDTH 14
#define LED_2_QUANTITY (LED_2_MATRIX_WIDTH * LED_2_MATRIX_HEIGHT)   // Total number of LEDs
#define LED_2_BAR_WIDTH (LED_2_MATRIX_WIDTH / (LED_MAX_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define LED_2_TOP_ELEMENT (LED_2_MATRIX_HEIGHT - 0)                 // Don't allow the bars to go offscreen

#define LED_3_PIN 26  // LED strip data

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

#define AP_WIFI_SSID "MATRIX_32x16"    // Access Point SSID
#define AP_WIFI_PASSWORD "0123456789"  // Access Point Password

#define TIME_ZONE_OFFSET 3600            // +1 hour UK offset to UTC
#define CONFIRM_CLICK_ACTION_TIME 10000  // to confirm action
#define UPDATE_TIME_LOOP 1000

int PHOTO_RESISTOR_PIN = 34;  // by some reason must be an int value

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
byte encoderValue = 1;

// Settings
const byte BRIGHTNESS_SETTINGS[(ROTARY_ENCODER_MAX_VALUE + 1)] = { 0, 1, 2, 5, 10, 20, 40, 60, 80, 100, 125, 150, 175, 200, 225, 250 };
const String ENCODER_MODE[3] = { "DISPLAY", "SCROLL", "CHANGE VALUE" };
byte encoderMode = 0;
const String ENCODER_BTN_MODE[3] = { "SELECT LED", "SELECT", "CONFIRM" };
byte encoderBtnMode = 0;
const String SETTINGS[LED_STRIP_NUMBERS + 1] = { "LED 1", "LED 2", "LED 3", "LED 4", "LOGS" };  // LED_STRIP_NUMBERS + 1(Logs)
const byte settingsPropsLength = 7;
const String LED_SETTINGS[settingsPropsLength + 1] = { "MODE", "BRIGHTNESS", "PEAKS", "EQUALIZER MODE", "AUTO CHANGE", "NOISE LEVEL", "NIGHT MODE", "EXIT" };  // settingsPropsLength + 1(Exit)
const String LED_MODE[6] = { "EQUALIZER", "TIME", "LIGHT", "GAME", "FIRE 1", "FIRE 2" };

byte LED_SETTINGS_VALUE[LED_STRIP_NUMBERS][settingsPropsLength] = {
  { 0, 10, 1, 1, 0, 1, 1 },
  { 0, 10, 1, 1, 0, 1, 0 },
  { 0, 10, 1, 1, 0, 1, 0 },
  { 0, 10, 1, 1, 0, 1, 0 },
};
byte currentSetting = 0;
byte ledSettingProperty = 0;
byte newEncoderValue = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty];


// Sampling and FFT
unsigned int sampling_period_us;
byte peak[LED_MAX_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int oldBarHeights[LED_MAX_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int bandValues[LED_MAX_BANDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
unsigned long newTime;
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFT_SAMPLES, FFT_SAMPLING_FREQ);

// Create OLED display object
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// FastLED
CRGB ledsConfiguration1[LED_1_QUANTITY];

DEFINE_GRADIENT_PALETTE(purple_gp){
  0, 0, 212, 255,   //blue
  255, 179, 0, 255  //purple
};
DEFINE_GRADIENT_PALETTE(outrun_gp){
  0, 141, 0, 100,    //purple
  127, 255, 192, 0,  //yellow
  255, 0, 5, 255     //blue
};
DEFINE_GRADIENT_PALETTE(greenblue_gp){
  0, 0, 255, 60,     //green
  64, 0, 236, 255,   //cyan
  128, 0, 5, 255,    //blue
  192, 0, 236, 255,  //cyan
  255, 0, 255, 60    //green
};
DEFINE_GRADIENT_PALETTE(redyellow_gp){
  0, 200, 200, 200,   //white
  64, 255, 218, 0,    //yellow
  128, 231, 0, 0,     //red
  192, 255, 218, 0,   //yellow
  255, 200, 200, 200  //white
};
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

const int LED_TOP_ELEMENTS[LED_STRIP_NUMBERS] = { LED_1_TOP_ELEMENT, LED_2_TOP_ELEMENT, LED_1_TOP_ELEMENT, LED_1_TOP_ELEMENT };
const int LED_BAR_WIDTHS[LED_STRIP_NUMBERS] = { LED_1_BAR_WIDTH, LED_2_BAR_WIDTH, LED_1_BAR_WIDTH, LED_1_BAR_WIDTH };
const int LED_MATRIX_HEIGHTS[LED_STRIP_NUMBERS] = { LED_1_MATRIX_HEIGHT, LED_2_MATRIX_HEIGHT, LED_1_MATRIX_HEIGHT, LED_1_MATRIX_HEIGHT };
const int LED_MATRIX_WIDTHS[LED_STRIP_NUMBERS] = { LED_1_MATRIX_WIDTH, LED_2_MATRIX_WIDTH, LED_1_MATRIX_WIDTH, LED_1_MATRIX_WIDTH };

// FastLED_NeoMatrix - see https://github.com/marcmerlin/FastLED_NeoMatrix
FastLED_NeoMatrix* matrix = new FastLED_NeoMatrix(ledsConfiguration1, LED_1_MATRIX_WIDTH, LED_1_MATRIX_HEIGHT, 1, 1,
                                                  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0);

// Sensor button
OneButton sensorButton(SENSOR_BTN_PIN, false);
bool isReadyToResetWiFi = false;
bool isLoggerActive = false;
unsigned long resetCounterMillis = 0;

// WiFi connection
bool connectionDefined = false;
String savedSsid = "";
String savedPass = "";
WebServer server(80);
HTTPClient http;
WiFiClient client;

String timeString = "";
String gameBtnAction = "";

// has to be before setup()
void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  // Wait for 1 second before initializing Serial to start hardware SPIFFS
  delay(2000);
  Serial.begin(115200);

  // Initialize file system
  if (!SPIFFS.begin()) {
    logLn("Failed to mount file system");
    return;
  }

  pinMode(BOARD_LED_PIN, OUTPUT);
  digitalWrite(BOARD_LED_PIN, HIGH);

  // Initialize BUTTON
  sensorButton.attachClick(sensorButtonSingleClick);
  sensorButton.attachDoubleClick(sensorButtonDoubleClick);
  sensorButton.attachLongPressStop(sensorButtonLongPress);

  // Initialize OLED display (with I2C address 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  display.clearDisplay();
  renderOLED();

  // rotary encoder
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


  // LED
  FastLED.addLeds<LED_1_CHIPSET, LED_1_PIN, LED_1_COLOR_ORDER>(ledsConfiguration1, LED_1_QUANTITY).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, LED_MAX_MILLI_AMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[LED_SETTINGS_VALUE[currentSetting][1]]);
  FastLED.clear();

  sampling_period_us = round(1000000 * (1.0 / FFT_SAMPLING_FREQ));

  logLn("SPIFFS is mounted");
  connectionDefined = false;                      // Initial Connection state before checking saved data
  File configFile = SPIFFS.open("/config.json");  // Read Configuration data from the JSON file

  if (!configFile) {
    logLn("Failed to open config file for reading");
  } else {
    String jsonStr = configFile.readString();  // Read the contents of the file into a string
    Serial.println("jsonStr");
    Serial.println(jsonStr);
    configFile.close();                      // Close the file to reduce memory usage
    JSONVar jsonData = JSON.parse(jsonStr);  // Deserialize the JSON string into an Arduino_JSON object

    if (!JSON.typeof(jsonData) || JSON.typeof(jsonData) == "undefined") {
      logLn("Failed to parse JSON data");
    } else {
      const char* ssidChar = jsonData["ssid"];
      const char* passChar = jsonData["pass"];

      savedSsid = String(ssidChar);
      savedPass = String(passChar);

      connectionDefined = savedSsid.length() && savedPass.length() ? true : false;
      logLn("connectionDefined: " + String(connectionDefined));
    }
  }

  // If no Config, run initial page with a wifi form
  // Setting the AP Mode with SSID, Password, and Max Connection Limit
  if (!connectionDefined) {
    logLn("Create a WiFi AP");

    if (WiFi.softAP(AP_WIFI_SSID, AP_WIFI_PASSWORD) == true) {
      logLn("WEB: " + WiFi.softAPIP().toString());
      logLn("WIFI: " + String(WiFi.softAPSSID()) + "   PASS: " + String(AP_WIFI_PASSWORD));
      WiFi.mode(WIFI_AP);
    } else {
      logLn("Unable to Create AP");
    }
  } else {
    // Connect to Wi-Fi
    if (connectToWifi(savedSsid, savedPass)) {
      // Start NTP client
      timeClient.begin();
      delay(1000);         // wait time from server
      updateTimeOffset();  // set summer/winter time
      timeClient.update();
      timeString = timeClient.getFormattedTime();
      Serial.println(WiFi.localIP());
    };
  }

  startServerListener();
}

void loop() {
  server.handleClient();
  rotaryLoop();

  for (byte ledStripNumber = 0; ledStripNumber < LED_STRIP_NUMBERS; ledStripNumber++) {
    // no ways to manage ledsConfiguration and matrix in the arrays, esp32 memory allocation issue due to the huge objects
    // TODO investigate and fix, get ledsConfiguration and matrix by iterator
    switch (LED_SETTINGS_VALUE[ledStripNumber][0]) {
      case (0):  // Equalizer
        switch (ledStripNumber) {
          case (0):
            buildEqualizer(ledStripNumber, ledsConfiguration1, matrix);
            break;
        }
        break;
      case (1):  // Time
        switch (ledStripNumber) {
          case (0):
            buildTime(ledStripNumber, timeString, matrix);
            break;
        }
        break;
      case (2):  // Light
        switch (ledStripNumber) {
          case (0):
            buildLight(ledStripNumber, matrix);
            break;
        }
        break;
      case (3):  // Game
        buildGame(ledStripNumber, ledsConfiguration1, matrix, gameBtnAction);
        gameBtnAction = " ";
        break;
      case (4):  // Full screen Fireplace
        fireplace(ledStripNumber, ledsConfiguration1, matrix);
        delay(30);
        break;
      case (5):  // Fireplace
        drawCampfire(ledStripNumber, ledsConfiguration1, matrix);
        delay(30);
        break;
    }

    FastLED.show();
  }

  EVERY_N_MILLISECONDS(TIMER_DECAY_PEAK) {
    for (byte band = 0; band < LED_MAX_BANDS; band++)
      if (peak[band] > 0) peak[band] -= 1;
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

  // update time
  EVERY_N_MILLISECONDS(UPDATE_TIME_LOOP) {
    timeString = timeClient.getFormattedTime();

    if (LED_SETTINGS_VALUE[0][6]) {
      int photoResistorValue = analogRead(PHOTO_RESISTOR_PIN);  // 0 - 4095
      int newBrightness = map(photoResistorValue, 0, 4095, 1, 12);
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[newBrightness]);
    }
  }

  EVERY_N_MILLISECONDS(CONFIRM_CLICK_ACTION_TIME) {
    if (isReadyToResetWiFi) {
      isReadyToResetWiFi = false;

      logLn("Reset canceled by time");
    }
  }

  EVERY_N_MILLISECONDS(UPDATE_TIME_LOOP * 60) {
    timeClient.update();
  }
}
