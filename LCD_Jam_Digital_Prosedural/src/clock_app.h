/**
 * ============================================================
 *  FILE: src/clock_app.h
 *  Deskripsi: Modul prosedural - Orchestrator state machine
 *             Menghubungkan wifi_manager, ntp_manager, lcd_display
 *
 *  PARADIGMA: Prosedural
 *    - Tidak ada class / constructor / metode
 *    - State machine dikelola melalui variabel global file-local
 *    - Seluruh fungsi bersifat bebas (free functions)
 *
 *  Project  : LCD Jam Digital PROSEDURAL - NodeMCU v3
 * ============================================================
 *
 *  State machine aplikasi:
 *
 *    [CONNECTING_WIFI] ──OK──→ [SYNCING_NTP] ──OK──→ [RUNNING]
 *         ↑                                               │
 *         └────────[WIFI_LOST] ←── WiFi putus ───────────┘
 *         └────────[ERROR]     ←── Error kritis
 *
 *  CARA PAKAI:
 *    #include "src/clock_app.h"
 *    appSetup();   // dipanggil dari setup()
 *    appLoop();    // dipanggil dari loop()
 * ============================================================
 */

#ifndef CLOCK_APP_H
#define CLOCK_APP_H

#include <Arduino.h>
#include "../config.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "lcd_display.h"

// ─────────────────────────────────────────────────────────────
//  Enum AppState - State machine aplikasi
// ─────────────────────────────────────────────────────────────
enum AppState {
    APP_CONNECTING_WIFI,   // Sedang menghubungkan ke WiFi
    APP_SYNCING_NTP,       // Sedang sinkronisasi NTP
    APP_RUNNING,           // Jam sedang berjalan normal
    APP_WIFI_LOST,         // Koneksi WiFi terputus
    APP_ERROR              // Terjadi error
};

// ─────────────────────────────────────────────────────────────
//  State internal aplikasi (hanya terlihat di file ini)
// ─────────────────────────────────────────────────────────────
static AppState       _appState           = APP_CONNECTING_WIFI;
static unsigned long  _appLastDisplayUpd  = 0;  // Timestamp terakhir refresh display
static unsigned long  _appLastNTPUpd      = 0;  // Timestamp terakhir update NTP
static unsigned long  _appLastWifiCheck   = 0;  // Timestamp terakhir cek WiFi
static uint8_t        _appConnectingStep  = 0;  // Langkah animasi koneksi
static unsigned long  _appLastAnimUpd     = 0;  // Timestamp animasi

// Interval waktu dalam ms
static const unsigned long APP_DISPLAY_INTERVAL   = 500;    // Refresh display
static const unsigned long APP_WIFI_CHECK_INTERVAL = 30000; // Cek WiFi
static const unsigned long APP_ANIM_INTERVAL       = 400;   // Animasi loading

// ─────────────────────────────────────────────────────────────
//  Deklarasi forward (handler state)
// ─────────────────────────────────────────────────────────────
static void _appHandleConnectingWifi();
static void _appHandleSyncingNTP();
static void _appHandleRunning();
static void _appHandleWifiLost();
static void _appTransitionTo(AppState newState);
static void _appPrintState(AppState state);

// ─────────────────────────────────────────────────────────────
//  appSetup() - Inisialisasi awal, dipanggil dari setup() Arduino
// ─────────────────────────────────────────────────────────────
void appSetup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    Serial.println();
    Serial.println("================================================");
    Serial.println("  LCD Jam Digital - NodeMCU v3");
    Serial.println("  by: Prosedural Single-File-per-Module Project");
    Serial.println("================================================");

    // Inisialisasi OLED SSD1306
    if (!lcdBegin()) {
        Serial.println(F("[APP] FATAL: OLED tidak ditemukan! Program berhenti."));
        while (true) { delay(1000); } // Halt
    }
    Serial.println("[APP] OLED siap.");

    // Tampilkan status awal
    lcdShowStatus("JAM DIGITAL", "Connecting WiFi");
    delay(800);

    // Mulai proses koneksi WiFi
    _appTransitionTo(APP_CONNECTING_WIFI);
}

// ─────────────────────────────────────────────────────────────
//  appLoop() - Loop utama, dipanggil dari loop() Arduino
// ─────────────────────────────────────────────────────────────
void appLoop() {
    switch (_appState) {
        case APP_CONNECTING_WIFI:
            _appHandleConnectingWifi();
            break;

        case APP_SYNCING_NTP:
            _appHandleSyncingNTP();
            break;

        case APP_RUNNING:
            _appHandleRunning();
            break;

        case APP_WIFI_LOST:
            _appHandleWifiLost();
            break;

        case APP_ERROR:
            // Tampilkan error dan reset setelah 5 detik
            delay(5000);
            _appTransitionTo(APP_CONNECTING_WIFI);
            break;
    }
}

// =============================================================
//  HANDLER STATE - FUNGSI PRIVAT INTERNAL
// =============================================================

