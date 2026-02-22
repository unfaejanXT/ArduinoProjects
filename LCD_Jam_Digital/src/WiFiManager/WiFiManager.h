/**
 * ============================================================
 *  FILE: src/WiFiManager/WiFiManager.h
 *  Deskripsi: Class untuk mengelola koneksi WiFi pada NodeMCU
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../../config.h"

class WiFiManager {
public:
    /**
     * Constructor
     * @param ssid     : SSID WiFi
     * @param password : Password WiFi
     */
    WiFiManager(const char* ssid, const char* password);

    /**
     * Memulai koneksi WiFi
     * Akan mencoba connect sampai timeout tercapai
     * @return true jika berhasil terhubung, false jika gagal
     */
    bool connect();

    /**
     * Mengecek apakah saat ini terhubung ke WiFi
     * @return true jika terhubung
     */
    bool isConnected() const;

    /**
     * Mencoba reconnect jika koneksi terputus
     * Dipanggil di loop() secara periodik
     * @return true jika berhasil reconnect
     */
    bool ensureConnected();

    /**
     * Memutus koneksi WiFi
     */
    void disconnect();

    /**
     * Mendapatkan IP Address yang diperoleh
     * @return String IP address
     */
    String getIPAddress() const;

    /**
     * Mendapatkan kekuatan sinyal WiFi (RSSI)
     * @return nilai RSSI dalam dBm
     */
    int32_t getRSSI() const;

private:
    const char* _ssid;
    const char* _password;
    unsigned long _timeout;

    void _printStatus(const String& message) const;
};

#endif // WIFI_MANAGER_H
