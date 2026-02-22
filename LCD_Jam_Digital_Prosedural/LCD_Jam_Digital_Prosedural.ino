/**
 * ============================================================
 *  FILE: LCD_Jam_Digital_Prosedural.ino
 *  Deskripsi: Entry point utama - Jam Digital NTP via WiFi
 *             Tampilan: HH:MM:SS dan DD-MM-YYYY di OLED SSD1306
 *
 *  PARADIGMA: PROSEDURAL
 *    - Tidak ada instansiasi class di sini
 *    - Hanya memanggil appSetup() dan appLoop()
 *    - Seluruh logika dibagi ke modul-modul .h di folder src/
 *
 *  Hardware  : NodeMCU v3 (ESP8266) + OLED SSD1306 128x64 (I2C)
 *  Author    : Prosedural Single-File-per-Module Project
 *  Versi     : 1.0.0
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
 *
 *  STRUKTUR FILE:
 *  ┌──────────────────────────────────────────────────────┐
 *  │ LCD_Jam_Digital_Prosedural.ino  ← entry point       │
 *  │ config.h                        ← konfigurasi       │
 *  │ src/                                                 │
 *  │   wifi_manager.h  ← modul koneksi WiFi (prosedural) │
 *  │   ntp_manager.h   ← modul sinkronisasi NTP          │
 *  │   lcd_display.h   ← modul tampilan OLED             │
 *  │   clock_app.h     ← orchestrator state machine      │
 *  └──────────────────────────────────────────────────────┘
 *
 *  PERBANDINGAN DENGAN VERSI OOP (LCD_Jam_Digital):
 *  ┌───────────────────────┬────────────────────────────┐
 *  │ OOP                   │ Prosedural                 │
 *  ├───────────────────────┼────────────────────────────┤
 *  │ class ClockApp        │ appSetup() / appLoop()     │
 *  │ class WiFiManager     │ wifiConnect() dll          │
 *  │ class NTPManager      │ ntpForceSync() dll         │
 *  │ class LCDDisplay      │ lcdShowClock() dll         │
 *  │ .h + .cpp per modul   │ .h saja per modul          │
 *  │ objek (instansiasi)   │ var global + fungsi bebas  │
 *  └───────────────────────┴────────────────────────────┘
 * ============================================================
 */

// Pola include hardware identik dengan versi OOP
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Include modul orchestrator (secara transitif menarik
// wifi_manager.h, ntp_manager.h, lcd_display.h)
#include "src/clock_app.h"

// ─────────────────────────────────────────────────────────────
//  setup() - Dipanggil sekali saat NodeMCU menyala
// ─────────────────────────────────────────────────────────────
void setup() {
    appSetup();
}

// ─────────────────────────────────────────────────────────────
//  loop() - Dipanggil terus-menerus setelah setup()
// ─────────────────────────────────────────────────────────────
void loop() {
    appLoop();
}
