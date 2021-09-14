/*
    WiFiQuick_OTA.ino  (c) 2021 Winford (Uncle Grumpy)

    This demo can be used to test that OTA updates do not disturb the saved WiFiQuick
    connection info or loss of other data (run counter) saved in RTC RAM.  For devices
    that do deepSleep it is not likely to do OTA this way, you would more likely download
    new firmware from an http(s) server but the underlying update mechanism works the same.
    This really just tests the ability to do OTA without loosing WiFiQuick data.
    
    WARNING: Make sure your board is set up to wake from DeepSleep! For example...

    D1 Mini > connect D0 to RST.
    ESP12-F > connect GPIO16 to RST
    ESP01 > see here: https://blog.enbiso.com/post/esp-01-deep-sleep/ to make the necessary
             modifications.  For this mod I personnaly like to use conductive paint and a
             sharp needle to apply it...

*/

#include <WiFiQuick.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif


const char* ssid = "NETGEAR_EXT";
const char* password = "uncledan";

uint32_t SleepSecs = 10;    // how long to sleep. 10 seconds default for this demo.
uint32_t WakeSecs = 120;    // how long to stay awake. 2 minutes default for this demo.

uint32_t nap = SleepSecs * 1e6;
uint32_t SleepNow = millis() + (WakeSecs * 1000);

void setup() {
  uint32_t setupStart = millis();
  String resetCause = ESP.getResetReason();
  Serial.begin(115200);
  delay(2000);
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
  // you can change this before doing OTA to make sure it changes but the run counter is not reset.
  Serial.println("Firmware Test 1");
  delay(1);

  uint32_t startWifi = WiFiInit(ssid, password); // this starts the wifi connection.

  // you can do some other setup stuff that does not reqire a connection here.

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp(8266/32)-[ChipID]
  ArduinoOTA.setHostname("wifiquick_demo");

  // Password defaults to none
  // ArduinoOTA.setPassword("safety-first");
  // or
  // ArduinoOTA.setPasswordHash("<MD5-OF-PASSWORD>");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  // you must pass the startWifi returned by WiFiInit and an optional timeout in seconds
  // (default is 10), if a connection is not completed in this time the device will go to
  // sleep and try again after 60 seconds multiplied by the number of missed connections.

  WiFiTimeout(startWifi, 15);

  ArduinoOTA.begin();

  Serial.print("WiFi Signal strength: ");
  Serial.println(WiFi.RSSI());
  Serial.println();
  delay(1);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(1);
  Serial.print("Ready for OTA update.");
  Serial.println();
  delay(1);

  uint32_t setupTime = millis() - setupStart;
  Serial.print("Setup took ");
  Serial.print(setupTime);
  Serial.println("ms.");
}

void loop() {
  if (millis() > SleepNow) {
    // Go to sleep at end of WakeSecs.
    // You can add RF_DISABLED as a second argument to deepSleep, it  will leave the wifi off
    // when the ESP first wakes up. This will save the most power if you have a longer seutp()
    // or want to take sensitive ADC readings with the wifi off.  The WiFiInit() function turns
    // the radio on before it starts a connection.
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    Serial.print("going to sleep for ");
    Serial.print(SleepSecs);
    Serial.println(" seconds.");
    delay(1);
    Serial.println();
    Serial.println();
    delay(1);
    ESP.deepSleep(nap, RF_DISABLED);
    delay(1);
  }
  
  ArduinoOTA.handle();
  
}
