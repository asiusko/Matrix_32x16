void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}

void satrtFireplaceAudio() {
  audio.connecttoFS(SD, "/fireplace.mp3");
  delay(1000);
}

void startStopAudio(bool isPlaying) {
  if (isPlaying && !audio.isRunning()) {
    satrtFireplaceAudio();
  } else if (!isPlaying && audio.isRunning()) {
    audio.stopSong();
  }  
}
