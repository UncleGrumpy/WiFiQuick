# WiFiQuick
ESP32/8266 Platformio/Arduino library that painlessly enables incredibly fast re-connect to the previous wireless network after deep sleep.

This library uses RTC RAM to store all the settings, so it will be saved during sleep as long as power is not lost. This method is much faster than the native ESP-IDF/Arduino one, which saves some of this information in a special segment of flash. Not only is this method faster, but it also eliminates those write cycles, saving flash wear and lowering power consumption.  The biggest time saver is eliminating the need to do a network scan before connection.  A smaller but still not insignificant amount of time is saved by saving the previously issued ip address, gateway, netmask, and dns server so that dhcp is not used for connections after the first time.  If a connection fails the device will sleep for 1 minute for each missed connection in a row.  So if you miss five conections in a row it will wait 5 minutes before attempting to reconnect. This is done to save battery power in case there is a problem with the network, like a power outage and the router is down... no point using up your batteries trying to connect to a router that isn't listening.

## *Installation*
### ArduinoIDE:
* Clone or download and extract this repository into your Arduino libraries folder.
### Platformio:
* add to project platformio.ini file like:
```
lib_deps = 
	https://github.com/UncleGrumpy/WiFiQuick.git
```

## *Usage*
A growing collection of samples are included. In ArduinoIDE they will show up in the usual menu:
> File > Examples > WiFiQuick 

The simplest usage looks like:
```
#include <WiFiQuick.h>

const char* ssid = "NETWORK_NAME";
const char* password = "PASSWORD";

void setup() {
  Serial.begin(115200);
  delay(1000);
  // this will bein the connection
  uint32_t startWifi = WiFiInit(ssid, password);

  /* you can do other setup that does not require wifi yet here... */

  // this will make sure the connection is established within 10 seconds
  // (or supply a second argument specifying the timeout period in seconds)
  // After the connection is established all the necessary data will be saved
  // during this function call for super fast reconnection when we wake up.
  WiFiTimeout(startWifi);
  
  /* Finish setup that requires wifi active... */
}

void loop() {

  /* Do whatever you need to do... */

  // You can safely add RF_DISABLED as a second argument to leave WiFi off
  // when the ESP first wakes up. WiFiInit() will turn it back on.
  ESP.deepSleep(10e6);
}
```

