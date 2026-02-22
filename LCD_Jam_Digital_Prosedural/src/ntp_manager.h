/**
 * ============================================================
 *  FILE: src/ntp_manager.h
 *  Deskripsi: Modul prosedural untuk sinkronisasi waktu NTP
 *
 *  PARADIGMA: Prosedural
 *    - Tidak ada class / constructor / metode
 *    - NTPClient & WiFiUDP disimpan sebagai variabel global file-local
 *    - Seluruh fungsi bersifat bebas (free functions)
 *
 *  Project  : LCD Jam Digital PROSEDURAL - NodeMCU v3
 * ============================================================
 *
 *  CARA PAKAI:
 *    #include "src/ntp_manager.h"
 *    ntpBegin();        // dipanggil setelah WiFi terhubung
 *    ntpForceSync();    // sinkronisasi paksa (blocking)
 *    ntpTick();         // update ringan di loop()
 *    ntpGetTimeString() // "HH:MM:SS"
 *    ntpGetDateString() // "DD-MM-YYYY"
 * ============================================================
 */

#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────
//  Struct TimeData - menyimpan data waktu yang sudah diparsing
// ─────────────────────────────────────────────────────────────
struct TimeData {
    uint8_t  hour;      // Jam     : 0-23
    uint8_t  minute;    // Menit   : 0-59
    uint8_t  second;    // Detik   : 0-59
    uint8_t  day;       // Tanggal : 1-31
    uint8_t  month;     // Bulan   : 1-12
    uint16_t year;      // Tahun   : e.g. 2026
    uint8_t  dayOfWeek; // Hari    : 0=Minggu, 1=Senin, ..., 6=Sabtu
};

// ─────────────────────────────────────────────────────────────
//  State internal (hanya terlihat di file ini)
// ─────────────────────────────────────────────────────────────
static WiFiUDP   _ntpUdp;
static NTPClient _ntpClient(_ntpUdp, NTP_SERVER, NTP_UTC_OFFSET, NTP_UPDATE_INTERVAL);
static bool      _ntpSynced = false;

// ─────────────────────────────────────────────────────────────
//  Deklarasi forward - fungsi publik yang dipakai sebelum didefinisi
// ─────────────────────────────────────────────────────────────
String ntpGetTimeString();
String ntpGetDateString();

// ─────────────────────────────────────────────────────────────
//  Deklarasi forward - fungsi privat internal
// ─────────────────────────────────────────────────────────────
static bool      _ntpIsLeapYear(uint16_t y);
static void      _ntpEpochToDate(unsigned long epochLocal,
                                  uint8_t& day, uint8_t& month, uint16_t& year);
static String    _ntpPad2(uint8_t val);

// ─────────────────────────────────────────────────────────────
//  ntpBegin() - Inisialisasi UDP & NTPClient
//  Dipanggil SETELAH WiFi terhubung
// ─────────────────────────────────────────────────────────────
void ntpBegin() {
    _ntpClient.begin();
    Serial.println(F("[NTP] Client diinisialisasi."));
    Serial.print(F("[NTP] Server : "));
    Serial.println(NTP_SERVER);
    Serial.print(F("[NTP] Offset : UTC+"));
    Serial.println(NTP_UTC_OFFSET / 3600);
}

// ─────────────────────────────────────────────────────────────
//  ntpForceSync() - Paksa sinkronisasi NTP (blocking)
//
//  Mengirim UDP request dan MENUNGGU reply dari server.
//  Hanya panggil saat startup atau setelah reconnect WiFi.
//  JANGAN panggil berulang di dalam loop() setiap 500ms!
//
//  @return true jika server merespons dan waktu valid
// ─────────────────────────────────────────────────────────────
bool ntpForceSync() {
    Serial.println(F("[NTP] Memulai force sync..."));

    bool ok = _ntpClient.forceUpdate();

    if (ok) {
        _ntpSynced = true;
        Serial.print(F("[NTP] Sync OK - Waktu  : "));
        Serial.println(ntpGetTimeString());
        Serial.print(F("[NTP] Sync OK - Tanggal: "));
        Serial.println(ntpGetDateString());
        Serial.print(F("[NTP] Epoch time       : "));
        Serial.println(_ntpClient.getEpochTime());
    } else {
        Serial.println(F("[NTP] forceSync GAGAL - server tidak merespons."));
    }

    return ok;
}

