/**
 * ============================================================
 *  FILE: src/ClockApp/ClockApp.cpp
 *  Deskripsi: Implementasi ClockApp - Orchestrator state machine
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#include "ClockApp.h"

// ─────────────────────────────────────────────────────────────
//  Constructor - inisialisasi semua dependency
// ─────────────────────────────────────────────────────────────
ClockApp::ClockApp()
    : _wifi(WIFI_SSID, WIFI_PASSWORD),
      _ntp(NTP_SERVER, NTP_UTC_OFFSET, NTP_UPDATE_INTERVAL),
      _lcd(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, OLED_ADDR, OLED_RESET_PIN),
      _state(AppState::CONNECTING_WIFI),
      _lastDisplayUpdate(0),
      _lastNTPUpdate(0),
      _lastWifiCheck(0),
      _connectingStep(0),
      _lastAnimUpdate(0)
{
}

// ─────────────────────────────────────────────────────────────
//  begin() - Setup awal, dipanggil dari setup() Arduino
// ─────────────────────────────────────────────────────────────
void ClockApp::begin() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    Serial.println();
    Serial.println("================================================");
    Serial.println("  LCD Jam Digital - NodeMCU v3");
    Serial.println("  by: OOP Multi-File Project");
    Serial.println("================================================");

    // Inisialisasi OLED SSD1306
    if (!_lcd.begin()) {
        Serial.println(F("[APP] FATAL: OLED tidak ditemukan! Program berhenti."));
        while (true) { delay(1000); } // Halt
    }
    Serial.println("[APP] OLED siap.");

    // Tampilkan status awal (splash sudah ada di LCDDisplay::begin())
    _lcd.showStatus("JAM DIGITAL", "Connecting WiFi");
    delay(800);

    // Mulai proses koneksi WiFi
    _transitionTo(AppState::CONNECTING_WIFI);
}

// ─────────────────────────────────────────────────────────────
//  run() - Loop utama, dipanggil dari loop() Arduino
// ─────────────────────────────────────────────────────────────
void ClockApp::run() {
    switch (_state) {
        case AppState::CONNECTING_WIFI:
            _handleConnectingWifi();
            break;

        case AppState::SYNCING_NTP:
            _handleSyncingNTP();
            break;

        case AppState::RUNNING:
            _handleRunning();
            break;

        case AppState::WIFI_LOST:
            _handleWifiLost();
            break;

        case AppState::ERROR:
            // Tampilkan error dan reset setelah 5 detik
            delay(5000);
            _transitionTo(AppState::CONNECTING_WIFI);
            break;
    }
}

// ─────────────────────────────────────────────────────────────
//  _handleConnectingWifi() - State: Menghubungkan ke WiFi
// ─────────────────────────────────────────────────────────────
void ClockApp::_handleConnectingWifi() {
    // Tampilkan animasi connecting
    unsigned long now = millis();
    if (now - _lastAnimUpdate >= ANIM_INTERVAL) {
        _lcd.showConnecting(_connectingStep);
        _connectingStep++;
        _lastAnimUpdate = now;
    }

    // Coba koneksi WiFi
    if (_wifi.connect()) {
        Serial.println("[APP] WiFi terhubung! Lanjut ke sinkronisasi NTP...");
        _lcd.showStatus("WiFi Terhubung!", "Sync NTP...");
        delay(800);
        _transitionTo(AppState::SYNCING_NTP);
    } else {
        // Koneksi gagal, coba lagi setelah delay
        _lcd.showError("WiFi Gagal!");
        Serial.println("[APP] WiFi gagal. Coba lagi dalam 5 detik...");
        delay(5000);
        _connectingStep = 0; // Reset animasi
    }
}

// ─────────────────────────────────────────────────────────────
//  _handleSyncingNTP() - State: Sinkronisasi NTP
// ─────────────────────────────────────────────────────────────
void ClockApp::_handleSyncingNTP() {
    _lcd.showStatus("Sync NTP...", NTP_SERVER);
    Serial.println(F("[APP] Memulai sinkronisasi NTP..."));

    _ntp.begin();

    // Coba forceSync hingga 3x (blocking, tapi hanya di fase init)
    bool synced = false;
    for (uint8_t attempt = 1; attempt <= 3; attempt++) {
        Serial.print(F("[APP] NTP percobaan ke-"));
        Serial.println(attempt);

        if (_ntp.forceSync()) {  // <-- forceSync, BUKAN update/tick
            synced = true;
            break;
        }
        delay(2000); // Tunggu 2 detik sebelum retry
    }

    if (synced) {
        _lastNTPUpdate = millis();
        Serial.println("[APP] NTP sinkron! Waktu: " + _ntp.getTimeString());
        _lcd.showStatus("NTP Berhasil!", _ntp.getTimeString());
        delay(1000);
        _transitionTo(AppState::RUNNING);
    } else {
        Serial.println("[APP] NTP gagal setelah 3 percobaan!");
        _lcd.showError("NTP Gagal!");
        delay(3000);
        // Jika WiFi masih konek, coba NTP lagi; jika tidak, reconnect WiFi
        if (_wifi.isConnected()) {
            _transitionTo(AppState::SYNCING_NTP);
        } else {
            _transitionTo(AppState::CONNECTING_WIFI);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _handleRunning() - State: Jam berjalan normal
// ─────────────────────────────────────────────────────────────
void ClockApp::_handleRunning() {
    unsigned long now = millis();

    // ── Refresh tampilan OLED setiap DISPLAY_INTERVAL ───────────
    if (now - _lastDisplayUpdate >= DISPLAY_INTERVAL) {
        _lastDisplayUpdate = now;

        // tick() ringan: NTPClient hanya request NTP jika updateInterval
        // sudah habis (default 1 jam). Tidak memblok, tidak spam server.
        _ntp.tick();

        String timeStr = _ntp.getTimeString();
        String dateStr = _ntp.getDateString();

        _lcd.showClock(timeStr, dateStr);

        // Log ke Serial tiap 10 detik untuk debug
        TimeData td = _ntp.getTimeData();
        if (td.second % 10 == 0) {
            Serial.println("[CLOCK] " + timeStr + "  |  " + dateStr);
        }
    }

    // ── Cek koneksi WiFi setiap WIFI_CHECK_INTERVAL ───────────
    if (now - _lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        _lastWifiCheck = now;
        if (!_wifi.isConnected()) {
            Serial.println("[APP] WiFi terputus! Mencoba reconnect...");
            _transitionTo(AppState::WIFI_LOST);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  _handleWifiLost() - State: Koneksi WiFi terputus
// ─────────────────────────────────────────────────────────────
void ClockApp::_handleWifiLost() {
    _lcd.showStatus("WiFi Terputus!", "Reconnecting..");
    Serial.println("[APP] Mencoba reconnect WiFi...");

    if (_wifi.ensureConnected()) {
        Serial.println("[APP] Reconnect berhasil! Resync NTP...");
        _transitionTo(AppState::SYNCING_NTP);
    } else {
        Serial.println("[APP] Reconnect gagal. Coba lagi...");
        delay(5000);
    }
}

// ─────────────────────────────────────────────────────────────
//  _transitionTo() - Pindah state dan log perubahan
// ─────────────────────────────────────────────────────────────
void ClockApp::_transitionTo(AppState newState) {
    if (_state != newState) {
        Serial.print("[APP] State: ");
        _printStateToSerial(_state);
        Serial.print(" → ");
        _printStateToSerial(newState);
        Serial.println();
    }
    _state = newState;
}

// ─────────────────────────────────────────────────────────────
//  _printStateToSerial() - Print nama state ke Serial Monitor
// ─────────────────────────────────────────────────────────────
void ClockApp::_printStateToSerial(AppState state) {
    switch (state) {
        case AppState::CONNECTING_WIFI: Serial.print("CONNECTING_WIFI"); break;
        case AppState::SYNCING_NTP:     Serial.print("SYNCING_NTP");     break;
        case AppState::RUNNING:         Serial.print("RUNNING");         break;
        case AppState::WIFI_LOST:       Serial.print("WIFI_LOST");       break;
        case AppState::ERROR:           Serial.print("ERROR");           break;
        default:                        Serial.print("UNKNOWN");         break;
    }
}