// ─────────────────────────────────────────────────────────────
//  _appHandleConnectingWifi() - State: Menghubungkan ke WiFi
// ─────────────────────────────────────────────────────────────
static void _appHandleConnectingWifi() {
    // Tampilkan animasi connecting
    unsigned long now = millis();
    if (now - _appLastAnimUpd >= APP_ANIM_INTERVAL) {
        lcdShowConnecting(_appConnectingStep);
        _appConnectingStep++;
        _appLastAnimUpd = now;
    }

    // Coba koneksi WiFi
    if (wifiConnect()) {
        Serial.println("[APP] WiFi terhubung! Lanjut ke sinkronisasi NTP...");
        lcdShowStatus("WiFi Terhubung!", "Sync NTP...");
        delay(800);
        _appTransitionTo(APP_SYNCING_NTP);
    } else {
        // Koneksi gagal, coba lagi setelah delay
        lcdShowError("WiFi Gagal!");
        Serial.println("[APP] WiFi gagal. Coba lagi dalam 5 detik...");
        delay(5000);
        _appConnectingStep = 0; // Reset animasi
    }
}

// ─────────────────────────────────────────────────────────────
//  _appHandleSyncingNTP() - State: Sinkronisasi NTP
// ─────────────────────────────────────────────────────────────
static void _appHandleSyncingNTP() {
    lcdShowStatus("Sync NTP...", NTP_SERVER);
    Serial.println(F("[APP] Memulai sinkronisasi NTP..."));

    ntpBegin();

    // Coba forceSync hingga 3x (blocking, hanya di fase init)
    bool synced = false;
    for (uint8_t attempt = 1; attempt <= 3; attempt++) {
        Serial.print(F("[APP] NTP percobaan ke-"));
        Serial.println(attempt);

        if (ntpForceSync()) {   // forceSync, BUKAN tick/update
            synced = true;
            break;
        }
        delay(2000); // Tunggu 2 detik sebelum retry
    }

    if (synced) {
        _appLastNTPUpd = millis();
        Serial.println("[APP] NTP sinkron! Waktu: " + ntpGetTimeString());
        lcdShowStatus("NTP Berhasil!", ntpGetTimeString());
        delay(1000);
        _appTransitionTo(APP_RUNNING);
    } else {
        Serial.println("[APP] NTP gagal setelah 3 percobaan!");
        lcdShowError("NTP Gagal!");
        delay(3000);
        // WiFi masih konek → coba NTP lagi; jika tidak → reconnect WiFi
        if (wifiIsConnected()) {
            _appTransitionTo(APP_SYNCING_NTP);
        } else {
            _appTransitionTo(APP_CONNECTING_WIFI);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _appHandleRunning() - State: Jam berjalan normal
// ─────────────────────────────────────────────────────────────
static void _appHandleRunning() {
    unsigned long now = millis();

    // ── Refresh tampilan OLED setiap APP_DISPLAY_INTERVAL ───────
    if (now - _appLastDisplayUpd >= APP_DISPLAY_INTERVAL) {
        _appLastDisplayUpd = now;

        // tick() ringan: NTPClient hanya request NTP jika updateInterval
        // sudah habis (default 1 jam). Tidak memblok, tidak spam server.
        ntpTick();

        String timeStr = ntpGetTimeString();
        String dateStr = ntpGetDateString();

        lcdShowClock(timeStr, dateStr);

        // Log ke Serial tiap 10 detik untuk debug
        TimeData td = ntpGetTimeData();
        if (td.second % 10 == 0) {
            Serial.println("[CLOCK] " + timeStr + "  |  " + dateStr);
        }
    }

    // ── Cek koneksi WiFi setiap APP_WIFI_CHECK_INTERVAL ─────────
    if (now - _appLastWifiCheck >= APP_WIFI_CHECK_INTERVAL) {
        _appLastWifiCheck = now;
        if (!wifiIsConnected()) {
            Serial.println("[APP] WiFi terputus! Mencoba reconnect...");
            _appTransitionTo(APP_WIFI_LOST);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _appHandleWifiLost() - State: Koneksi WiFi terputus
// ─────────────────────────────────────────────────────────────
static void _appHandleWifiLost() {
    lcdShowStatus("WiFi Terputus!", "Reconnecting..");
    Serial.println("[APP] Mencoba reconnect WiFi...");

    if (wifiEnsureConnected()) {
        Serial.println("[APP] Reconnect berhasil! Resync NTP...");
        _appTransitionTo(APP_SYNCING_NTP);
    } else {
        Serial.println("[APP] Reconnect gagal. Coba lagi...");
        delay(5000);
    }
}

// ─────────────────────────────────────────────────────────────
//  _appTransitionTo() - Pindah state dan log perubahan
// ─────────────────────────────────────────────────────────────
static void _appTransitionTo(AppState newState) {
    if (_appState != newState) {
        Serial.print("[APP] State: ");
        _appPrintState(_appState);
        Serial.print(" -> ");
        _appPrintState(newState);
        Serial.println();
    }
    _appState = newState;
}

// ─────────────────────────────────────────────────────────────
//  _appPrintState() - Print nama state ke Serial Monitor
// ─────────────────────────────────────────────────────────────
static void _appPrintState(AppState state) {
    switch (state) {
        case APP_CONNECTING_WIFI: Serial.print("CONNECTING_WIFI"); break;
        case APP_SYNCING_NTP:     Serial.print("SYNCING_NTP");     break;
        case APP_RUNNING:         Serial.print("RUNNING");         break;
        case APP_WIFI_LOST:       Serial.print("WIFI_LOST");       break;
        case APP_ERROR:           Serial.print("ERROR");           break;
        default:                  Serial.print("UNKNOWN");         break;
    }
}

#endif // CLOCK_APP_H
