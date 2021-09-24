/*
*  WiFiQuick_HomeAssistant-sensor.ino  (c) 2021 Winford (Uncle Grumpy)
*
*  This demo shows how to use WiFiQuick Library with the home-assistant-integration library
*  to create a simple sensor node that will wake up and report in to HomeAssistant and then
*  go back to sleep.
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
#include <ArduinoHA.h>

WiFiQuick WiFiQuick;  // Use WiFiQuick as WiFiQuick (use any name you like... "wl", "wfq", or even "WiFi" if you're crazy)

uint32_t SleepSecs = 30;

const char* ssid = "NETWORK";
const char* password = "PASSWORD";
#define BROKER_ADDR IPAddress(192,168,0,3)
#ifndef WL_MAC_ADDR_LENGTH
  #define WL_MAC_ADDR_LENGTH 6
#endif

unsigned long lastSentAt = millis();
double lastValue = 0;


WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensor wireless("ESP-WiFi-Signal"); // "wireless" is unique ID of the sensor. You should define your own ID.
HASensor timer("ESP-Connect-Timer");
HASensor counter("ESP-Wake-Counter");

void onBeforeSwitchStateChanged(bool state, HASwitch* s)
{
    // this callback will be called before publishing new state to HA
    // in some cases there may be delay before onStateChanged is called due to network latency
}

void setup() {
    Serial.begin(115200);
    delay(5000);
    WiFiQuick.UpdateWakes(); // incriment the wake counter.
    Serial.println();
    delay(5);
    Serial.println();
    delay(1);
    
    // Unique ID must be set fo HA!
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFiQuick.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));
    
    WiFiQuick.begin(ssid, password);
    
    // set device's details
    device.setName("ESP32-node1");
    device.setSoftwareVersion("0.2.1");
    device.setManufacturer("Uncle Grumpy");
    device.setModel("nodemcu-esp32-s");

    // configure sensor

    wireless.setUnitOfMeasurement("dB");
    wireless.setIcon("mdi:wifi");
    wireless.setName("ESP32-s WiFi Signal");
    timer.setUnitOfMeasurement("ms");
    timer.setIcon("mdi:timer");
    timer.setName("ESP32-s Connect Timer");
    counter.setIcon("mdi:counter");
    counter.setName("ESP32-s Wake Counter");

    mqtt.begin(BROKER_ADDR);
    mqtt.loop();  // run this once here to publish device info before sensor readings.
}

void loop() {
  //mqtt.loop();
  uint32_t nap = SleepSecs * 1e6;
  float sigLevel = WiFi.RSSI();

  wireless.setValue(sigLevel);
  timer.setValue(WiFiQuick::authTimer);
  counter.setValue(WiFiQuick.WakeCount());
  delay(10);
  mqtt.loop();
  delay(1000);
  WiFi.disconnect( true );
  delay(1);
  WiFi.mode( WIFI_OFF );
  delay(1);
  Serial.println();
  delay(1);
  #ifdef ESP32
    esp_sleep_enable_timer_wakeup(nap);
    esp_deep_sleep_start();
  #endif
  #ifdef ESP8266
    ESP.deepSleep(nap);
  #endif
  delay(1);    
 
}
