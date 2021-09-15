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
//#include <WiFiClient.h>
#include <coredecls.h>         // crc32()
#include "WiFiQuick.h"

#ifdef WQ_DEBUG
#define WQ_SERIAL
#endif
#ifndef WQ_SERIAL_DISABLE
#define WQ_SERIAL
#endif

//#define WQ_DEBUG

struct nv_s {
  uint8_t OTAbootloaderCMD [128];  // Leave this space free! it is used by ota to install the new bootloader command. 
  uint32_t crc;  // =) Stored outside of the rtcData struct so we don't have to wory about offset when we calculate crc32 of the data.
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
    uint32_t userData[70];
  } rtcData;
};

static nv_s* nv = (nv_s*)RTC_USER_MEM; // user RTC RAM area
uint32_t resetCount = 0;

bool updateRTCcrc() {  // updates the reset count CRC
  nv->crc = crc32((uint8_t*)&nv->rtcData, sizeof(nv->rtcData));
  if (!rtcValid()){
    return false;
  }
  return true;
}

bool rtcValid() {
  bool valid;
  // Calculate the CRC of what we just read from RTC memory
  uint32_t crc = crc32((uint8_t*)&nv->rtcData, sizeof(nv->rtcData));
  if( crc != nv->crc ) {
    valid = false;
  } else {
    valid = true;
  }
  return valid;
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
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

uint32_t WiFiInit(const char* ssid, const char* password) {
  uint32_t startWifi = millis();
  IPAddress staticIP;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;
  uint8_t wifiID[6];

  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(10);
  WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
  if ((rtcValid()) && (nv->rtcData.noWifi == 0)) {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    #endif
    for (unsigned int mem = 0; mem < 4; mem++) {
      staticIP[mem] = nv->rtcData.myIP[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      gateway[mem] = nv->rtcData.wlGateway[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      subnet[mem] = nv->rtcData.wlSubNet[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      dns[mem] = nv->rtcData.wlDNS[mem];
    }
    for (unsigned int mem = 0; mem < 6; mem++) {
      wifiID[mem] = nv->rtcData.bssid[mem];
    }
    WiFi.config(staticIP, gateway, subnet, dns);
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Reconnecting to previous network");
    #endif
    #ifdef WQ_DEBUG
    Serial.print(" using channel ");
    Serial.print(nv->rtcData.channel);
    Serial.print(" and bssid  ");
    for (int mem = 0; mem < 6; mem++ ) {
      Serial.print(wifiID[mem], HEX);
      delay(1);
    }
    delay(1);
    #endif
    WiFi.begin(ssid, password, nv->rtcData.channel, wifiID, true);
  } else {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    Serial.print("wifiMissed = ");
    Serial.println(nv->rtcData.noWifi);
    delay(1);
    #endif
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Connecting to network");
    delay(1);
    #endif
    WiFi.begin(ssid, password);
  }
  return startWifi;
}

uint32_t WiFiInit(const char* ssid, const char* password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  uint32_t startWifi = millis();
  uint8_t wifiID[6];

  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(10);
  WiFi.persistent(false);   // Dont's save WiFiState to flash we will store it in RTC RAM later.
  if ((rtcValid()) && (nv->rtcData.noWifi == 0)) {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    #endif
    for (unsigned int mem = 0; mem < 4; mem++) {
      staticIP[mem] = nv->rtcData.myIP[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      gateway[mem] = nv->rtcData.wlGateway[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      subnet[mem] = nv->rtcData.wlSubNet[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      dns[mem] = nv->rtcData.wlDNS[mem];
    }
    for (unsigned int mem = 0; mem < 6; mem++) {
      wifiID[mem] = nv->rtcData.bssid[mem];
    }
    WiFi.config(staticIP, gateway, subnet, dns);
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Reconnecting to previous network");
    #endif
    #ifdef WQ_DEBUG
    Serial.print(" using channel ");
    Serial.print(nv->rtcData.channel);
    Serial.print(" and bssid  ");
    for (int mem = 0; mem < 6; mem++ ) {
      Serial.print(wifiID[mem], HEX);
      delay(1);
    }
    delay(1);
    #endif
    WiFi.begin(ssid, password, nv->rtcData.channel, wifiID, true);
  } else {
    #ifdef WQ_DEBUG
    Serial.print("rtcOK = ");
    Serial.println(rtcOK);
    Serial.print("wifiMissed = ");
    Serial.println(nv->rtcData.noWifi);
    delay(1);
    #endif
    #ifdef WQ_SERIAL
    Serial.println();
    Serial.print("Connecting to network");
    delay(1);
    #endif
    WiFi.config(staticIP, gateway, subnet, dns);
    WiFi.begin(ssid, password);
  }
  return startWifi;
}

void WiFiTimeout(uint32_t wifiTime, int MaxSecs) {
  uint32_t wifiMissed;
  uint32_t MaxTimeout = MaxSecs * 1000;
  uint32_t GiveUp = millis() + MaxTimeout;   // 10 seconds max before giving up.
  while ( (WiFi.status() != WL_CONNECTED) && (millis() < GiveUp) ) {
    delay(50);
    #ifdef WQ_SERIAL
    Serial.print(".");
    #endif
  }
  if (WiFi.status() != WL_CONNECTED) {
    // char err[] = "WiFi connect failed. Retry in 60 Seconds.";
    delay(1);
    wifiMissed = nv->rtcData.noWifi;  // read the previous wifi fail count
    delay(1);
    wifiMissed++;
    #ifdef WQ_DEBUG
    Serial.print("SAVING WIFI MISSED #");
    Serial.println(wifiMissed);
    delay(1);
    #endif
    nv->rtcData.noWifi = wifiMissed; // update the missed connection count and save to rtc
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
    updateRTCcrc();
    delay(1);
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    uint32_t NapTime = wifiMissed * 60e6;
    ESP.deepSleep(NapTime, RF_DISABLED);    // Try again in 60 seconds multiplied by missed connections.
  } else {
    nv->rtcData.channel = WiFi.channel();
    #ifdef WQ_SERIAL
    uint32_t ConTime = millis() - wifiTime;
    Serial.println();
    Serial.print("Connected in ");
    Serial.print(ConTime);
    Serial.println("ms.");
    delay(1);
    #endif
    #ifdef WQ_DEBUG
    Serial.print("Wrote channel #");
    Serial.println(nv->rtcData.channel);
    delay(1);
    #endif
    nv->rtcData.noWifi = 0;   // reset missed connection counter.
    delay(1);
    uint8_t* bss_id = WiFi.BSSID();
    #ifdef WQ_DEBUG
    Serial.print("Wrote network bssid > ");
    #endif
    for (unsigned int len = 0; len < WL_MAC_ADDR_LENGTH; len++ ) {
      nv->rtcData.bssid[len] = bss_id[len];
      #ifdef WQ_DEBUG
      Serial.print(nv->rtcData.bssid[len], HEX);
      #endif
    }
    IPAddress staticIP = WiFi.localIP();
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress dns = WiFi.dnsIP();
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcData.myIP[mem] = staticIP[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcData.wlGateway[mem] = gateway[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcData.wlSubNet[mem] = subnet[mem];
    }
    for (unsigned int mem = 0; mem < 4; mem++) {
      nv->rtcData.wlDNS[mem] = dns[mem];
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
}

void UpdateResetCount() {
    if (rtcValid()) {
      resetCount = nv->rtcData.rstCount;  // read the previous reset count
    }
    resetCount++;
    nv->rtcData.rstCount = resetCount;
    updateRTCcrc();
}

uint32_t GetResetCount() {
    return nv->rtcData.rstCount;
}
