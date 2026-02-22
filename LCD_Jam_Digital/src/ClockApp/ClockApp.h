/**
 * ============================================================
 *  FILE: src/ClockApp/ClockApp.h
 *  Deskripsi: Orchestrator utama aplikasi jam digital
 *             Menghubungkan WiFiManager, NTPManager, LCDDisplay
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#ifndef CLOCK_APP_H
#define CLOCK_APP_H

#include <Arduino.h>
#include "../../config.h"
#include "../WiFiManager/WiFiManager.h"
#include "../NTPManager/NTPManager.h"
#include "../LCDDisplay/LCDDisplay.h"

// State machine untuk aplikasi
enum class AppState {
    CONNECTING_WIFI,   // Sedang menghubungkan ke WiFi
    SYNCING_NTP,       // Sedang sinkronisasi NTP
    RUNNING,           // Jam sedang berjalan normal
    WIFI_LOST,         // Koneksi WiFi terputus
    ERROR              // Terjadi error
};

class ClockApp {
public:
    /**
     * Constructor - inisialisasi semua komponen
     */
    ClockApp();

    /**
     * Setup awal - dipanggil di setup() Arduino
     * Koneksi WiFi, sync NTP, inisialisasi LCD
     */
    void begin();

    /**
     * Loop utama - dipanggil di loop() Arduino
     * Update waktu, refresh tampilan, cek koneksi
     */
    void run();

private:
    WiFiManager _wifi;
    NTPManager  _ntp;
    LCDDisplay  _lcd;
    AppState    _state;

    unsigned long _lastDisplayUpdate;  // Timestamp terakhir refresh display
    unsigned long _lastNTPUpdate;      // Timestamp terakhir update NTP
    unsigned long _lastWifiCheck;      // Timestamp terakhir cek WiFi
    uint8_t       _connectingStep;     // Langkah animasi koneksi
    unsigned long _lastAnimUpdate;     // Timestamp animasi

    // ─── State Handlers ───────────────────────────────────────
    void _handleConnectingWifi();
    void _handleSyncingNTP();
    void _handleRunning();
    void _handleWifiLost();

    // ─── Helper Methods ───────────────────────────────────────
    void _transitionTo(AppState newState);
    void _printStateToSerial(AppState state);

    // Interval waktu dalam ms
    static const unsigned long DISPLAY_INTERVAL = 500;    // Refresh display
    static const unsigned long WIFI_CHECK_INTERVAL = 30000; // Cek WiFi
    static const unsigned long ANIM_INTERVAL = 400;        // Animasi loading
};

#endif // CLOCK_APP_H
