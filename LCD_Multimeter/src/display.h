#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────────
//  OBJEK OLED
// ─────────────────────────────────────────────────────────────────
static Adafruit_SSD1306 _display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
static bool _display_initialized = false;

// ─────────────────────────────────────────────────────────────────
//  displaySetup()
// ─────────────────────────────────────────────────────────────────
void displaySetup() {
    Wire.begin();
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        _display_initialized = false;
        return;
    }
    _display_initialized = true;
    _display.clearDisplay();
    
    // Tampilkan tulisan Standby awal agar user tahu OLED berfungsi
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(22, 25);
    _display.print(F("SYSTEM STANDBY"));
    _display.setCursor(10, 40);
    _display.print(F("Press Button (D2) ->"));
    _display.display();
}

// ─────────────────────────────────────────────────────────────────
//  UPDATE TAMPILAN UTAMA (METER)
// ─────────────────────────────────────────────────────────────────
// Menerima data dari sensors.h
void displayUpdateMeter(float volt, float current, float power, bool relayOn) {
    if (!_display_initialized) return;
    
    _display.clearDisplay();
    
    // Header
    _display.fillRect(0, 0, OLED_WIDTH, 14, SSD1306_WHITE);
    _display.setTextColor(SSD1306_BLACK);
    _display.setTextSize(1);
    _display.setCursor(2, 3);
    _display.print(F(" SMART MULTIMETER "));
    
    // Nilai Voltase & Arus
    _display.setTextColor(SSD1306_WHITE);
    
    // V
    _display.setTextSize(2);
    _display.setCursor(0, 18);
    _display.print(volt, 1);
    _display.setTextSize(1);
    _display.print(F("V"));
    
    // A
    _display.setTextSize(2);
    _display.setCursor(64, 18);
    _display.print(current, 2);
    _display.setTextSize(1);
    _display.print(F("A"));
    
    // Daya (P) di Bawah - Lebih besar sedikit
    _display.drawLine(0, 38, OLED_WIDTH, 38, SSD1306_WHITE);
    _display.setTextSize(2);
    _display.setCursor(14, 44);
    _display.print(power, 1);
    _display.setTextSize(1);
    _display.print(F(" Watt"));
    
    _display.display();
}
