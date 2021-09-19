#include <WiFiQuick.h>
#include <ArduinoHA.h>

WiFiQuick WiFiQuick;

uint32_t SleepSecs = 30;
uint32_t ConTime;

const char* ssid = "NETGEAR_EXT";
const char* password = "uncledan";
#define BROKER_ADDR IPAddress(192,168,0,3)

unsigned long lastSentAt = millis();
double lastValue = 0;
ADC_MODE(ADC_VCC);

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensor volts("ESP01-Vcc"); // "volts" is unique ID of the sensor. You should define your own ID.
HASensor wireless("ESP01-WiFi-Signal"); // "wireless" is unique ID of the sensor. You should define your own ID.
HASensor timer("ESP01-Connect-Stopwatch");

void onBeforeSwitchStateChanged(bool state, HASwitch* s)
{
    // this callback will be called before publishing new state to HA
    // in some cases there may be delay before onStateChanged is called due to network latency
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println();
    delay(5);
    Serial.println();
    delay(1);
    
      // Unique ID must be set!
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFiQuick.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));
    
    Serial.print("UUID = ");
    for (unsigned int mem = 0; mem < 6; mem++) {
      Serial.print(mac[mem], HEX);
    }
    Serial.println();
    
    // you don't need to verify return status
    uint32_t startWifi = millis();
    WiFiQuick.begin(ssid, password);
    //WiFiTimeout(startWifi);
    ConTime = startWifi - millis();
    
    // set device's details (optional)
    device.setName("ESP-D1");
    device.setSoftwareVersion("B-0.1.0");
    device.setManufacturer("Uncle Grumpy");
    device.setModel("test-d1");

    // configure sensor (optional)
    volts.setUnitOfMeasurement("V");
    volts.setDeviceClass("voltage");
    volts.setIcon("mdi:battery-plus");
    volts.setName("ESP Vcc");
    wireless.setUnitOfMeasurement("dB");
    //wireless.setDeviceClass("Signal");
    wireless.setIcon("mdi:wifi");
    wireless.setName("ESP WiFi Signal");
    timer.setUnitOfMeasurement("ms");
    timer.setIcon("mdi:timer");
    timer.setName("ESP Connect Timer");

    mqtt.begin(BROKER_ADDR);
}

void loop() {
  mqtt.loop();
  uint32_t nap = SleepSecs * 1e6;
  float espVoltage = ESP.getVcc() * .001;
  float sigLevel = WiFi.RSSI();

  volts.setValue(espVoltage);
  wireless.setValue(sigLevel);
  timer.setValue(ConTime);
  delay(10);
  mqtt.loop();
  delay(1000);
  WiFi.disconnect( true );
  delay(1);
  WiFi.mode( WIFI_OFF );
  delay(1);
  Serial.println();
  delay(1);
  ESP.deepSleep(nap);
  delay(1);    
 
}
