/**
 * ============================================================
 *  FILE: src/lcd_display.h
 *  Deskripsi: Modul prosedural untuk menampilkan konten di
 *             OLED SSD1306 128x64 via Adafruit SSD1306 Driver
 *
 *  PARADIGMA: Prosedural
 *    - Tidak ada class / constructor / metode
 *    - Objek Adafruit_SSD1306 disimpan sebagai variabel global file-local
 *    - Seluruh fungsi bersifat bebas (free functions)
 *
 *  Project  : LCD Jam Digital PROSEDURAL - NodeMCU v3
 * ============================================================
 *
 *  Library yang dibutuhkan (install via Library Manager):
 *  - Adafruit SSD1306
 *  - Adafruit GFX Library
 *
 *  Koneksi OLED SSD1306 ke NodeMCU v3:
 *  ┌──────────┬───────────────┐
 *  │ OLED Pin │ NodeMCU v3    │
 *  ├──────────┼───────────────┤
 *  │  VCC     │  3.3V         │
 *  │  GND     │  GND          │
 *  │  SDA     │  D2 (GPIO4)   │
 *  │  SCL     │  D1 (GPIO5)   │
 *  └──────────┴───────────────┘
 *
 *  CARA PAKAI:
 *    #include "src/lcd_display.h"
 *    lcdBegin();                     // inisialisasi OLED
 *    lcdShowClock(timeStr, dateStr); // tampilkan jam & tanggal
 *    lcdShowStatus(line1, line2);    // tampilkan status
 *    lcdShowConnecting(step);        // animasi connecting
 *    lcdShowError(msg);              // tampilkan error
 *    lcdClear();                     // bersihkan layar
 * ============================================================
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────
//  State internal (hanya terlihat di file ini)
// ─────────────────────────────────────────────────────────────
static Adafruit_SSD1306 _oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT,
                               &Wire, OLED_RESET_PIN);

// ─────────────────────────────────────────────────────────────
//  Deklarasi forward (fungsi privat internal)
// ─────────────────────────────────────────────────────────────
static int16_t _lcdCenterX(const String& text, uint8_t textSize);
static void    _lcdDrawText(const String& text, int16_t x, int16_t y, uint8_t size);
static void    _lcdDrawCentered(const String& text, int16_t y, uint8_t size);

// ─────────────────────────────────────────────────────────────
//  lcdBegin() - Inisialisasi OLED SSD1306
//  @return true jika berhasil, false jika OLED tidak terdeteksi
// ─────────────────────────────────────────────────────────────
bool lcdBegin() {
    // Pola inisialisasi identik dengan LCD_Animation
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("[ERROR] OLED SSD1306 tidak ditemukan! Cek koneksi I2C."));
        return false;
    }

    // Fast Mode I2C (400 kHz)
    Wire.setClock(400000UL);

    _oled.clearDisplay();
    _oled.display();

    Serial.println(F("[OLED] Inisialisasi berhasil!"));
    Serial.print(F("[OLED] Ukuran: "));
    Serial.print(OLED_SCREEN_WIDTH);
    Serial.print(F("x"));
    Serial.println(OLED_SCREEN_HEIGHT);

    // ── Splash Screen ─────────────────────────────────────────
    // Layout 128x64:
    //  Y=0  ┌──────────────────────────────────┐  border
    //  Y=8  │         JAM DIGITAL              │  textSize(1)
    //  Y=18 │ ─────────────────────────────── │  separator line
    //  Y=26 │        NTP via WiFi              │  textSize(1)
    //  Y=38 │         NodeMCU v3              │  textSize(1)
    //  Y=63 └──────────────────────────────────┘  border
    _oled.setTextColor(SSD1306_WHITE);

    // Border luar
    _oled.drawRect(0, 0, OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, SSD1306_WHITE);

    // Baris 1: Judul
    _lcdDrawCentered("JAM  DIGITAL", 8, 1);

    // Separator line tipis
    _oled.drawFastHLine(8, 19, OLED_SCREEN_WIDTH - 16, SSD1306_WHITE);

    // Baris 2: Info sync
    _lcdDrawCentered("NTP via WiFi", 26, 1);

    // Baris 3: Platform
    _lcdDrawCentered("NodeMCU  v3", 40, 1);

    _oled.display();
    delay(1800);

    _oled.clearDisplay();
    _oled.display();

    return true;
}

// ─────────────────────────────────────────────────────────────
//  lcdShowClock() - Tampilan utama jam & tanggal
//
//  Kalkulasi layout OLED 128x64 pixel:
//
//  textSize(2): char = 12px lebar × 16px tinggi
//  "HH:MM:SS"   = 8  char → 96px  lebar ✓ (< 128px)
//  "DD-MM-YYYY" = 10 char → 120px lebar ✓ (< 128px)
//
//  Y=0  ┌──────────────────────────────────┐
//  Y=12 │        12:34:56                  │ ← size(2), tinggi 16px
//  Y=28 │                                  │ ← gap ~14px
//  Y=42 │       22-02-2026                 │ ← size(2), tinggi 16px
//  Y=58 │                                  │
//  Y=64 └──────────────────────────────────┘
//
//  @param timeStr : String waktu "HH:MM:SS"
//  @param dateStr : String tanggal "DD-MM-YYYY"
// ─────────────────────────────────────────────────────────────
void lcdShowClock(const String& timeStr, const String& dateStr) {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    // ── Baris 1: Jam HH:MM:SS ─────────────────────────────────
    // textSize(2) → 8 karakter × 12px = 96px lebar, 16px tinggi
    _lcdDrawCentered(timeStr, 12, 2);

    // ── Baris 2: Tanggal DD-MM-YYYY ───────────────────────────
    // textSize(2) → 10 karakter × 12px = 120px lebar, 16px tinggi
    _lcdDrawCentered(dateStr, 42, 2);

    _oled.display();
}

// ─────────────────────────────────────────────────────────────
//  lcdShowStatus() - Pesan status 2 baris teks
//  @param line1 : Teks baris pertama
//  @param line2 : Teks baris kedua (opsional, default "")
// ─────────────────────────────────────────────────────────────
void lcdShowStatus(const String& line1, const String& line2 = "") {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    _lcdDrawCentered(line1, 16, 1);

    if (line2.length() > 0) {
        _lcdDrawCentered(line2, 36, 1);
    }

    _oled.display();
}

// ─────────────────────────────────────────────────────────────
//  lcdShowConnecting() - Animasi dots saat koneksi WiFi
//
//  ┌──────────────────────────────────┐
//  │     Connecting WiFi              │
//  │     NTP Sync...                  │
//  │          . . .                   │  ← Animasi titik
//  └──────────────────────────────────┘
//
//  @param step : Langkah animasi (auto-increment dari luar)
// ─────────────────────────────────────────────────────────────
void lcdShowConnecting(uint8_t step) {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    _lcdDrawCentered("Connecting WiFi", 10, 1);
    _lcdDrawCentered("NTP Sync...", 22, 1);

    // Animasi titik: . .. ... ....
    String dots = "";
    uint8_t numDots = (step % 4) + 1;
    for (uint8_t i = 0; i < numDots; i++) {
        dots += ". ";
    }
    _lcdDrawCentered(dots, 44, 1);

    _oled.display();
}

// ─────────────────────────────────────────────────────────────
//  lcdShowError() - Tampilkan pesan error
//  @param errorMsg : Pesan error
// ─────────────────────────────────────────────────────────────
void lcdShowError(const String& errorMsg) {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    _lcdDrawCentered("!! ERROR !!", 10, 1);
    _lcdDrawCentered(errorMsg, 36, 1);

    _oled.display();
}

// ─────────────────────────────────────────────────────────────
//  lcdClear() - Bersihkan layar OLED
// ─────────────────────────────────────────────────────────────
void lcdClear() {
    _oled.clearDisplay();
    _oled.display();
}

// =============================================================
//  FUNGSI PRIVAT INTERNAL
// =============================================================

// ─────────────────────────────────────────────────────────────
//  _lcdCenterX() - Hitung posisi X untuk teks rata tengah
//
//  Rumus Adafruit GFX default font:
//    lebar per karakter = 6 * textSize  (5px glyph + 1px spacing)
//  Tidak perlu getTextBounds() (non-const) → kalkulasi manual.
// ─────────────────────────────────────────────────────────────
static int16_t _lcdCenterX(const String& text, uint8_t textSize) {
    int16_t textWidth = (int16_t)(text.length()) * 6 * (int16_t)textSize;
    int16_t posX      = ((int16_t)OLED_SCREEN_WIDTH - textWidth) / 2;
    return posX < 0 ? 0 : posX;
}

// ─────────────────────────────────────────────────────────────
//  _lcdDrawText() - Gambar teks di posisi (x, y)
// ─────────────────────────────────────────────────────────────
static void _lcdDrawText(const String& text, int16_t x, int16_t y, uint8_t size) {
    _oled.setTextSize(size);
    _oled.setCursor(x, y);
    _oled.print(text);
}

// ─────────────────────────────────────────────────────────────
//  _lcdDrawCentered() - Gambar teks rata tengah pada baris Y
// ─────────────────────────────────────────────────────────────
static void _lcdDrawCentered(const String& text, int16_t y, uint8_t size) {
    int16_t x = _lcdCenterX(text, size);
    _lcdDrawText(text, x, y, size);
}

#endif // LCD_DISPLAY_H
