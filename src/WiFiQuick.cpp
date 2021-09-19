/*
    WiFiQuick   (c) 2021 Winford (Uncle Grumpy)

    Library to facilitate fast reconnection to wifi
    after deep sleep. Uses RTC user memory to store
    the necessary information to allow connection to
    the previous wifi network without the expense of
    doing a network scan.

*/
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <coredecls.h>         // crc32()
#include "WiFiQuick.h"

#ifdef WQ_DEBUG
#define WQ_SERIAL
#endif
#ifndef WQ_SERIAL_DISABLE
#define WQ_SERIAL
#endif

#define WQ_SERIAL_DISABLE

struct nv_s {
  uint8_t OTAbootloaderCMD [128];  // Leave this space free! it is used by ota to install the new bootloader command. 
  uint32_t crc;  // =) Stored outside of the rtcMEM struct so we don't have to wory about offset when we calculate crc32 of the data.
  struct {
    // Add anything here that you want to save in RTC_USER_MEM. MUST be 4-byte aligned for crc to work!
    uint32_t rstCount;  // stores the Deep Sleep reset count
    uint32_t noWifi;     // stores the number of consecutive missed connections
    uint32_t channel;    // stores the wifi channel for faster no-scan connetion
    uint32_t bssid[6];   // stores mac address of AP for no-san connection
    uint32_t myIP[4];       // use last IP for faster static (no dhcp delay) connection
    uint32_t wlGateway[4];  // store gateway for static connection
    uint32_t wlSubNet[4];  // subnet for static connection
    uint32_t wlDNS[4];      // DNS for static connection
    #ifdef ESP32
    uint32_t userData[454];
    #elif defined(ESP8266)
    uint32_t userData[70];
    #endif
  } rtcMEM;
};

static nv_s* nv = (nv_s*)RTC_USER_MEM; // user RTC RAM area

uint32_t WiFiQuick::resetCount = 0;
uint32_t WiFiQuick::_MissedWiFi = 0;
IPAddress WiFiQuick::_noIP = IPAddress(0,0,0,0);
uint32_t WiFiQuick::_wlStart = 0;
uint32_t WiFiQuick::_ConTime = 0;

bool WiFiQuick::updateRTCcrc() {  // updates the reset count CRC
  nv->crc = crc32((uint8_t*)&nv->rtcMEM, sizeof(nv->rtcMEM));
  return rtcValid();
}

bool WiFiQuick::rtcValid() {
  bool valid;
  // Calculate the CRC of what we just read from RTC memory
  uint32_t crc = crc32((uint8_t*)&nv->rtcMEM, sizeof(nv->rtcMEM));
  if( crc != nv->crc ) {
    valid = false;
  } else {
    valid = true;
  }
  return valid;
}

