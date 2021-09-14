/*
    WiFiQuick   (c) 2021 Winford (Uncle Grumpy)

    Library to facilitate fast reconnection to wifi
    after deep sleep. Uses RTC user memory to store
    the necessary information to allow connection to
    the previous wifi network without the expense of
    doing a network scan.

*/
#ifndef WiFiQuick_h
#define WiFiQuick_h

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP8266WiFi.h>
#include <coredecls.h>         // crc32()
//#include "credentials.h"

//void WiFiTimeout(uint32_t wifiTime);
void WiFiTimeout(uint32_t wifiTime, int MaxSecs=10);
uint32_t WiFiInit(const char* ssid, const char* password);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
bool rtcValid();
bool updateRTCcrc();
void UpdateResetCount();
uint32_t GetResetCount();
byte GetMyMAC();

#endif