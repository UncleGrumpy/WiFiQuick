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


class WiFiQuick {

    static uint32_t _resetCount;
    static uint32_t _MissedWiFi;
    static IPAddress _noIP;
    static uint32_t _wlStart;
    static uint32_t resetCount;

    bool rtcValid(void);
    bool updateRTCcrc(void);

    public:

        static uint32_t MissedWiFi;
        static uint32_t authTimer;

        uint32_t init(const char* ssid, const char* password, IPAddress staticIP=_noIP, IPAddress gateway=_noIP, IPAddress subnet=_noIP, IPAddress dns=_noIP);
        bool begin(uint MaxSecs=10);
        bool begin(const char* ssid, const char* password, IPAddress staticIP=_noIP, IPAddress gateway=_noIP, IPAddress subnet=_noIP, IPAddress dns=_noIP, uint MaxSecs=10);
        void UpdateWakes(void);
        uint32_t WakeCount(void);
        void ResetWakes(void);
        uint32_t wifiMissed(void);
        uint8_t* macAddress(uint8_t* mac);
        String macAddress(void);

};

#endif