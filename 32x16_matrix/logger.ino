void logLn(String text) {
  Serial.println(text);
}

void log(String text) {
  Serial.print(text);
}

void timelogLn(String text) {
  Serial.println(timeString + ": " + text);
}

void timelog(String text) {
  Serial.print(timeString + ": " + text);
}