#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║            sonar.h - Modul Sensor Ultrasonik HC-SR04         ║
 * ║              SONAR GUARDIAN - Smart Proximity System         ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  Modul ini mengelola pembacaan sensor HC-SR04:
 *    - Pengukuran jarak dengan timeout aman
 *    - Moving average (smoothing) untuk hasil stabil
 *    - Deteksi zona (SAFE / WARNING / DANGER)
 *    - Pendeteksian perubahan zona
 *
 *  PARADIGMA: PROSEDURAL
 *    - State disimpan di variabel global static
 *    - Tidak ada instansiasi class
 */

#include <Arduino.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────────
//  KONSTANTA ZONA
// ─────────────────────────────────────────────────────────────────
#define ZONE_SAFE    0
#define ZONE_WARNING 1
#define ZONE_DANGER  2

// ─────────────────────────────────────────────────────────────────
//  STATE INTERNAL
// ─────────────────────────────────────────────────────────────────
static float         _sonar_rawDist     = (float)DIST_DISPLAY_MAX; // [FIX] Mulai dari jauh (SAFE)
static float         _sonar_smoothDist  = (float)DIST_DISPLAY_MAX; // [FIX] Bukan 0 → hindari false DANGER saat boot
static float         _sonar_samples[SMOOTH_SAMPLES];
static int           _sonar_sampleIdx   = 0;
static bool          _sonar_initialized = false;
static int           _sonar_currentZone = ZONE_SAFE;               // [FIX] Eksplisit SAFE
static int           _sonar_prevZone    = ZONE_SAFE;
static unsigned long _sonar_lastRead    = 0;
static bool          _sonar_zoneChanged = false;                    // [FIX] Flag perubahan zona terpisah

// ─────────────────────────────────────────────────────────────────
//  sonarSetup() - Inisialisasi pin sensor
// ─────────────────────────────────────────────────────────────────
void sonarSetup() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);

    // Isi array sample dengan nilai awal (jauh = SAFE)
    for (int i = 0; i < SMOOTH_SAMPLES; i++) {
        _sonar_samples[i] = (float)DIST_DISPLAY_MAX;
    }
    // [FIX] Sinkronkan state awal agar tidak ada false DANGER saat boot
    _sonar_rawDist     = (float)DIST_DISPLAY_MAX;
    _sonar_smoothDist  = (float)DIST_DISPLAY_MAX;
    _sonar_currentZone = ZONE_SAFE;
    _sonar_prevZone    = ZONE_SAFE;
    _sonar_zoneChanged = false;
    _sonar_initialized = true;
}

// ─────────────────────────────────────────────────────────────────
//  _sonarReadRaw() - Baca jarak mentah dari sensor (cm)
//  Mengembalikan -1 jika timeout / tidak ada objek terdeteksi
// ─────────────────────────────────────────────────────────────────
static float _sonarReadRaw() {
    // Pastikan TRIG LOW dulu
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);

    // Kirim pulse 10µs ke TRIG
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    // Ukur durasi ECHO HIGH (dengan timeout)
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, SONAR_TIMEOUT_US);

    if (duration == 0) {
        return -1.0; // Timeout → tidak terdeteksi
    }

    // Konversi ke cm: kecepatan suara ~0.0343 cm/µs, dibagi 2 (pulang-pergi)
    float distCm = duration * 0.0343f / 2.0f;

    // Filter nilai di luar range sensor
    if (distCm < 2.0f || distCm > DIST_MAX_CM) {
        return -1.0;
    }

    return distCm;
}

// ─────────────────────────────────────────────────────────────────
//  _sonarMovingAverage(newVal) - Hitung moving average
// ─────────────────────────────────────────────────────────────────
static float _sonarMovingAverage(float newVal) {
    _sonar_samples[_sonar_sampleIdx] = newVal;
    _sonar_sampleIdx = (_sonar_sampleIdx + 1) % SMOOTH_SAMPLES;

    float sum = 0.0;
    for (int i = 0; i < SMOOTH_SAMPLES; i++) {
        sum += _sonar_samples[i];
    }
    return sum / SMOOTH_SAMPLES;
}

// ─────────────────────────────────────────────────────────────────
//  sonarLoop() - Update pembacaan sensor (non-blocking)
//  Panggil di loop() setiap iterasi
// ─────────────────────────────────────────────────────────────────
void sonarLoop() {
    unsigned long now = millis();
    if (now - _sonar_lastRead < SENSOR_READ_MS) return;
    _sonar_lastRead = now;

    float raw = _sonarReadRaw();

    // [FIX] Jika sensor timeout/tidak ada objek → anggap jarak maksimum (SAFE)
    //       Jangan pertahankan nilai lama → bisa stuck di DANGER selamanya!
    if (raw < 0) {
        raw = (float)DIST_DISPLAY_MAX;
    }

    _sonar_rawDist    = raw;
    _sonar_smoothDist = _sonarMovingAverage(raw);

    // [FIX] Update zona & set flag perubahan — prevZone TIDAK diubah di sini
    //       prevZone hanya diubah saat sonarClearZoneChange() dipanggil
    //       Sehingga sonarZoneChanged() selalu akurat dari luar
    int newZone;
    if (_sonar_smoothDist <= ZONE_DANGER_CM) {
        newZone = ZONE_DANGER;
    } else if (_sonar_smoothDist <= ZONE_WARNING_CM) {
        newZone = ZONE_WARNING;
    } else {
        newZone = ZONE_SAFE;
    }

    if (newZone != _sonar_currentZone) {
        _sonar_zoneChanged = true;  // Tandai ada perubahan
    }
    _sonar_currentZone = newZone;
}

// ─────────────────────────────────────────────────────────────────
//  GETTER FUNCTIONS
// ─────────────────────────────────────────────────────────────────

// Jarak terhaluskan (moving average) dalam cm
float sonarGetDistance() {
    return _sonar_smoothDist;
}

// Jarak mentah (raw) dalam cm
float sonarGetRawDistance() {
    return _sonar_rawDist;
}

// Zona saat ini: ZONE_SAFE / ZONE_WARNING / ZONE_DANGER
int sonarGetZone() {
    return _sonar_currentZone;
}

// Cek apakah zona baru saja berubah
// [FIX] Gunakan flag dedicated, bukan perbandingan prev vs current
//       karena prevZone bisa tidak sinkron jika sonarLoop() belum dipanggil
bool sonarZoneChanged() {
    return _sonar_zoneChanged;
}

// [FIX] Zona sebelum perubahan (untuk keperluan handler di guardian.h)
int sonarGetPrevZone() {
    return _sonar_prevZone;
}

// Reset pendeteksian perubahan zona (panggil setelah diproses)
void sonarClearZoneChange() {
    _sonar_prevZone    = _sonar_currentZone;
    _sonar_zoneChanged = false;
}

// Nama zona dalam string (untuk debugging / display)
const char* sonarGetZoneName() {
    switch (_sonar_currentZone) {
        case ZONE_DANGER:  return "DANGER";
        case ZONE_WARNING: return "WARNING";
        default:           return "SAFE";
    }
}

// Persen jarak (0-100%) dari DIST_DISPLAY_MAX, untuk visualisasi bar
int sonarGetDistPercent() {
    float capped = constrain(_sonar_smoothDist, 0.0, (float)DIST_DISPLAY_MAX);
    return (int)((capped / (float)DIST_DISPLAY_MAX) * 100.0f);
}
