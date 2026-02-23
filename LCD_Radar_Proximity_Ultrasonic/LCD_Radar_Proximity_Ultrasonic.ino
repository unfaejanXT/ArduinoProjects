/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║           SONAR GUARDIAN - Smart Proximity System            ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  FILE    : DoYourMagic_Arduino.ino                           ║
 * ║  Deskripsi: Sistem radar sonar cerdas dengan tampilan OLED   ║
 * ║             animasi dan alarm buzzer adaptif berbasis jarak  ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  HARDWARE:                                                   ║
 * ║    • Arduino Uno                                             ║
 * ║    • OLED Display 128x64 → SCL(A5), SDA(A4)                 ║
 * ║    • Ultrasonic Sensor HC-SR04 → TRIG(9), ECHO(10)          ║
 * ║    • Passive Buzzer → Pin 3                                  ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  LIBRARY (Install via Library Manager):                      ║
 * ║    ┌─────────────────────────────────────────────────────┐   ║
 * ║    │ 1. Adafruit SSD1306                                  │   ║
 * ║    │ 2. Adafruit GFX Library                              │   ║
 * ║    └─────────────────────────────────────────────────────┘   ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  PARADIGMA: PROSEDURAL                                       ║
 * ║    - Seluruh logika terbagi ke modul .h di folder src/       ║
 * ║    - Entry point hanya memanggil appSetup() dan appLoop()    ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  STRUKTUR FILE:                                              ║
 * ║    DoYourMagic_Arduino.ino  ← entry point (file ini)        ║
 * ║    config.h                 ← pin & parameter konfigurasi   ║
 * ║    src/                                                      ║
 * ║      buzzer.h     ← modul buzzer & melodi                   ║
 * ║      sonar.h      ← modul sensor ultrasonik                 ║
 * ║      display.h    ← modul tampilan OLED & animasi           ║
 * ║      guardian.h   ← orchestrator state machine              ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Modul utama (menarik semua modul src/ secara transitif)
#include "src/guardian.h"

// ─────────────────────────────────────────────────────────────────
//  setup() - Dipanggil sekali saat Arduino menyala
// ─────────────────────────────────────────────────────────────────
void setup() {
    appSetup();
}

// ─────────────────────────────────────────────────────────────────
//  loop() - Berjalan terus-menerus setelah setup()
// ─────────────────────────────────────────────────────────────────
void loop() {
    appLoop();
}
