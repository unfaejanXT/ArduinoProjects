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
        Serial.println(F("OLED SSD1306 Alokasi Gagal. Periksa I2C."));
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
//  ANIMASI STARTUP
// ─────────────────────────────────────────────────────────────────
void displayAnimStartup() {
    if (!_display_initialized) return;
    
    // Tampilkan tulisan "MULTIMETER" mendatar dari bawah lalu meluas (keren)
    for (int y = OLED_HEIGHT; y >= 20; y -= 4) {
        _display.clearDisplay();
        
        _display.setTextSize(2);
        _display.setTextColor(SSD1306_WHITE);
        _display.setCursor(4, y);
        _display.print(F("MULTIMETER"));
        
        _display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
        
        _display.display();
        delay(30);
    }
    
    _display.setTextSize(1);
    _display.setCursor(20, 45);
    _display.print(F("System Ready..."));
    _display.display();
    delay(500);
}

// ─────────────────────────────────────────────────────────────────
//  ANIMASI SHUTDOWN
// ─────────────────────────────────────────────────────────────────
void displayAnimShutdown() {
    if (!_display_initialized) return;
    
    // Kotak mengecil
    for(int i = 0; i < OLED_HEIGHT / 2; i+=2) {
        _display.clearDisplay();
        _display.drawRect(i*2, i, OLED_WIDTH - i*4, OLED_HEIGHT - i*2, SSD1306_WHITE);
        
        _display.setTextSize(2);
        _display.setTextColor(SSD1306_WHITE);
        _display.setCursor(20, 25);
        _display.print(F("OFF"));
        
        _display.display();
        delay(30);
    }
    
    _display.clearDisplay();
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
void displayUpdateMeter(float volt, float current, float power) {
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
    _display.setTextSize(2);
    
    // V
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
