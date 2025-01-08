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
