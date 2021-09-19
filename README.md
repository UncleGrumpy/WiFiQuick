# WiFiQuick
ESP32/8266 Platformio/Arduino library that painlessly enables incredibly fast re-connect to the previous wireless network after deep sleep.

This library uses RTC RAM to store all the settings, so it will be saved during sleep as long as power is not lost. This method is much faster than the native ESP-IDF/Arduino one, which saves some of this information in a special segment of flash. Not only is this method faster, but it also eliminates those write cycles, saving flash wear and lowering power consumption.  The biggest time saver is eliminating the need to do a network scan before connection.  A smaller but still not insignificant amount of time is saved by saving the previously issued ip address, gateway, netmask, and dns server so that dhcp is not used for connections after the first time.  The WiFiQuick.begin() has a default timout of 10 seconds, this can be changed by supplying a time in seconds as its final or only argument depending on how you choose to initiate the connection.  
## *Installation*
### ArduinoIDE:
* Clone or download and extract this repository into your Arduino libraries folder.
### Platformio:
* Install or add "winford/WiFiQuick" to your project using the pio library manager.
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

WiFiQuick WiFiQuick;

void setup() {

  // Start connection...
  WiFiQuick.begin(ssid, password);

}

void loop() {

  // You can safely add RF_DISABLED as a second argument to leave WiFi off
  // when the ESP first wakes up. WiFiQuick will turn it back on before connecting.
  ESP.deepSleep(10e6);
}
```
There are two basic methods of starting a connection. WiFiQuick.begin() should work very much like you would expect. Using this function like in the example above will start a connection.
#### Method 1
```
WiFiQuick.begin(ssid, pass);
```
 For Static IP it looks like:
```
WiFiQuick.begin(ssid, pass, IP, gateway, netmask, dns);
```
or for a longer timeout:
```
WiFiQuick.begin(ssid, pass, IP, gateway, netmask, dns, 30);  // 30 second timeout 
```
#### Method 2
You can start the connection, but not wait for it to finish.  This might be useful if you are trying to optimize your run time.  Just be sure to include delays to give time to the wifi to negotiate the connection, and avoid anything processor heavy.  This is started with the init() method, but must still include the begin() function to make sure your settings are stored for faster re-connection next time.
```
WiFiQuick.init(SSID, PASSWORD);  // starts the connection

/* do some other stuff... */

WiFiQuick.begin();	// returns "true" if connection is successful
			// saves your netinfo for faster connection next time.
```
 Or for static IP:
 ```
 WiFiQuick.init(SSID, PASSWORD, IP, gateway, netmask, dns);  // starts the connection

/* do some other stuff... */

WiFiQuick.begin(30) // 30 second timeout.
```
## *WARNING*
Make sure your board is set up to wake from DeepSleep! For example...
 
 *  D1 Mini > connect D0 to RST.
 *  ESP12-F > connect GPIO16 to RST
 *  ESP01 > see here: https://blog.enbiso.com/post/esp-01-deep-sleep/ to make the necessary modifications.
    * For this mod I personally like to use conductive paint and a sharp needle to apply it...
