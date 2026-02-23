#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║          guardian.h - Orchestrator / State Machine           ║
 * ║              SONAR GUARDIAN - Smart Proximity System         ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  Modul ini adalah "otak" sistem — mengorkestrasi semua modul:
 *    - buzzer.h   → output suara
 *    - sonar.h    → input jarak
 *    - display.h  → output tampilan OLED
 *
 *  State machine:
 *    STATE_BOOT    → Tampil splash, mainkan melodi startup
 *    STATE_RUNNING → Loop utama: baca sensor, update display & buzzer
 *
 *  Event handling:
 *    - Perubahan zona → trigger efek suara + (opsional) log serial
 *    - Masuk DANGER   → buzzerPlayDangerPulse()
 *    - Kembali SAFE   → buzzerPlayAllClear()
 *
 *  PARADIGMA: PROSEDURAL
 *    - Fungsi publik: appSetup(), appLoop()
 */

#include <Arduino.h>
#include "../config.h"
#include "buzzer.h"
#include "sonar.h"
#include "display.h"

// ─────────────────────────────────────────────────────────────────
//  STATE MACHINE
// ─────────────────────────────────────────────────────────────────
#define STATE_BOOT    0
#define STATE_RUNNING 1
static int _app_state = STATE_BOOT;

// ─────────────────────────────────────────────────────────────────
//  _appHandleZoneChange(newZone, oldZone)
//  Dipanggil saat zona jarak berubah — memicu efek suara
// ─────────────────────────────────────────────────────────────────
static void _appHandleZoneChange(int newZone, int oldZone) {
    if (newZone == ZONE_DANGER) {
        // Masuk zona bahaya → alarm nada tinggi
        buzzerPlayDangerPulse();
        Serial.println(F("[GUARDIAN] Zone changed → DANGER"));
    } else if (newZone == ZONE_SAFE && oldZone != ZONE_SAFE) {
        // Kembali ke zona aman → all clear
        buzzerPlayAllClear();
        Serial.println(F("[GUARDIAN] Zone changed → SAFE"));
    } else if (newZone == ZONE_WARNING) {
        Serial.println(F("[GUARDIAN] Zone changed → WARNING"));
    }
}

// ─────────────────────────────────────────────────────────────────
//  _appPrintSerialStatus() - Debug info ke Serial Monitor
// ─────────────────────────────────────────────────────────────────
static unsigned long _app_lastSerial = 0;
static void _appPrintSerialStatus() {
    unsigned long now = millis();
    if (now - _app_lastSerial < 500) return;
    _app_lastSerial = now;

    Serial.print(F("[SONAR] Raw: "));
    Serial.print(sonarGetRawDistance(), 1);
    Serial.print(F(" cm | Smooth: "));
    Serial.print(sonarGetDistance(), 1);
    Serial.print(F(" cm | Zone: "));
    Serial.println(sonarGetZoneName());
}

// ─────────────────────────────────────────────────────────────────
//  appSetup() - Inisialisasi seluruh sistem
//  Dipanggil dari setup() di file .ino
// ─────────────────────────────────────────────────────────────────
void appSetup() {
    Serial.begin(9600);
    Serial.println();
    Serial.println(F("╔══════════════════════════════╗"));
    Serial.println(F("║     SONAR GUARDIAN v1.0      ║"));
    Serial.println(F("║  Smart Proximity Alert Sys   ║"));
    Serial.println(F("╚══════════════════════════════╝"));

    // Inisialisasi buzzer
    buzzerSetup();
    Serial.println(F("[INIT] Buzzer OK"));

    // Inisialisasi sensor
    sonarSetup();
    Serial.println(F("[INIT] Sonar sensor OK"));

    // Inisialisasi OLED
    if (!displaySetup()) {
        Serial.println(F("[INIT] OLED FAILED! Check wiring."));
        tone(PIN_BUZZER, 400, 200);
        delay(250);
        tone(PIN_BUZZER, 400, 200);
        delay(250);
        noTone(PIN_BUZZER);
        // Tetap lanjut meski OLED gagal
    } else {
        Serial.println(F("[INIT] OLED OK"));
    }

    // Tampilkan splash screen
    displayShowSplash();

    // Mainkan melodi startup
    if (STARTUP_MELODY_ENABLED) {
        buzzerPlayStartupMelody();
    }

    Serial.println(F("[INIT] System ready! Entering main loop..."));
    _app_state = STATE_RUNNING;
}

// ─────────────────────────────────────────────────────────────────
//  appLoop() - Loop utama sistem
//  Dipanggil dari loop() di file .ino
// ─────────────────────────────────────────────────────────────────
void appLoop() {
    if (_app_state != STATE_RUNNING) return;

    // ── 1. Baca sensor (non-blocking) ──
    sonarLoop();
    int currZone = sonarGetZone();
    float dist   = sonarGetDistance();

    // ── 2. Handle perubahan zona ──
    // [FIX] Gunakan sonarGetPrevZone() yang sinkron dengan flag internal sonar
    if (sonarZoneChanged()) {
        int prevZone = sonarGetPrevZone();
        _appHandleZoneChange(currZone, prevZone);
        sonarClearZoneChange();
    }

    // ── 3. Update parameter alarm buzzer ──
    buzzerSetAlarm(currZone, dist);

    // ── 4. Tick buzzer (non-blocking) ──
    buzzerLoop();

    // ── 5. Update tampilan OLED (non-blocking) ──
    displayLoop(dist, currZone);

    // ── 6. Debug serial (non-blocking) ──
    _appPrintSerialStatus();
}
