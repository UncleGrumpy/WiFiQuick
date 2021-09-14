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


const char* ssid = "NETWORK_NAME";
const char* password = "PASSWORD";

uint32_t SleepSecs = 20;    // how long to sleep. 20 seconds default for this demo.


void setup() {
  uint32_t setupStart = millis();
  String resetCause = ESP.getResetReason();
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  delay(5);
  Serial.println();
  delay(1);
  Serial.println(resetCause);
  delay(1);
  UpdateResetCount();
  Serial.print("run number ");
  Serial.println(GetResetCount());
  delay(1);
  
  uint32_t startWifi = WiFiInit(ssid, password); // this starts the wifi connection.
  
  /*
   *  you can do some other setup stuff that does not reqire a connection here.
   */

  // you must pass the startWifi returned by WiFiInit and an optional timeout in seconds
  // (default is 10), if a connection is not completed in this time the device will go to
  // sleep and try again after 60 seconds multiplied by the number of missed connections.
  // this is done so that battery powered devices do not waist power trying to connect to
  // the network if there is a problem or it isn't there, like during a power outage.
  /* Without calling this function you should still end up with a working connection, but the
  // network configs for fast reconnect will not be saved and you will have to perform some 
  // other check to see if you have a connection yet.
  */

  WiFiTimeout(startWifi);

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
  ESP.deepSleep(nap);
  delay(1);

}