uint32_t WiFiQuick::init(const char* ssid, const char* password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  _wlStart = millis();
  uint8_t wifiID[6];

  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(10);
  WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
  if ((rtcValid()) && (nv->rtcMEM.noWifi == 0)) {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    #endif
    for (unsigned int mem = 0; mem < 4; mem++) {
      staticIP[mem] = nv->rtcMEM.myIP[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      gateway[mem] = nv->rtcMEM.wlGateway[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      subnet[mem] = nv->rtcMEM.wlSubNet[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      dns[mem] = nv->rtcMEM.wlDNS[mem];
    }
    for (unsigned int mem = 0; mem < 6; mem++) {
      wifiID[mem] = nv->rtcMEM.bssid[mem];
    }
    if (staticIP != _noIP) {
      WiFi.config(staticIP, gateway, subnet, dns);
    }
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Reconnecting to previous network");
    #endif
    #ifdef WQ_DEBUG
    Serial.print(" using channel ");
    Serial.print(nv->rtcMEM.channel);
    Serial.print(" and bssid  ");
    for (int mem = 0; mem < 6; mem++ ) {
      Serial.print(wifiID[mem], HEX);
      delay(1);
    }
    delay(1);
    #endif
    WiFi.begin(ssid, password, nv->rtcMEM.channel, wifiID, true);
  } else {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    Serial.print("wifiMissed = ");
    Serial.println(nv->rtcMEM.noWifi);
    delay(1);
    #endif
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Connecting to network");
    delay(1);
    #endif
    if (staticIP != _noIP) {
      WiFi.config(staticIP, gateway, subnet, dns);
    }
    WiFi.begin(ssid, password);
  }
  return _wlStart;
}

// bool WiFiQuick::begin(const char* ssid, const char* password, uint MaxSecs) {
//   bool result;
//   const char* _net = ssid;
//   const char* _pass = password;
//   WiFiQuick::init(_net, _pass);
//   if (WiFiQuick::begin(MaxSecs)) {
//      result = true;
//   } else {
//     result = false;
//   }
//   return result;
// }

bool WiFiQuick::begin(const char* ssid, const char* password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress dns, uint MaxSecs) {
  bool result;
  WiFiQuick::init(ssid, password, staticIP, gateway, subnet, dns);
  if (WiFiQuick::begin(MaxSecs)) {
     result = true;
  } else {
    result = false;
  }
  return result;
}

bool WiFiQuick::begin(uint MaxSecs) {
  uint32_t wifiMissed;
  uint32_t MaxTimeout = MaxSecs * 1000;
  uint32_t GiveUp = _wlStart + MaxTimeout;   // 10 seconds max before giving up.
  while ( (WiFi.status() != WL_CONNECTED) && (millis() < GiveUp) ) {
    delay(50);
    #ifdef WQ_SERIAL
    Serial.print(".");
    #endif
  }
  if (WiFi.status() != WL_CONNECTED) {
    // char err[] = "WiFi connect failed. Retry in 60 Seconds.";
    delay(1);
    wifiMissed = nv->rtcMEM.noWifi;  // read the previous wifi fail count
    delay(1);
    wifiMissed++;
    #ifdef WQ_DEBUG
    Serial.print("SAVING WIFI MISSED #");
    Serial.println(wifiMissed);
    delay(1);
    #endif
    nv->rtcMEM.noWifi = wifiMissed; // update the missed connection count and save to rtc
    delay(1);
    digitalWrite(4, LOW);   // Power off peripherals.
    #ifdef WQ_SERIAL
    uint32_t reTrySec = 60 * wifiMissed;
    Serial.println();
    Serial.print("WiFi connect failed. Retry in ");
    Serial.print(reTrySec);
    Serial.println(" seconds.");
    Serial.println();
    delay(1);
    #endif
    WiFiQuick::updateRTCcrc();
    delay(1);
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    return false;
  } else {
    nv->rtcMEM.channel = WiFi.channel();
    #ifdef WQ_SERIAL
    _ConTime = millis() - _wlStart;
    Serial.println();
    Serial.print("Connected in ");
    Serial.print(_ConTime);
    Serial.println("ms.");
    delay(1);
    #endif
    #ifdef WQ_DEBUG
    Serial.print("Wrote channel #");
    Serial.println(nv->rtcMEM.channel);
    delay(1);
    #endif
    nv->rtcMEM.noWifi = 0;   // reset missed connection counter.
    delay(1);
    uint8_t* bss_id = WiFi.BSSID();
    #ifdef WQ_DEBUG
    Serial.print("Wrote network bssid > ");
    #endif
    for (unsigned int len = 0; len < WL_MAC_ADDR_LENGTH; len++ ) {
      nv->rtcMEM.bssid[len] = bss_id[len];
      #ifdef WQ_DEBUG
      Serial.print(nv->rtcMEM.bssid[len], HEX);
      #endif
    }
    IPAddress staticIP = WiFi.localIP();
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress dns = WiFi.dnsIP();
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcMEM.myIP[mem] = staticIP[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcMEM.wlGateway[mem] = gateway[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcMEM.wlSubNet[mem] = subnet[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcMEM.wlDNS[mem] = dns[mem];
    }
    WiFiQuick::updateRTCcrc();
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

void WiFiQuick::UpdateWakes() {
    if (WiFiQuick::rtcValid()) {
      WiFiQuick::resetCount = nv->rtcMEM.rstCount;  // read the previous reset count
    }
    WiFiQuick::resetCount++;
    nv->rtcMEM.rstCount = WiFiQuick::resetCount;
    WiFiQuick::updateRTCcrc();
}

void WiFiQuick::ResetWakes() {
    if (WiFiQuick::rtcValid()) {
      WiFiQuick::resetCount = 0;  // read the previous reset count
      nv->rtcMEM.rstCount = WiFiQuick::resetCount;
    }
    WiFiQuick::updateRTCcrc();
}

uint32_t WiFiQuick::WakeCount() {
    return nv->rtcMEM.rstCount;
}

uint8_t* WiFiQuick::macAddress(uint8_t* mac) {
     wifi_get_macaddr(STATION_IF, mac);
     return mac;
 }

 String WiFiQuick::macAddress(void) {
     uint8_t mac[6];
     char macStr[18] = { 0 };
     wifi_get_macaddr(STATION_IF, mac);

     sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
     return String(macStr);
 }

uint32_t WiFiQuick::wifiMissed() {
    return nv->rtcMEM.noWifi;
}