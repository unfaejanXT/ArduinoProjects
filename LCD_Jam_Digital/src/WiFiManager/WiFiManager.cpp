/**
 * ============================================================
 *  FILE: src/WiFiManager/WiFiManager.cpp
 *  Deskripsi: Implementasi class WiFiManager
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#include "WiFiManager.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid),
      _password(password),
      _timeout(WIFI_TIMEOUT_MS)
{
}

// ─────────────────────────────────────────────────────────────
//  connect() - Memulai koneksi WiFi
// ─────────────────────────────────────────────────────────────
bool WiFiManager::connect() {
    _printStatus("Menghubungkan ke WiFi: " + String(_ssid));

    // Set mode WiFi sebagai Station (client)
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    unsigned long startTime = millis();

    // Tunggu koneksi atau timeout
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= _timeout) {
            _printStatus("[WiFi] GAGAL: Timeout setelah " + String(_timeout / 1000) + " detik");
            return false;
        }
        delay(250);
        Serial.print(".");
    }

    Serial.println();
    _printStatus("[WiFi] Terhubung!");
    _printStatus("[WiFi] IP Address : " + getIPAddress());
    _printStatus("[WiFi] RSSI       : " + String(getRSSI()) + " dBm");

    return true;
}

// ─────────────────────────────────────────────────────────────
//  isConnected() - Cek status koneksi
// ─────────────────────────────────────────────────────────────
bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

// ─────────────────────────────────────────────────────────────
//  ensureConnected() - Reconnect jika terputus
// ─────────────────────────────────────────────────────────────
bool WiFiManager::ensureConnected() {
    if (isConnected()) {
        return true;
    }

    _printStatus("[WiFi] Koneksi terputus! Mencoba reconnect...");
    return connect();
}

// ─────────────────────────────────────────────────────────────
//  disconnect() - Putus koneksi WiFi
// ─────────────────────────────────────────────────────────────
void WiFiManager::disconnect() {
    WiFi.disconnect();
    _printStatus("[WiFi] Disconnected.");
}

// ─────────────────────────────────────────────────────────────
//  getIPAddress() - Dapatkan IP yang didapat dari DHCP
// ─────────────────────────────────────────────────────────────
String WiFiManager::getIPAddress() const {
    return WiFi.localIP().toString();
}

// ─────────────────────────────────────────────────────────────
//  getRSSI() - Dapatkan kekuatan sinyal
// ─────────────────────────────────────────────────────────────
int32_t WiFiManager::getRSSI() const {
    return WiFi.RSSI();
}

// ─────────────────────────────────────────────────────────────
//  _printStatus() - Helper print ke Serial Monitor
// ─────────────────────────────────────────────────────────────
void WiFiManager::_printStatus(const String& message) const {
    Serial.println(message);
}
