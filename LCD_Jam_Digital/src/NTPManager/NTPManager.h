/**
 * ============================================================
 *  FILE: src/NTPManager/NTPManager.h
 *  Deskripsi: Class untuk sinkronisasi waktu via NTP server
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "../../config.h"

// Struktur untuk menyimpan data waktu yang sudah diparsing
struct TimeData {
    uint8_t  hour;      // Jam     : 0-23
    uint8_t  minute;    // Menit   : 0-59
    uint8_t  second;    // Detik   : 0-59
    uint8_t  day;       // Tanggal : 1-31
    uint8_t  month;     // Bulan   : 1-12
    uint16_t year;      // Tahun   : e.g. 2026
    uint8_t  dayOfWeek; // Hari    : 0=Minggu, 1=Senin, ..., 6=Sabtu
};

class NTPManager {
public:
    NTPManager(const char* server, long utcOffset, unsigned long updateInterval);

    /**
     * Inisialisasi NTP client - dipanggil setelah WiFi terhubung
     */
    void begin();

    /**
     * FORCE sinkronisasi ke server NTP (blocking, menunggu UDP response).
     * Gunakan HANYA saat startup / setelah reconnect WiFi.
     * @return true jika server menjawab dan waktu valid
     */
    bool forceSync();

    /**
     * Tick ringan - hanya update jika interval sudah habis (non-blocking).
     * Aman dipanggil setiap loop() tanpa throttle.
     */
    void tick();

    /**
     * Mendapatkan data waktu yang sudah diparsing
     */
    TimeData getTimeData();

    /**
     * Mendapatkan waktu dalam format "HH:MM:SS"
     */
    String getTimeString();

    /**
     * Mendapatkan tanggal dalam format "DD-MM-YYYY"
     */
    String getDateString();

    /**
     * Mendapatkan nama hari dalam Bahasa Indonesia
     */
    String getDayName();

    /**
     * Apakah sudah berhasil sinkronisasi minimal sekali?
     */
    bool isSynced() const;

    /**
     * Epoch time lokal (UTC + offset zona waktu)
     */
    unsigned long getEpochTime();

private:
    WiFiUDP   _udp;
    NTPClient _client;
    bool      _synced;

    /**
     * Konversi epoch lokal ke DD/MM/YYYY
     * Menggunakan algoritma loop sederhana, bebas overflow & bug Julian.
     */
    void _epochToDate(unsigned long epochLocal,
                      uint8_t& day, uint8_t& month, uint16_t& year);

    bool   _isLeapYear(uint16_t y) const;
    String _pad2(uint8_t val) const;
};

#endif // NTP_MANAGER_H
