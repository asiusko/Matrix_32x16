void rotaryOnButtonClick() {
  static unsigned long lastTimePressed = 0;
  //ignore multiple press in that time milliseconds
  if (millis() - lastTimePressed < ROTARY_ENCODER_NOISE) {
    return;
  }

  lastTimePressed = millis();

  if (ledSettingProperty == settingsPropsLength) {
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
    case 0:  // LED_MODE
      value = value > 5 ? 5 : value;
      value = value < 0 ? 0 : value;
      break;
    case 1:  // BRIGHTNESS
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[value]);
      break;
    case 2:  // PEAKS
      value = value > 3 ? 3 : value;
      value = value < 1 ? 1 : value;
      break;
    case 3:  // EQUALIZER_MODE
      value = value > 6 ? 6 : value;
      value = value < 1 ? 1 : value;
      break;
    case 4:  // AUTO_CHANGE
      value = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty] == 1 ? 0 : 1;
      break;
    case 5:  // NOISE_LEVEL
      value = value > 0 ? value : 1;
      break;
    case 6:  // NIGHT MODE
      value = LED_SETTINGS_VALUE[currentSetting][ledSettingProperty] == 1 ? 0 : 1;
    case 7:  // EXIT
      break;
  }

  if (ledSettingProperty != 7) LED_SETTINGS_VALUE[currentSetting][ledSettingProperty] = value;

  return value;
}

void rotaryLoop() {
  if (rotaryEncoder.isEncoderButtonClicked()) {
    rotaryOnButtonClick();
  } else if (rotaryEncoder.encoderChanged()) {
    byte encoderValue = rotaryEncoder.readEncoder();

    switch (encoderMode) {
      // Select LED
      case (0):
        changeLed(encoderValue);
        encoderBtnMode = 1;
        break;
      // Scroll Settings
      case (1):
        changeMode(encoderValue);
        encoderBtnMode = 0;
        break;
      // Change value
      case (2):
        newEncoderValue = changeModeValue(encoderValue);
        encoderBtnMode = 1;
        break;
    }

    renderOLED();
  }
}
