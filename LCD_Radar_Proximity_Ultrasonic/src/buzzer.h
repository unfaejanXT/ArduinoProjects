#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║              buzzer.h - Modul Buzzer & Melodi                ║
 * ║              SONAR GUARDIAN - Smart Proximity System         ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  Modul ini mengelola semua output suara:
 *    - Melodi startup animatif
 *    - Alarm beep adaptif berdasarkan zona jarak
 *    - Efek suara khusus (danger pulse, all-clear)
 *
 *  PARADIGMA: PROSEDURAL
 *    - State disimpan di variabel global static
 *    - Tidak ada instansiasi class
 */

#include <Arduino.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────────
//  STATE INTERNAL (static = scope file ini saja)
// ─────────────────────────────────────────────────────────────────
static bool          _buz_beepActive    = false;
static unsigned long _buz_lastToggle    = 0;
static unsigned long _buz_beepInterval  = BEEP_INTERVAL_MAX;
static int           _buz_currentFreq   = 0;
static int           _buz_beepDur       = 0;

// ─────────────────────────────────────────────────────────────────
//  NOTE FREQUENCIES (untuk melodi & efek suara)
//  Harus didefinisikan SEBELUM array yang menggunakannya
// ─────────────────────────────────────────────────────────────────
#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_A4   440
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_D5   587
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_G5   784
#define NOTE_A5   880
#define NOTE_REST   0

// ── State machine untuk efek suara non-blocking ──────────────────
// Efek multi-nada (DangerPulse / AllClear) dijalankan tanpa delay()
#define FX_IDLE       0
#define FX_DANGER     1
#define FX_ALLCLEAR   2
static int           _buz_fxState      = FX_IDLE;
static int           _buz_fxStep       = 0;
static unsigned long _buz_fxLastStep   = 0;

// Tabel nada efek DANGER (frekuensi, durasi)
static const int _buz_dangerFreqs[] = { 1800, 1400, 1000 };
static const int _buz_dangerDurs[]  = {   80,   80,  110 };
#define BUZ_DANGER_STEPS 3

// Tabel nada efek ALL CLEAR (frekuensi, durasi)
// NOTE_C5/E5/G5 sudah terdefinisi di atas → tidak error
static const int _buz_clearFreqs[]  = { NOTE_C5, NOTE_E5, NOTE_G5 };
static const int _buz_clearDurs[]   = {      90,      90,     160 };
#define BUZ_CLEAR_STEPS 3

// ─────────────────────────────────────────────────────────────────
//  buzzerSetup() - Inisialisasi pin buzzer
// ─────────────────────────────────────────────────────────────────
void buzzerSetup() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

// ─────────────────────────────────────────────────────────────────
//  buzzerTone(freq, dur) - Mainkan nada dengan frekuensi & durasi
// ─────────────────────────────────────────────────────────────────
void buzzerTone(int freq, int duration) {
    if (freq > 0) {
        tone(PIN_BUZZER, freq, duration);
    } else {
        noTone(PIN_BUZZER);
    }
}

// ─────────────────────────────────────────────────────────────────
//  buzzerSilence() - Hentikan suara
// ─────────────────────────────────────────────────────────────────
void buzzerSilence() {
    noTone(PIN_BUZZER);
    digitalWrite(PIN_BUZZER, LOW);
    _buz_beepActive = false;
    _buz_fxState    = FX_IDLE;   // Reset efek jika sedang berjalan
}

// ─────────────────────────────────────────────────────────────────
//  buzzerPlayStartupMelody() - Melodi boot animatif
//  → Blocking (dijalankan sekali saat setup)
// ─────────────────────────────────────────────────────────────────
void buzzerPlayStartupMelody() {
    // Melodi startup — dipercepat, total ~1.1 detik
    // Gap antar nada dikurangi dari +30ms → +15ms
    static const int notes[]     = { NOTE_E4, NOTE_G4, NOTE_B4, NOTE_E5,
                                     NOTE_G5, NOTE_REST };
    static const int durations[] = {      80,      80,      80,     200,
                                          280,        0 };
    const int count = sizeof(notes) / sizeof(notes[0]);

    for (int i = 0; i < count; i++) {
        if (notes[i] == NOTE_REST) {
            noTone(PIN_BUZZER);
        } else {
            tone(PIN_BUZZER, notes[i], durations[i]);
        }
        delay(durations[i] + 15);   // Gap 15ms (dari 30ms)
    }
    noTone(PIN_BUZZER);

    // Konfirmasi READY: dua beep naik (dipercepat dari 3×130ms → 2×90ms)
    static const int confirmNotes[] = { NOTE_E5, NOTE_G5 };
    for (int i = 0; i < 2; i++) {
        tone(PIN_BUZZER, confirmNotes[i], 70);
        delay(95);
    }
    noTone(PIN_BUZZER);
}

