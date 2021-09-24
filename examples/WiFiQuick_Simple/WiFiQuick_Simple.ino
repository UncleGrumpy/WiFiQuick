/*
 *  WiFiQuick_Simple.ino  (c) 2021 Winford (Uncle Grumpy)
 * 
 *  This demo shows how to use WiFiQuick Library to start a wireless connection that will be
 *  saved to RTC RAM so that later connections after waking from deep sleep can be completed
 *  much faster.  The first connection often takes 8-9 seconds, using this library post-sleep
 *  connections can be as fast as 1-2 seconds.
 * 
 *  WARNING: Make sure your board is set up to wake from DeepSleep! For example...
 *
 *  D1 Mini > connect D0 to RST.
 *  ESP12-F > connect GPIO16 to RST
 *  ESP01 > see here: https://blog.enbiso.com/post/esp-01-deep-sleep/ to make the necessary
 *           modifications.  For this mod I personnaly like to use conductive paint and a
 *           sharp needle to apply it...
 *
*/

#include <WiFiQuick.h>


const char* ssid = "NETWORK";
const char* password = "PASSWORD";
uint32_t SleepSecs = 20;    // how long to sleep. 20 seconds default for this demo.

WiFiQuick WiFiQuick;


void setup() {
  String resetCause;
  #ifdef ESP32
    esp_sleep_wakeup_cause_t resetWhy;
    resetWhy = esp_sleep_get_wakeup_cause();
    if (resetWhy == 4) {
    resetCause = "Deep-Sleep Wake";
    } else {
    resetCause = "Not-Deep-Sleep-Wake";
  }
  #elif ESP8266
    resetCause = ESP.getResetReason();
  #endif
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  delay(5);
  delay(1);
  Serial.println(resetCause);
  delay(1);
  WiFiQuick.UpdateWakes();
  Serial.print("run number ");
  Serial.println(WiFiQuick.WakeCount());
  delay(1);
  
  if (!WiFiQuick.begin(ssid, password, 15)) {
    Serial.println("WiFi connection Failed!");
  } else {
    Serial.print("Connected in ");
    Serial.print(WiFiQuick::authTimer);
    Serial.println("ms.");
  }

}

void loop() {
  uint32_t nap = SleepSecs * 1e6;
  Serial.println();
  Serial.print("WiFi Signal strength: ");
  Serial.println(WiFi.RSSI());
  delay(1);
  Serial.print("My IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(1);

  WiFi.disconnect( true );
  delay(1);
  WiFi.mode( WIFI_OFF );
  Serial.print("going to sleep for ");
  Serial.print(SleepSecs);
  Serial.println(" seconds...");
  delay(1);
  Serial.println();
  delay(1);
  #ifdef ESP32
    esp_sleep_enable_timer_wakeup(nap);
    esp_deep_sleep_start();
  #elif ESP8266
    ESP.deepSleep(nap);
  #endif
  delay(1);
}
