/*
    WiFiQuick   (c) 2021 Winford (Uncle Grumpy)

    Library to facilitate fast reconnection to wifi. 
    Uses RTC user memory to store the necessary
    information to allow connection to the previous
    wifi network without the expense of
    doing a network scan.

*/
#include <Arduino.h>
#include "WiFiQuick.h"
#ifdef ESP32
  #include <WiFi.h>
#elif ESP8266
  #include <ESP8266WiFi.h>
#endif

//#define WQ_DEBUG
//#define WQ_SERIAL

#ifdef WQ_DEBUG
    #define WQ_SERIAL
#endif

bool rtcValid(void);
bool updateRTCcrc(void);
uint32_t crc32(const uint8_t *data, size_t length);


#ifdef ESP8266
struct nv_s {
  uint8_t OTAbootloaderCMD [128];  // Leave this space free! it is used by ota to install the new bootloader command. 
  uint32_t crc;  // =) Stored outside of the rtcMEM struct so we don't have to wory about offset when we calculate crc32 of the data.
  struct {
    // MUST be 4-byte aligned for crc to work!
    uint32_t rstCount;  // stores the Deep Sleep reset count
    uint32_t noWifi;     // stores the number of consecutive missed connections
    uint32_t channel;    // stores the wifi channel for faster no-scan connetion
    uint32_t bssid[6];   // stores mac address of AP for no-san connection
    uint32_t myIP[4];       // use last IP for faster static (no dhcp delay) connection
    uint32_t wlGateway[4];  // store gateway for static connection
    uint32_t wlSubNet[4];  // subnet for static connection
    uint32_t wlDNS[4];      // DNS for static connection
    //uint32_t userData[70];  // possible future usage...
  } rtcMEM;
};

  static nv_s* nv = (nv_s*)RTC_USER_MEM; // user RTC RAM area

  uint32_t WiFiQuick::MissedWiFi = nv->rtcMEM.noWifi;

#endif

#ifdef ESP32
  typedef struct {
    uint32_t crc;  // =) Stored outside of the rtcMEM struct so we don't have to wory about offset when we calculate crc32 of the data.
    struct {
      // MUST be 4-byte aligned for crc to work!
      uint32_t rstCount;  // stores the Deep Sleep reset count
      uint32_t noWifi;     // stores the number of consecutive missed connections
      uint32_t channel;    // stores the wifi channel for faster no-scan connetion
      uint32_t bssid[6];   // stores mac address of AP for no-san connection
      uint32_t myIP[4];       // use last IP for faster static (no dhcp delay) connection
      uint32_t wlGateway[4];  // store gateway for static connection
      uint32_t wlSubNet[4];  // subnet for static connection
      uint32_t wlDNS[4];      // DNS for static connection
    } MEM;
  } RTC;

  RTC_DATA_ATTR RTC rtc;
  #ifndef WL_MAC_ADDR_LENGTH
    #define WL_MAC_ADDR_LENGTH 6
  #endif

  uint32_t WiFiQuick::MissedWiFi = rtc.MEM.noWifi;

#endif


uint32_t WiFiQuick::resetCount = 0;
IPAddress WiFiQuick::useDHCP(0, 0, 0, 0);
uint32_t WiFiQuick::_wlStart = 0;
uint32_t WiFiQuick::authTimer = 0;

WiFiQuick::WiFiQuick() : useRTC(false), _MaxSecs(10) {};
WiFiQuick::~WiFiQuick() {};

bool updateRTCcrc(void) {  // updates the reset count CRC
  #ifdef ESP32
    rtc.crc = crc32((uint8_t*)&rtc.MEM, sizeof(rtc.MEM));
    return rtcValid();
  #elif ESP8266
    nv->crc = crc32((uint8_t*)&nv->rtcMEM, sizeof(nv->rtcMEM));
    return rtcValid();
  #endif
}

