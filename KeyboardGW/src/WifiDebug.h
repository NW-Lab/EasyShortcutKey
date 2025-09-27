#ifndef WIFI_DEBUG_H
#define WIFI_DEBUG_H

#include <WiFi.h>
#include <WiFiClient.h>

class WifiDebug {
private:
    WiFiServer server;
    WiFiClient client;
    bool connected = false;
    
public:
    WifiDebug(int port = 23) : server(port) {}
    
    void begin(const char* ssid, const char* password) {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
        }
        server.begin();
    }
    
    void println(String message) {
        // Serial出力も併用
        Serial.println(message);
        
        // WiFi経由でも送信
        if (!connected) {
            client = server.available();
            if (client) {
                connected = true;
            }
        }
        
        if (connected && client.connected()) {
            client.println(message);
        }
    }
};

#endif