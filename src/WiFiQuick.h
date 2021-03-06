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
#elif ESP8266
    #include <ESP8266WiFi.h>
#endif


class WiFiQuick {
    public:
        WiFiQuick();
        static uint32_t MissedWiFi;
        static uint32_t authTimer;
        uint32_t init(const char* ssid, const char* password, IPAddress staticIP=useDHCP, IPAddress gateway=useDHCP, IPAddress subnet=useDHCP, IPAddress dns=useDHCP);
        bool begin(uint8_t MaxSecs=10);
        bool begin(const char* ssid, const char* password, IPAddress staticIP=useDHCP, IPAddress gateway=useDHCP, IPAddress subnet=useDHCP, IPAddress dns=useDHCP, uint8_t MaxSecs=10);
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
        uint8_t _MaxSecs;     
};

#endif