bool rtcValid(void) {
  bool valid;
  #ifdef ESP32
    uint32_t crc = crc32((uint8_t*)&rtc.MEM, sizeof(rtc.MEM));
    if( crc != rtc.crc ) {
  #elif ESP8266
    uint32_t crc = crc32((uint8_t*)&nv->rtcMEM, sizeof(nv->rtcMEM));
    if( crc != nv->crc ) {
  #endif
      valid = false;
    } else {
      valid = true;
    }
  return valid;
}

uint32_t WiFiQuick::init(const char* ssid, const char* password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  _wlStart = millis();
  uint8_t wifiID[6];
  uint32_t wifiCHAN;
  WiFi.persistent(false);
  #ifdef ESP32
    WiFi.setSleep(false);
  #elif ESP8266
    WiFi.forceSleepWake();
  #endif
  delay(1);
  WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
  WiFi.mode(WIFI_STA);
  // #ifdef ESP32
  //   WiFi.setTxPower(10);
  // #elif ESP8266
  //   WiFi.setOutputPower(10);
  // #endif
  //WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
  if ((useRTC == true) && (WiFiQuick::MissedWiFi == 0)) {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcValid());
    #endif
    for (unsigned int mem = 0; mem < 4; mem++) {
      #ifdef ESP32
        staticIP[mem] = rtc.MEM.myIP[mem];
      #elif ESP8266
        staticIP[mem] = nv->rtcMEM.myIP[mem];
      #endif
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      #ifdef ESP32
        gateway[mem] = rtc.MEM.wlGateway[mem];
      #elif ESP8266
        gateway[mem] = nv->rtcMEM.wlGateway[mem];
      #endif
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      #ifdef ESP32
        subnet[mem] = rtc.MEM.wlSubNet[mem];
      #elif ESP8266
        subnet[mem] = nv->rtcMEM.wlSubNet[mem];
      #endif
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      #ifdef ESP32
        dns[mem] = rtc.MEM.wlDNS[mem];
      #elif ESP8266
        dns[mem] = nv->rtcMEM.wlDNS[mem];
      #endif
    }
    for (unsigned int mem = 0; mem < 6; mem++) {
      #ifdef ESP32
        wifiID[mem] = rtc.MEM.bssid[mem];
      #elif ESP8266
        wifiID[mem] = nv->rtcMEM.bssid[mem];
      #endif
    }
    #ifdef ESP32
      wifiCHAN = rtc.MEM.channel;
    #elif ESP8266
      wifiCHAN = nv->rtcMEM.channel;
    #endif
    if (IPAddress(staticIP) != WiFiQuick::useDHCP) { // make sure they are not default = 0.0.0.0
      WiFi.config(staticIP, gateway, subnet, dns);
    }
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Reconnecting to previous network");
    #endif
    #ifdef WQ_DEBUG
    Serial.print(" using channel ");
    Serial.print(wifiCHAN);
    Serial.print(" and bssid  ");
    for (int mem = 0; mem < 6; mem++ ) {
      Serial.print(wifiID[mem], HEX);
      delay(1);
    }
    delay(1);
    #endif
    WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
    WiFi.begin(ssid, password, wifiCHAN, wifiID, true);
  } else {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcValid());
    Serial.print("wifiMissed = ");
    Serial.println(WiFiQuick::MissedWiFi);
    delay(1);
    #endif
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Connecting to network");
    delay(1);
    #endif
    if (IPAddress(staticIP) != WiFiQuick::useDHCP) {
      WiFi.config(staticIP, gateway, subnet, dns);
    }
    WiFi.begin(ssid, password);
  }
  return _wlStart;
}

bool WiFiQuick::begin(const char* ssid, const char* password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress dns, uint8_t MaxSecs) {
  bool result;
  WiFiQuick::init(ssid, password, staticIP, gateway, subnet, dns);
  if (WiFiQuick::begin(MaxSecs)) {
     result = true;
  } else {
    result = false;
  }
  return result;
}

