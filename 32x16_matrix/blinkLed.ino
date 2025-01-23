void blinkLed(int times = 1, int delayTime = 200) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BOARD_LED_PIN, LOW);
    delay(delayTime);
    digitalWrite(BOARD_LED_PIN, HIGH);
    delay(delayTime);
  }
}
