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
//  _debugTestSensors()
//  Pembacaan serial loop saat start seperti permintaan user
// ─────────────────────────────────────────────────────────────────
static void _debugTestSensors() {
    Serial.println(F("\n=== Memuat Kerja Sensor (Hitung Kalibrasi) ==="));
    
    // UJI 1: ZMPT1010B Relay OFF
    Serial.println(F("AC Voltage Sensor (Relay OFF)"));
    digitalWrite(PIN_RELAY, HIGH); // OFF
    for (int i = 1; i <= 10; i++) {
        sensorsForceRead(); // paksa kalkulasi tanpa terblokir timer 500ms
        Serial.print(F("  Looping ")); Serial.print(i);
        Serial.print(F(" | ADC RMS: ")); Serial.print(sensorsGetRawRMSVoltage());
        Serial.print(F(" | x K(")); Serial.print(VOLT_CALIBRATION); Serial.print(F(") -> "));
        Serial.print(sensorsGetVoltage()); Serial.println(F(" V"));
        delay(100);
    }
    
    // UJI 2: ZMPT1010B Relay ON
    Serial.println(F("AC Voltage Sensor (Relay ON)"));
    digitalWrite(PIN_RELAY, LOW); // ON
    for (int i = 1; i <= 10; i++) {
        sensorsForceRead();
        Serial.print(F("  Looping ")); Serial.print(i);
        Serial.print(F(" | ADC RMS: ")); Serial.print(sensorsGetRawRMSVoltage());
        Serial.print(F(" | x K(")); Serial.print(VOLT_CALIBRATION); Serial.print(F(") -> "));
        Serial.print(sensorsGetVoltage()); Serial.println(F(" V"));
        delay(100);
    }
    Serial.println(F("Selesai"));

    // UJI 3: ACS712 Relay OFF
    Serial.println(F("AC Current Sensor (Relay OFF)"));
    digitalWrite(PIN_RELAY, HIGH); // OFF
    for (int i = 1; i <= 10; i++) {
        sensorsForceRead();
        Serial.print(F("  Looping ")); Serial.print(i);
        Serial.print(F(" | ADC RMS: ")); Serial.print(sensorsGetRawRMSCurrent());
        Serial.print(F(" | -> "));
        Serial.print(sensorsGetCurrent()); Serial.println(F(" A"));
        delay(100);
    }

    // UJI 4: ACS712 Relay ON
    Serial.println(F("AC Current Sensor (Relay ON)"));
    digitalWrite(PIN_RELAY, LOW); // ON
    for (int i = 1; i <= 10; i++) {
        sensorsForceRead();
        Serial.print(F("  Looping ")); Serial.print(i);
        Serial.print(F(" | ADC RMS: ")); Serial.print(sensorsGetRawRMSCurrent());
        Serial.print(F(" | -> "));
        Serial.print(sensorsGetCurrent()); Serial.println(F(" A"));
        delay(100);
    }
    Serial.println(F("Selesai\n"));
    
    // Kembalikan ke Standby (OFF)
    digitalWrite(PIN_RELAY, HIGH);
}

// ─────────────────────────────────────────────────────────────────
//  appSetup()
// ─────────────────────────────────────────────────────────────────
void appSetup() {
    Serial.begin(115200);
    Serial.println(F("\n[APP] Memulai SMART MULTIMETER PROSEDURAL..."));
    
    // Inisialisasi Modul
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Relay (Active-LOW), jadi HIGH = MATI di awal
    
    buzzerSetup();
    displaySetup();
    sensorsSetup();
    
    // Jalankan Uji Sensor di Serial Monitor
    _debugTestSensors();
    
    _app_isOn = false;
    
    Serial.println(F("[APP] Setup Selesai (Standby). Menunggu Tombol ditekan..."));
}

// ─────────────────────────────────────────────────────────────────
//  TRANSIF STATE - Menyalakan Sistem
// ─────────────────────────────────────────────────────────────────
static void _appTurnOn() {
    Serial.println(F("[APP] Menyalakan Multimeter..."));
    _app_isOn = true;
    
    // Bunyi Beep Startup & Animasi secara berurutan
    // (Bisa diperbaiki jadi async bila mau, tapi karena ini transisi, synchronous cukup keren)
    buzzerStartupMelody();
    displayAnimStartup();
    
    // Hidupkan Relay (ON)
    digitalWrite(PIN_RELAY, LOW); // Beri logika LOW agar Relay cetek/nyala
    Serial.println(F("[APP] Relay ON. Sistem Aktif."));
}

// ─────────────────────────────────────────────────────────────────
//  TRANSIF STATE - Mematikan Sistem
// ─────────────────────────────────────────────────────────────────
static void _appTurnOff() {
    Serial.println(F("[APP] Mematikan Multimeter..."));
    _app_isOn = false;
    
    // Matikan Relay (OFF)
    digitalWrite(PIN_RELAY, HIGH); // Beri logika HIGH agar Relay putus/mati
    
    // Animasi dan Bunyi
    buzzerShutdownMelody();
    displayAnimShutdown();
    Serial.println(F("[APP] Relay OFF. Sistem Standby."));
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
    
    // Jika ON, baca sensor dan update display
    if (_app_isOn) {
        sensorsLoop(); // Modul sensor (RMS + Moving Average) akan jalan
        
        // Update display secara real-time
        static unsigned long lastDisplayUpdate = 0;
        if (millis() - lastDisplayUpdate >= 250) { // 4 kali per detik
            lastDisplayUpdate = millis();
            
            float v = sensorsGetVoltage();
            float i = sensorsGetCurrent();
            float p = sensorsGetPower();
            
            displayUpdateMeter(v, i, p);
        }
    }
}
