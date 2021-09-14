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


const char* ssid = "NETGEAR_EXT";
const char* password = "uncledan";
IPAddress MYip(192, 168, 82, 66);
IPAddress MYgateway(192, 168, 0, 1);
IPAddress MYnetmask(255, 255, 0, 0);
IPAddress MYdns(192, 168, 0, 1); 

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


  // You can pass the IP, gateway, subnet, and dns after ssid & pass to setup a 
  // connection with a static IP address.  This is only necessary if you want to
  // specify the IP address. Setting a static IP here will only make the first
  // connection a little bit faster than using DHCP to request an IP from the
  // router.  On all connections after the first a static connection is created
  // using the IP settings from the last connection.
  uint32_t startWifi = WiFiInit(ssid, password, MYip, MYgateway, MYnetmask, MYdns); // this starts the wifi connection.
  
  /*
   *  you can do some other setup stuff that does not reqire a connection here.
   */

  // you must pass the startWifi returned by WiFiInit and an optional timeout in seconds
  // (default is 10), if a connection is not completed in this time the device will go to
  // sleep and try again after 60 seconds multiplied by the number of missed connections.

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
