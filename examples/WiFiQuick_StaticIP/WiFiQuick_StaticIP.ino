/*
 *  WiFiQuick_StaticIP.ino  (c) 2021 Winford (Uncle Grumpy)
 * 
 *  This demo shows how to use WiFiQuick Library to start a wireless connection with a static 
 *  IP address that will be saved to RTC RAM so that later connections after waking from deep
 *  sleep can be completed much faster.  The first connection often takes 8-9 seconds, using 
 *  this library post-sleep connections can be as fast as 1-2 seconds.
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

uint32_t SleepSecs = 20;    // how long to sleep. 20 seconds default for this demo.
const char* ssid = "NETWORK";
const char* password = "PASSWORD";
IPAddress MYip(192, 168, 82, 66);
IPAddress MYgateway(192, 168, 0, 1);
IPAddress MYnetmask(255, 255, 0, 0);
IPAddress MYdns(192, 168, 0, 1); 

WiFiQuick WiFiQuick;

void setup() {
  String resetCause;
  #ifdef ESP32
    resetWhy = esp_sleep_get_wakeup_cause();
    if (resetWhy == 4) {
      resetCause = "Deep-Sleep Wake";
    } else {
      resetCause = "Not-Deep-Sleep-Wake";
    }
  #elif ESP826
    resetCause = ESP.getResetReason();
  #endif
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  delay(5);
  Serial.println();
  delay(1);
  Serial.println(resetCause);
  delay(1);
  WiFiQuick.UpdateWakes();
  Serial.print("run number ");
  Serial.println(WiFiQuick.WakeCount());
  delay(1);


  // You can pass the IP, gateway, subnet, and dns after ssid & pass to setup a 
  // connection with a static IP address.  This is only necessary if you want to
  // specify the IP address. Setting a static IP here will only make the first
  // connection a little bit faster than using DHCP to request an IP from the
  // router.  On all connections after the first a static connection is created
  // using the IP settings from the last connection.
  WiFiQuick.init(ssid, password, MYip, MYgateway, MYnetmask, MYdns); // this starts the wifi connection.
  
  /*
   *  you can do some other setup stuff that does not reqire a connection here.
   */

  // the timeout argument is optional. default is 10 seconds.
  if (!WiFiQuick.begin(20)) {
    Serial.println("WiFi connection failed!");
  }  // 15 second timeout before giving up on connection.

}

void loop() {
  uint32_t nap = SleepSecs * 1e6;
  Serial.println();
  Serial.print("WiFi connection took: ");
  Serial.print(WiFiQuick::authTimer);
  Serial.println("ms.");
  Serial.println();
  Serial.print("WiFi Signal strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println("dB.");
  delay(1);
  Serial.print("My IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(1);

  WiFi.disconnect( false );
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