// ─────────────────────────────────────────────────────────────────
//  buzzerPlayDangerPulse() - Trigger efek DANGER (NON-BLOCKING)
//  Efek diputar bertahap di dalam buzzerFxTick() setiap loop
// ─────────────────────────────────────────────────────────────────
void buzzerPlayDangerPulse() {
    _buz_fxState   = FX_DANGER;
    _buz_fxStep    = 0;
    _buz_fxLastStep = millis();
    // Langsung mainkan nada pertama
    tone(PIN_BUZZER, _buz_dangerFreqs[0], _buz_dangerDurs[0]);
}

// ─────────────────────────────────────────────────────────────────
//  buzzerPlayAllClear() - Trigger efek ALL CLEAR (NON-BLOCKING)
// ─────────────────────────────────────────────────────────────────
void buzzerPlayAllClear() {
    _buz_fxState    = FX_ALLCLEAR;
    _buz_fxStep     = 0;
    _buz_fxLastStep = millis();
    tone(PIN_BUZZER, _buz_clearFreqs[0], _buz_clearDurs[0]);
}

// ─────────────────────────────────────────────────────────────────
//  buzzerFxTick() - Advance multi-nada effect (NON-BLOCKING)
//  Dipanggil di dalam buzzerLoop() setiap iterasi
// ─────────────────────────────────────────────────────────────────
static void buzzerFxTick() {
    if (_buz_fxState == FX_IDLE) return;

    unsigned long now = millis();
    const int* freqs;
    const int* durs;
    int steps;

    if (_buz_fxState == FX_DANGER) {
        freqs = _buz_dangerFreqs;
        durs  = _buz_dangerDurs;
        steps = BUZ_DANGER_STEPS;
    } else {
        freqs = _buz_clearFreqs;
        durs  = _buz_clearDurs;
        steps = BUZ_CLEAR_STEPS;
    }

    // Cek apakah sudah waktunya ke langkah berikutnya
    int waitMs = durs[_buz_fxStep] + 15;   // durasi nada + gap 15ms
    if (now - _buz_fxLastStep < (unsigned long)waitMs) return;

    _buz_fxStep++;
    _buz_fxLastStep = now;

    if (_buz_fxStep >= steps) {
        // Efek selesai
        noTone(PIN_BUZZER);
        _buz_fxState = FX_IDLE;
    } else {
        tone(PIN_BUZZER, freqs[_buz_fxStep], durs[_buz_fxStep]);
    }
}

// ─────────────────────────────────────────────────────────────────
//  buzzerSetAlarm(zone, distCm) - Set parameter alarm berdasarkan zona
//    zone: 0 = SAFE, 1 = WARNING, 2 = DANGER
// ─────────────────────────────────────────────────────────────────
void buzzerSetAlarm(int zone, float distCm) {
    if (zone == 0) {
        // SAFE → diam
        _buz_currentFreq  = 0;
        _buz_beepDur      = 0;
        _buz_beepInterval = 5000; // efektif diam
    } else if (zone == 1) {
        // WARNING → beep interval diperkecil makin dekat
        _buz_currentFreq = BEEP_FREQ_WARNING;
        _buz_beepDur     = BEEP_DUR_WARNING;

        // Mapping jarak ke interval: semakin dekat → interval makin kecil
        float ratio = constrain((distCm - ZONE_DANGER_CM) /
                                (float)(ZONE_WARNING_CM - ZONE_DANGER_CM), 0.0, 1.0);
        _buz_beepInterval = (unsigned long)(BEEP_INTERVAL_MIN +
                            ratio * (BEEP_INTERVAL_MAX - BEEP_INTERVAL_MIN));
    } else {
        // DANGER → beep sangat cepat & tinggi
        _buz_currentFreq = BEEP_FREQ_DANGER;
        _buz_beepDur     = BEEP_DUR_DANGER;

        // Makin dekat makin cepat (interval lebih kecil dari BEEP_INTERVAL_MIN)
        float ratio = constrain(distCm / (float)ZONE_DANGER_CM, 0.0, 1.0);
        _buz_beepInterval = (unsigned long)(BEEP_INTERVAL_MIN * ratio + 30);
    }
}

// ─────────────────────────────────────────────────────────────────
//  buzzerLoop() - Update alarm + efek suara (non-blocking)
//  Panggil di loop() setiap iterasi
// ─────────────────────────────────────────────────────────────────
void buzzerLoop() {
    // Tick efek multi-nada (DangerPulse / AllClear) dulu
    // Efek punya prioritas → skip alarm beep selama efek berjalan
    if (_buz_fxState != FX_IDLE) {
        buzzerFxTick();
        return;
    }

    // Alarm beep adaptif
    if (_buz_currentFreq == 0) {
        noTone(PIN_BUZZER);
        _buz_beepActive = false;
        return;
    }

    unsigned long now     = millis();
    unsigned long elapsed = now - _buz_lastToggle;

    if (!_buz_beepActive) {
        if (elapsed >= _buz_beepInterval) {
            tone(PIN_BUZZER, _buz_currentFreq, _buz_beepDur);
            _buz_beepActive = true;
            _buz_lastToggle = now;
        }
    } else {
        if (elapsed >= (unsigned long)_buz_beepDur) {
            noTone(PIN_BUZZER);
            _buz_beepActive = false;
            _buz_lastToggle = now;
        }
    }
}
