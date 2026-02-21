// ============================================================
// LCD_Animation.ino  —  Entry point utama
//
// Project   : NodeMCU V3 + OLED SSD1306 (128x64, I2C)
// Fungsi    : Memutar 3 animasi 3D bergantian setiap 5 detik
//
// Struktur folder:
//   LCD_Animation.ino           ← File ini (setup + loop saja)
//   src/
//     config.h                  ← Konstanta global
//     sin_lut.h / .cpp          ← LUT Sinus shared (PROGMEM)
//     FPSCounter.h              ← Class FPS overlay
//     AnimationManager.h        ← Orkestrator rotasi animasi
//     animations/
//       Anim3DCube.h            ← Kubus 3D berputar (float)
//       AnimBouncingCube.h      ← Kubus pantul DVD-logo style (LUT)
//       AnimPipeCrawler.h       ← Pipa 3D acak (screensaver style)
//
// Library yang dibutuhkan (install via Library Manager):
//   - Adafruit SSD1306
//   - Adafruit GFX Library
// ============================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "src/config.h"
#include "src/AnimationManager.h"

// --- Objek display global ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Manager yang mengurus semua animasi ---
AnimationManager animManager(&display);

// ============================================================
void setup() {
  Serial.begin(115200);

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[ERROR] OLED tidak ditemukan! Cek koneksi I2C."));
    while (true); // Berhenti jika display tidak ada
  }

  Wire.setClock(400000UL); // Fast Mode I2C (400 kHz)

  display.clearDisplay();
  display.display();

  // Mulai manager animasi
  animManager.begin();

  Serial.println(F("[OK] LCD Animation siap!"));
}

// ============================================================
void loop() {
  // Semua logika ada di dalam AnimationManager
  animManager.tick();
}