bool WiFiQuick::begin(uint8_t MaxSecs) {
  uint32_t wifiMissed;
  uint32_t MaxTimeout = MaxSecs * 1000;
  uint32_t GiveUp = _wlStart + MaxTimeout;   // 10 seconds max before giving up.
  while ( (WiFi.status() != WL_CONNECTED) && (millis() < GiveUp) ) {
    delay(50);
    #ifdef WQ_SERIAL
    Serial.print(".");
    #endif
  }
  authTimer = millis() - _wlStart;
  if (WiFi.status() != WL_CONNECTED) {
    // char err[] = "WiFi connect failed. Retry in 60 Seconds.";
    delay(1);
    wifiMissed = WiFiQuick::MissedWiFi; // read the previous wifi fail count
    delay(1);
    wifiMissed++;
    #ifdef WQ_DEBUG
    Serial.print("SAVING WIFI MISSED #");
    Serial.println(wifiMissed);
    delay(1);
    #endif
    #ifdef ESP32
      rtc.MEM.noWifi = wifiMissed; // update the missed connection count and save to rtc
    #elif ESP8266
      nv->rtcMEM.noWifi = wifiMissed;
    #endif
    delay(1);
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.println("WiFi connect failed.");
    Serial.println();
    delay(1);
    #endif
    updateRTCcrc();
    delay(1);
    WiFiQuick::disconnect();
    delay(1);
    WiFi.mode(WIFI_OFF);
    return false;
  } else {
    #ifdef ESP32
      rtc.MEM.channel = WiFi.channel();
    #elif ESP8266
      nv->rtcMEM.channel = WiFi.channel();
    #endif
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Connected in ");
    Serial.print(authTimer);
    Serial.println("ms.");
    delay(1);
    #endif
    #ifdef WQ_DEBUG
    Serial.print("Wrote channel #");
    #ifdef ESP32
    Serial.println(rtc.MEM.channel);
    #elif ESP8266
      Serial.println(nv->rtcMEM.channel);
    #endif
    delay(1);
    #endif
    #ifdef ESP32
      rtc.MEM.noWifi = 0;   // reset missed connection counter.
    #elif ESP8266
      nv->rtcMEM.noWifi = 0;
    #endif
    delay(1);
    uint8_t* bss_id = WiFi.BSSID();
    #ifdef WQ_DEBUG
    Serial.print("Wrote network bssid > ");
    #endif
    for (unsigned int len = 0; len < WL_MAC_ADDR_LENGTH; len++ ) {
      #ifdef ESP32
        rtc.MEM.bssid[len] = bss_id[len];
      #elif ESP8266
        nv->rtcMEM.bssid[len] = bss_id[len];
      #endif
      #ifdef WQ_DEBUG
      #ifdef ESP32
        Serial.print(rtc.MEM.bssid[len], HEX);
      #elif ESP8266
        Serial.print(nv->rtcMEM.bssid[len], HEX);
      #endif
      #endif
    }
    IPAddress staticIP = WiFi.localIP();
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress dns = WiFi.dnsIP();
    for (unsigned int mem = 0; mem < 4; mem++) {
      #ifdef ESP32
        rtc.MEM.myIP[mem] = staticIP[mem];
        rtc.MEM.wlGateway[mem] = gateway[mem];
        rtc.MEM.wlSubNet[mem] = subnet[mem];
        rtc.MEM.wlDNS[mem] = dns[mem];
      #elif ESP8266
        nv->rtcMEM.myIP[mem] = staticIP[mem];
        nv->rtcMEM.wlGateway[mem] = gateway[mem];
        nv->rtcMEM.wlSubNet[mem] = subnet[mem];
        nv->rtcMEM.wlDNS[mem] = dns[mem];
      #endif
    }
    
    updateRTCcrc();
    #ifdef WQ_DEBUG
    Serial.println();
    Serial.print("Connected to ");
    Serial.print(WiFi.BSSIDstr());
    Serial.print("  -- Channel ");
    Serial.println(WiFi.channel());
    delay(1);
    #endif
  }
  return true;
}

bool WiFiQuick::disconnect(void) {
  #ifdef ESP32
    if (!WiFi.disconnect(true, true)){
      return false;
    } else {
      return true;
    }
  #elif ESP8266
    if (!WiFi.disconnect(true)){
      return false;
    } else {
      return true;
    }
  #endif
}

void WiFiQuick::UpdateWakes(void) {
  bool UpdateSafe = false;
  if (rtcValid() == true ) {
    #ifdef ESP32
      WiFiQuick::resetCount = rtc.MEM.rstCount;  // read the previous reset count
    #elif ESP8266
      WiFiQuick::resetCount = nv->rtcMEM.rstCount;
    #endif
    UpdateSafe = true;
    WiFiQuick::useRTC = true;
  }
  WiFiQuick::resetCount++;
  #ifdef ESP32
    rtc.MEM.rstCount = WiFiQuick::resetCount;
  #elif ESP8266
    nv->rtcMEM.rstCount = WiFiQuick::resetCount;
  #endif
  if ( UpdateSafe == true) {
    updateRTCcrc();
  }
}

void WiFiQuick::ResetWakes(void) {
  #ifdef ESP32
    rtc.MEM.rstCount = 0;
  #endif
  #ifdef ESP8266
    nv->rtcMEM.rstCount = 0;
  #endif
  updateRTCcrc();
}

uint32_t WiFiQuick::WakeCount(void) {
    #ifdef ESP32
      return rtc.MEM.rstCount;
    #endif
    #ifdef ESP8266
      return nv->rtcMEM.rstCount;
    #endif
}

uint8_t* WiFiQuick::macAddress(uint8_t* mac) {
     WiFi.macAddress(mac);
     return mac;
 }

 String WiFiQuick::macAddress(void) {
     uint8_t mac[6];
     char macStr[18] = { 0 };
     WiFi.macAddress(mac);

     sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
     return String(macStr);
 }

uint32_t WiFiQuick::wifiMissed(void) {
    #ifdef ESP32
      return rtc.MEM.noWifi;
    #endif
    #ifdef ESP8266
      return nv->rtcMEM.noWifi;
    #endif
}

uint32_t crc32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}
