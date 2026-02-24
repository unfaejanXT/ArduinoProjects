#pragma once

#include <Arduino.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────────
//  buzzerSetup()
// ─────────────────────────────────────────────────────────────────
void buzzerSetup() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW); // Pastikan mati
}

// ─────────────────────────────────────────────────────────────────
//  Nada
// ─────────────────────────────────────────────────────────────────
void buzzerBeep(int freq, int duration) {
    if (freq > 0) {
        tone(PIN_BUZZER, freq);
        delay(duration);
        noTone(PIN_BUZZER);
    } else {
        noTone(PIN_BUZZER);
        delay(duration);
    }
}

// ─────────────────────────────────────────────────────────────────
//  Melodi Startup - dimainkan secara sinkron saat animasi
// ─────────────────────────────────────────────────────────────────
void buzzerStartupMelody() {
    buzzerBeep(FREQ_STARTUP_1, 100);
    delay(50);
    buzzerBeep(FREQ_STARTUP_2, 100);
    delay(50);
    buzzerBeep(FREQ_STARTUP_3, 200);
}

// ─────────────────────────────────────────────────────────────────
//  Melodi Shutdown - dimainkan secara sinkron saat mati
// ─────────────────────────────────────────────────────────────────
void buzzerShutdownMelody() {
    buzzerBeep(FREQ_SHUTDOWN_1, 100);
    delay(50);
    buzzerBeep(FREQ_SHUTDOWN_2, 100);
    delay(50);
    buzzerBeep(FREQ_SHUTDOWN_3, 200);
}

// ─────────────────────────────────────────────────────────────────
//  Nada Pendek - Untuk tombol
// ─────────────────────────────────────────────────────────────────
void buzzerShortBeep() {
    buzzerBeep(2000, 50);
}
