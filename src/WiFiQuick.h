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
    #ifndef WL_MAC_ADDR_LENGTH
        #define WL_MAC_ADDR_LENGTH 6
    #endif
    #define WIFI_NVS_ENABLED 0
#elif ESP8266
    #include <ESP8266WiFi.h>
    //#include <coredecls.h>
#endif


class WiFiQuick {
    public:
        WiFiQuick();
        ~WiFiQuick();
        static uint32_t MissedWiFi;
        static uint32_t authTimer;
        uint32_t init(const char* ssid, const char* password, IPAddress staticIP=useDHCP, IPAddress gateway=useDHCP, IPAddress subnet=useDHCP, IPAddress dns=useDHCP);
        bool begin(uint MaxSecs=10);
        bool begin(const char* ssid, const char* password, uint MaxSecs=10);
        bool begin(const char* ssid, const char* password, IPAddress staticIP=useDHCP, IPAddress gateway=useDHCP, IPAddress subnet=useDHCP, IPAddress dns=useDHCP, uint MaxSecs=10);
        bool disconnect(void);
        void UpdateWakes(void);
        uint32_t WakeCount(void);
        void ResetWakes(void);
        uint32_t wifiMissed(void);
        uint8_t* macAddress(uint8_t* mac);
        String macAddress(void);
    private:
        static IPAddress useDHCP;
        static uint32_t _wlStart;
        static uint32_t resetCount;
        bool useRTC;
        //bool rtcValid(void);
        //bool updateRTCcrc(void);
        //uint32_t crc32(const uint8_t *data, size_t length);      
};

#endif