// ─────────────────────────────────────────────────────────────
//  ntpTick() - Update ringan periodik (non-blocking)
//
//  NTPClient hanya mengirim request NTP jika updateInterval
//  sudah habis. Aman dipanggil setiap iterasi loop().
// ─────────────────────────────────────────────────────────────
void ntpTick() {
    _ntpClient.update();
}

// ─────────────────────────────────────────────────────────────
//  ntpGetTimeData() - Struct waktu yang sudah diparsing
//  @return TimeData berisi jam, menit, detik, tanggal, bulan, tahun
// ─────────────────────────────────────────────────────────────
TimeData ntpGetTimeData() {
    TimeData td;
    td.hour      = (uint8_t)_ntpClient.getHours();
    td.minute    = (uint8_t)_ntpClient.getMinutes();
    td.second    = (uint8_t)_ntpClient.getSeconds();
    td.dayOfWeek = (uint8_t)_ntpClient.getDay();
    _ntpEpochToDate(_ntpClient.getEpochTime(), td.day, td.month, td.year);
    return td;
}

// ─────────────────────────────────────────────────────────────
//  ntpGetTimeString() - Waktu dalam format "HH:MM:SS"
// ─────────────────────────────────────────────────────────────
String ntpGetTimeString() {
    return _ntpPad2((uint8_t)_ntpClient.getHours())   + ":" +
           _ntpPad2((uint8_t)_ntpClient.getMinutes()) + ":" +
           _ntpPad2((uint8_t)_ntpClient.getSeconds());
}

// ─────────────────────────────────────────────────────────────
//  ntpGetDateString() - Tanggal dalam format "DD-MM-YYYY"
// ─────────────────────────────────────────────────────────────
String ntpGetDateString() {
    uint8_t  d, m;
    uint16_t y;
    _ntpEpochToDate(_ntpClient.getEpochTime(), d, m, y);
    return _ntpPad2(d) + "-" + _ntpPad2(m) + "-" + String(y);
}

// ─────────────────────────────────────────────────────────────
//  ntpGetDayName() - Nama hari dalam Bahasa Indonesia
// ─────────────────────────────────────────────────────────────
String ntpGetDayName() {
    static const char* HARI[] = {
        "Minggu","Senin","Selasa","Rabu","Kamis","Jumat","Sabtu"
    };
    return String(HARI[_ntpClient.getDay() % 7]);
}

// ─────────────────────────────────────────────────────────────
//  ntpIsSynced() - Apakah sudah berhasil sinkronisasi minimal sekali?
// ─────────────────────────────────────────────────────────────
bool ntpIsSynced() {
    return _ntpSynced;
}

// ─────────────────────────────────────────────────────────────
//  ntpGetEpochTime() - Epoch time lokal (UTC + offset zona waktu)
// ─────────────────────────────────────────────────────────────
unsigned long ntpGetEpochTime() {
    return _ntpClient.getEpochTime();
}

// =============================================================
//  FUNGSI PRIVAT INTERNAL
// =============================================================

// ─────────────────────────────────────────────────────────────
//  _ntpIsLeapYear() - Cek tahun kabisat Gregorian
// ─────────────────────────────────────────────────────────────
static bool _ntpIsLeapYear(uint16_t y) {
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

// ─────────────────────────────────────────────────────────────
//  _ntpEpochToDate() - Konversi epoch lokal ke DD/MM/YYYY
//
//  Algoritma loop sederhana, bebas overflow & bug Julian:
//  1. Hitung jumlah hari sejak 1970-01-01
//  2. Kurangi per-tahun (sambil cek kabisat)
//  3. Kurangi per-bulan
// ─────────────────────────────────────────────────────────────
static void _ntpEpochToDate(unsigned long epochLocal,
                             uint8_t& day, uint8_t& month, uint16_t& year) {
    unsigned long dayCount = epochLocal / 86400UL;

    // ── Hitung tahun ──────────────────────────────────────────
    uint16_t y = 1970;
    while (true) {
        uint16_t diy = _ntpIsLeapYear(y) ? 366 : 365;
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
        if (m == 1 && _ntpIsLeapYear(y)) dim = 29; // Februari kabisat
        if (dayCount < dim) break;
        dayCount -= dim;
    }
    month = m + 1;                  // 1-indexed
    day   = (uint8_t)dayCount + 1; // 1-indexed
}

// ─────────────────────────────────────────────────────────────
//  _ntpPad2() - Format angka ke 2 digit dengan leading zero
// ─────────────────────────────────────────────────────────────
static String _ntpPad2(uint8_t val) {
    return (val < 10) ? ("0" + String(val)) : String(val);
}

#endif // NTP_MANAGER_H
