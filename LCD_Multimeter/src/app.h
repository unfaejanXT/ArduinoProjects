#pragma once

#include <Arduino.h>
#include "../config.h"
#include "buzzer.h"
#include "display.h"
#include "sensors.h"

// ─────────────────────────────────────────────────────────────────
//  STATE UTAMA
// ─────────────────────────────────────────────────────────────────
static bool _app_isOn = false;

// ─────────────────────────────────────────────────────────────────
//  STATE BUTTON (DEBOUNCE)
// ─────────────────────────────────────────────────────────────────
static bool          _app_btnState     = HIGH;
static bool          _app_lastBtnRead  = HIGH;
static unsigned long _app_lastDebounce = 0;

// ─────────────────────────────────────────────────────────────────
//  appSetup()
// ─────────────────────────────────────────────────────────────────
void appSetup() {
    // Inisialisasi Modul
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Relay (Active-LOW), jadi HIGH = MATI di awal
    
    buzzerSetup();
    displaySetup();
    sensorsSetup();
    
    _app_isOn = false;
}

// ─────────────────────────────────────────────────────────────────
//  TRANSIF STATE - Menyalakan Sistem
// ─────────────────────────────────────────────────────────────────
static void _appTurnOn() {
    _app_isOn = true;
    
    // Bunyi Beep Startup
    buzzerStartupMelody();
    
    // Hidupkan Relay (ON)
    digitalWrite(PIN_RELAY, LOW); // Beri logika LOW agar Relay cetek/nyala
}

// ─────────────────────────────────────────────────────────────────
//  TRANSIF STATE - Mematikan Sistem
// ─────────────────────────────────────────────────────────────────
static void _appTurnOff() {
    _app_isOn = false;
    
    // Matikan Relay (OFF)
    digitalWrite(PIN_RELAY, HIGH); // Beri logika HIGH agar Relay putus/mati
    
    // Bunyi
    buzzerShutdownMelody();
    displayUpdateMeter(0.0, 0.0, 0.0, false);
}

// ─────────────────────────────────────────────────────────────────
//  TOMBOL DEBOUNCE LOOP
// ─────────────────────────────────────────────────────────────────
static void _appButtonLoop() {
    bool reading = digitalRead(PIN_BTN);
    
    if (reading != _app_lastBtnRead) {
        _app_lastDebounce = millis();
    }
    
    if ((millis() - _app_lastDebounce) > DEBOUNCE_MS) {
        if (reading != _app_btnState) {
            _app_btnState = reading;
            
            // Tombol ditekan (LOW karena PULLUP)
            if (_app_btnState == LOW) {
                buzzerShortBeep(); // Bunyi klik kecil
                
                // Toggle State
                if (_app_isOn) {
                    _appTurnOff();
                } else {
                    _appTurnOn();
                }
            }
        }
    }
    _app_lastBtnRead = reading;
}

// ─────────────────────────────────────────────────────────────────
//  LOOP UTAMA - appLoop()
// ─────────────────────────────────────────────────────────────────
void appLoop() {
    // Selalu baca tombol meskipun OFF
    _appButtonLoop();
    
    // Jika ON, baca sensor
    if (_app_isOn) {
        sensorsLoop(); // Modul sensor (RMS + Moving Average) akan jalan
    }

    // Update display secara real-time di mode ON/OFF (tetap full monitor)
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate >= 250) { // 4 kali per detik
        lastDisplayUpdate = millis();

        float v = _app_isOn ? sensorsGetVoltage() : 0.0;
        float i = _app_isOn ? sensorsGetCurrent() : 0.0;
        float p = _app_isOn ? sensorsGetPower() : 0.0;
        displayUpdateMeter(v, i, p, _app_isOn);
    }
}
