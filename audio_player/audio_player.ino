/*
  ESP32 SD I2S Music Player
  Plays MP3 file from microSD card
  Uses MAX98357 I2S Amplifier Module
  Uses ESP32-audioI2S Library
*/

// #include "Arduino.h"
#include "Audio.h"
#include "SD.h"
// #include "FS.h"
#include "WiFi.h"

// microSD Card Reader connections
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25

// Create Audio object
Audio audio;
String ssid =     "Hometelecom9ZQG2G";
String password = "hqtG4yCyeyttZRZY";

void setup() {
  delay(4000);
    Serial.begin(115200);
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) delay(1500);
Serial.println("Init SD.");
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SD.begin(SD_CS);
    Serial.println("Init SD done.");

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(20);
    audio.setFileLoop(true);
    audio.forceMono(true);
    audio.connecttoFS(SD, "/fireplace.mp3");     // SD
    delay(1000);
    // audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Olsen-Banden.mp3");
}

void loop() {
      // Continuously process audio data
  audio.loop();

  // Optionally, monitor playback status
  if (audio.isRunning()) {
    // Playback is running
  } else {
    // Playback has stopped (e.g., end of file)    
    Serial.println("Playback finished.");
    audio.connecttoFS(SD, "/fireplace.mp3");
    delay(1000);
  }
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

