/**
 * ============================================================
 *  FILE: LCD_Jam_Digital.ino
 *  Deskripsi: Entry point utama - Jam Digital NTP via WiFi
 *             Tampilan: HH:MM:SS dan DD-MM-YYYY di OLED SSD1306
 *
 *  Hardware  : NodeMCU v3 (ESP8266) + OLED SSD1306 128x64 (I2C)
 *  Author    : OOP Multi-File Project
 *  Versi     : 2.0.0
 * ============================================================
 *
 *  LIBRARY YANG HARUS DI-INSTALL (via Library Manager):
 *  ┌────────────────────────────────────────────────────┐
 *  │ 1. NTPClient by Fabrice Weinberg                   │
 *  │ 2. Adafruit SSD1306                                │
 *  │ 3. Adafruit GFX Library                            │
 *  └────────────────────────────────────────────────────┘
 *
 *  BOARD MANAGER:
 *  ┌────────────────────────────────────────────────────┐
 *  │ Board   : NodeMCU 1.0 (ESP-12E Module)             │
 *  │ Package : esp8266 by ESP8266 Community             │
 *  └────────────────────────────────────────────────────┘
 *
 *  KONFIGURASI:
 *  → Edit file config.h untuk mengisi WiFi & pengaturan lain
 * ============================================================
 */

// Pola include identik dengan LCD_Animation.ino
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "src/ClockApp/ClockApp.h"

// ─── Instansiasi Aplikasi ─────────────────────────────────────
ClockApp app;

// ─────────────────────────────────────────────────────────────
//  setup() - Dipanggil sekali saat NodeMCU menyala
// ─────────────────────────────────────────────────────────────
void setup() {
    app.begin();
}

// ─────────────────────────────────────────────────────────────
//  loop() - Dipanggil terus-menerus setelah setup()
// ─────────────────────────────────────────────────────────────
void loop() {
    app.run();
}
