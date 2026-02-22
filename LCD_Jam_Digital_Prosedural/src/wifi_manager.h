/**
 * ============================================================
 *  FILE: src/wifi_manager.h
 *  Deskripsi: Modul prosedural untuk mengelola koneksi WiFi
 *             pada NodeMCU v3 (ESP8266).
 *
 *  PARADIGMA: Prosedural
 *    - Tidak ada class / constructor / metode
 *    - State internal disimpan dalam variabel global file-local (static)
 *    - Seluruh fungsi bersifat bebas (free functions)
 *
 *  Project  : LCD Jam Digital PROSEDURAL - NodeMCU v3
 * ============================================================
 *
 *  CARA PAKAI:
 *    #include "src/wifi_manager.h"
 *    wifiConnect();          // koneksi pertama
 *    wifiIsConnected();      // cek status
 *    wifiEnsureConnected();  // reconnect jika terputus
 * ============================================================
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────
//  State internal (hanya terlihat di file ini)
// ─────────────────────────────────────────────────────────────
static const char* _wfSsid     = WIFI_SSID;
static const char* _wfPassword = WIFI_PASSWORD;

// ─────────────────────────────────────────────────────────────
//  wifiConnect() - Memulai koneksi WiFi
//  @return true jika berhasil terhubung, false jika timeout
// ─────────────────────────────────────────────────────────────
bool wifiConnect() {
    Serial.println("[WiFi] Menghubungkan ke: " + String(_wfSsid));

    // Set mode WiFi sebagai Station (client)
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wfSsid, _wfPassword);

    unsigned long startTime = millis();

    // Tunggu koneksi atau timeout
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= WIFI_TIMEOUT_MS) {
            Serial.println();
            Serial.println("[WiFi] GAGAL: Timeout setelah " +
                           String(WIFI_TIMEOUT_MS / 1000) + " detik");
            return false;
        }
        delay(250);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("[WiFi] Terhubung!");
    Serial.println("[WiFi] IP Address : " + WiFi.localIP().toString());
    Serial.println("[WiFi] RSSI       : " + String(WiFi.RSSI()) + " dBm");

    return true;
}

// ─────────────────────────────────────────────────────────────
//  wifiIsConnected() - Cek apakah sedang terhubung
//  @return true jika status WL_CONNECTED
// ─────────────────────────────────────────────────────────────
bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// ─────────────────────────────────────────────────────────────
//  wifiEnsureConnected() - Reconnect jika koneksi terputus
//  @return true jika terhubung (sudah atau baru reconnect)
// ─────────────────────────────────────────────────────────────
bool wifiEnsureConnected() {
    if (wifiIsConnected()) {
        return true;
    }
    Serial.println("[WiFi] Koneksi terputus! Mencoba reconnect...");
    return wifiConnect();
}

// ─────────────────────────────────────────────────────────────
//  wifiDisconnect() - Putus koneksi WiFi
// ─────────────────────────────────────────────────────────────
void wifiDisconnect() {
    WiFi.disconnect();
    Serial.println("[WiFi] Disconnected.");
}

// ─────────────────────────────────────────────────────────────
//  wifiGetIP() - Dapatkan IP yang diperoleh dari DHCP
//  @return String IP address
// ─────────────────────────────────────────────────────────────
String wifiGetIP() {
    return WiFi.localIP().toString();
}

// ─────────────────────────────────────────────────────────────
//  wifiGetRSSI() - Dapatkan kekuatan sinyal WiFi
//  @return nilai RSSI dalam dBm
// ─────────────────────────────────────────────────────────────
int32_t wifiGetRSSI() {
    return WiFi.RSSI();
}

#endif // WIFI_MANAGER_H
