bool isBST(int dayCount, int monthCount, int weekCount) {
  if (monthCount > 3 && monthCount < 10) return true;
  if (monthCount < 3 || monthCount > 10) return false;

  if (monthCount == 3) {
    int lastSunday = dayCount - weekCount;

    return lastSunday >= 25;
  }

  if (monthCount == 10) {
    int lastSunday = dayCount - weekCount;

    return lastSunday < 25;
  }

  return false;
}

void updateTimeOffset() {
  // Update DST offset based on current date
  unsigned long epochTime = timeClient.getEpochTime();
  int dayCount = day(epochTime);
  int monthCount = month(epochTime);
  int weekCount = weekday(epochTime) - 1;

  if (isBST(dayCount, monthCount, weekCount)) {
    timeClient.setTimeOffset(TIME_ZONE_OFFSET);  // Add 1 hour for BST
  } else {
    timeClient.setTimeOffset(0);  // UTC for GMT
  }
}
