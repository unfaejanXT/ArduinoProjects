/**
 * ============================================================
 *  FILE: src/NTPManager/NTPManager.cpp
 *  Deskripsi: Implementasi NTPManager
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#include "NTPManager.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
NTPManager::NTPManager(const char* server, long utcOffset, unsigned long updateInterval)
    : _client(_udp, server, utcOffset, updateInterval),
      _synced(false)
{
}

// ─────────────────────────────────────────────────────────────
//  begin() - Inisialisasi UDP & NTPClient
// ─────────────────────────────────────────────────────────────
void NTPManager::begin() {
    _client.begin();
    Serial.println(F("[NTP] Client diinisialisasi."));
    Serial.print(F("[NTP] Server : "));
    Serial.println(NTP_SERVER);
    Serial.print(F("[NTP] Offset : UTC+"));
    Serial.println(NTP_UTC_OFFSET / 3600);
}

// ─────────────────────────────────────────────────────────────
//  forceSync() - Paksa sinkronisasi NTP (blocking)
//
//  Mengirim UDP request dan MENUNGGU reply dari server.
//  Hanya panggil saat startup atau setelah reconnect WiFi.
//  JANGAN panggil di dalam loop() setiap 500ms!
// ─────────────────────────────────────────────────────────────
bool NTPManager::forceSync() {
    Serial.println(F("[NTP] Memulai force sync..."));

    bool ok = _client.forceUpdate();

    if (ok) {
        _synced = true;
        Serial.print(F("[NTP] Sync OK - Waktu  : "));
        Serial.println(getTimeString());
        Serial.print(F("[NTP] Sync OK - Tanggal: "));
        Serial.println(getDateString());
        Serial.print(F("[NTP] Epoch time       : "));
        Serial.println(_client.getEpochTime());
    } else {
        Serial.println(F("[NTP] forceSync GAGAL - server tidak merespons."));
    }

    return ok;
}

// ─────────────────────────────────────────────────────────────
//  tick() - Update ringan periodik (non-blocking)
//
//  NTPClient secara internal mengirim request HANYA jika
//  updateInterval sudah habis. Aman dipanggil setiap loop().
// ─────────────────────────────────────────────────────────────
void NTPManager::tick() {
    _client.update(); // Hanya update jika waktunya (tidak memblok)
}

// ─────────────────────────────────────────────────────────────
//  getTimeData() - Struct waktu dari NTPClient
// ─────────────────────────────────────────────────────────────
TimeData NTPManager::getTimeData() {
    TimeData td;

    // Ambil jam/menit/detik langsung dari NTPClient
    // (sudah termasuk UTC offset yang diset di constructor)
    td.hour      = (uint8_t)_client.getHours();
    td.minute    = (uint8_t)_client.getMinutes();
    td.second    = (uint8_t)_client.getSeconds();
    td.dayOfWeek = (uint8_t)_client.getDay();

    // Konversi epoch → tanggal menggunakan algoritma sederhana
    // getEpochTime() sudah include UTC offset → hasilnya tanggal lokal
    _epochToDate(_client.getEpochTime(), td.day, td.month, td.year);

    return td;
}

// ─────────────────────────────────────────────────────────────
//  getTimeString() - "HH:MM:SS"
// ─────────────────────────────────────────────────────────────
String NTPManager::getTimeString() {
    return _pad2((uint8_t)_client.getHours())   + ":" +
           _pad2((uint8_t)_client.getMinutes()) + ":" +
           _pad2((uint8_t)_client.getSeconds());
}

// ─────────────────────────────────────────────────────────────
//  getDateString() - "DD-MM-YYYY"
// ─────────────────────────────────────────────────────────────
String NTPManager::getDateString() {
    uint8_t  d, m;
    uint16_t y;
    _epochToDate(_client.getEpochTime(), d, m, y);
    return _pad2(d) + "-" + _pad2(m) + "-" + String(y);
}

// ─────────────────────────────────────────────────────────────
//  getDayName() - Nama hari dalam Bahasa Indonesia
// ─────────────────────────────────────────────────────────────
String NTPManager::getDayName() {
    static const char* HARI[] = {
        "Minggu","Senin","Selasa","Rabu","Kamis","Jumat","Sabtu"
    };
    return String(HARI[_client.getDay() % 7]);
}

// ─────────────────────────────────────────────────────────────
//  isSynced()
// ─────────────────────────────────────────────────────────────
bool NTPManager::isSynced() const {
    return _synced;
}

// ─────────────────────────────────────────────────────────────
//  getEpochTime()
// ─────────────────────────────────────────────────────────────
unsigned long NTPManager::getEpochTime() {
    return _client.getEpochTime();
}

// ─────────────────────────────────────────────────────────────
//  _epochToDate() - Konversi epoch ke tanggal (algoritma loop)
//
//  Menerima epoch lokal (UTC + offset) → hasilnya tanggal lokal.
//
//  Algoritma loop sederhana:
//  1. Hitung berapa hari sejak 1970-01-01
//  2. Kurangi per-tahun (sambil cek kabisat)
//  3. Kurangi per-bulan
//  Tidak ada overflow, tidak ada bug Julian Day.
// ─────────────────────────────────────────────────────────────
void NTPManager::_epochToDate(unsigned long epochLocal,
                               uint8_t& day, uint8_t& month, uint16_t& year)
{
    // Jumlah hari sejak Unix epoch (1 Januari 1970)
    unsigned long dayCount = epochLocal / 86400UL;

    // ── Hitung tahun ──────────────────────────────────────────
    uint16_t y = 1970;
    while (true) {
        uint16_t diy = _isLeapYear(y) ? 366 : 365;
        if (dayCount < diy) break;
        dayCount -= diy;
        y++;
    }
    year = y;

    // ── Hitung bulan ──────────────────────────────────────────
    static const uint8_t DAYS_IN_MONTH[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    uint8_t m = 0;
    for (m = 0; m < 12; m++) {
        uint8_t dim = DAYS_IN_MONTH[m];
        if (m == 1 && _isLeapYear(y)) dim = 29; // Februari kabisat
        if (dayCount < dim) break;
        dayCount -= dim;
    }
    month = m + 1;          // 1-indexed
    day   = (uint8_t)dayCount + 1; // 1-indexed
}

// ─────────────────────────────────────────────────────────────
//  _isLeapYear() - Cek tahun kabisat Gregorian
// ─────────────────────────────────────────────────────────────
bool NTPManager::_isLeapYear(uint16_t y) const {
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

// ─────────────────────────────────────────────────────────────
//  _pad2() - Format angka ke 2 digit dengan leading zero
// ─────────────────────────────────────────────────────────────
String NTPManager::_pad2(uint8_t val) const {
    return (val < 10) ? ("0" + String(val)) : String(val);
}
