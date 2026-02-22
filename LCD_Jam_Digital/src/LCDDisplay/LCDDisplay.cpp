/**
 * ============================================================
 *  FILE: src/LCDDisplay/LCDDisplay.cpp
 *  Deskripsi: Implementasi class LCDDisplay menggunakan OLED SSD1306
 *             Driver: Adafruit SSD1306 + Adafruit GFX
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 */

#include "LCDDisplay.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
LCDDisplay::LCDDisplay(uint8_t width, uint8_t height, uint8_t address, int8_t resetPin)
    : _display(width, height, &Wire, resetPin),
      _width(width),
      _height(height),
      _address(address)
{
}

// ─────────────────────────────────────────────────────────────
//  begin() - Inisialisasi OLED
// ─────────────────────────────────────────────────────────────
bool LCDDisplay::begin() {
    // Pola inisialisasi identik dengan LCD_Animation
    if (!_display.begin(SSD1306_SWITCHCAPVCC, _address)) {
        Serial.println(F("[ERROR] OLED SSD1306 tidak ditemukan! Cek koneksi I2C."));
        return false;
    }

    // Fast Mode I2C (400 kHz) - sama seperti LCD_Animation
    Wire.setClock(400000UL);

    _display.clearDisplay();
    _display.display();

    Serial.println(F("[OLED] Inisialisasi berhasil!"));
    Serial.print(F("[OLED] Ukuran: "));
    Serial.print(_width);
    Serial.print(F("x"));
    Serial.println(_height);

    // ── Splash Screen ─────────────────────────────────────────
    // Layout 128x64:
    //  Y=0  ┌──────────────────────────────────┐  border
    //  Y=8  │         JAM DIGITAL              │  textSize(1)
    //  Y=18 │ ─────────────────────────────── │  separator line
    //  Y=26 │        NTP via WiFi              │  textSize(1)
    //  Y=38 │         NodeMCU v3              │  textSize(1)
    //  Y=63 └──────────────────────────────────┘  border
    _display.setTextColor(SSD1306_WHITE);

    // Border luar
    _display.drawRect(0, 0, _width, _height, SSD1306_WHITE);

    // Baris 1: Judul
    _drawCentered("JAM  DIGITAL", 8, 1);

    // Separator line tipis
    _display.drawFastHLine(8, 19, _width - 16, SSD1306_WHITE);

    // Baris 2: Info sync
    _drawCentered("NTP via WiFi", 26, 1);

    // Baris 3: Platform
    _drawCentered("NodeMCU  v3", 40, 1);

    _display.display();
    delay(1800);


    _display.clearDisplay();
    _display.display();

    return true;
}

// ─────────────────────────────────────────────────────────────
//  showClock() - Tampilan utama jam & tanggal
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
// ─────────────────────────────────────────────────────────────
void LCDDisplay::showClock(const String& timeStr, const String& dateStr) {
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    // ── Baris 1: Jam HH:MM:SS ─────────────────────────────────
    // textSize(2) → 8 karakter × 12px = 96px lebar, 16px tinggi → pas
    _drawCentered(timeStr, 12, 2);

    // ── Baris 2: Tanggal DD-MM-YYYY ───────────────────────────
    // textSize(2) → 10 karakter × 12px = 120px lebar, 16px tinggi → pas
    _drawCentered(dateStr, 42, 2);

    _display.display();
}

// ─────────────────────────────────────────────────────────────
//  showStatus() - Pesan status 2 baris teks
// ─────────────────────────────────────────────────────────────
void LCDDisplay::showStatus(const String& line1, const String& line2) {
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    _drawCentered(line1, 16, 1);

    if (line2.length() > 0) {
        _drawCentered(line2, 36, 1);
    }

    _display.display();
}

// ─────────────────────────────────────────────────────────────
//  showConnecting() - Animasi dots saat koneksi WiFi
//
//  ┌──────────────────────────────────┐
//  │     Connecting WiFi              │
//  │          . . .                   │  ← Animasi titik
//  └──────────────────────────────────┘
// ─────────────────────────────────────────────────────────────
void LCDDisplay::showConnecting(uint8_t step) {
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    _drawCentered("Connecting WiFi", 10, 1);
    _drawCentered("NTP Sync...", 22, 1);

    // Animasi titik: . .. ... ....
    String dots = "";
    uint8_t numDots = (step % 4) + 1;
    for (uint8_t i = 0; i < numDots; i++) {
        dots += ". ";
    }
    _drawCentered(dots, 44, 1);

    _display.display();
}

// ─────────────────────────────────────────────────────────────
//  showError() - Tampil pesan error
// ─────────────────────────────────────────────────────────────
void LCDDisplay::showError(const String& errorMsg) {
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    _drawCentered("!! ERROR !!", 10, 1);
    _drawCentered(errorMsg, 36, 1);

    _display.display();
}

// ─────────────────────────────────────────────────────────────
//  clear() - Bersihkan layar
// ─────────────────────────────────────────────────────────────
void LCDDisplay::clear() {
    _display.clearDisplay();
    _display.display();
}

// ─────────────────────────────────────────────────────────────
//  _centerX() - Hitung posisi X untuk teks rata tengah
//
//  Rumus Adafruit GFX default font:
//    lebar per karakter = 6 * textSize  (5px glyph + 1px spacing)
//  Tidak perlu getTextBounds() (non-const) → kalkulasi manual.
// ─────────────────────────────────────────────────────────────
int16_t LCDDisplay::_centerX(const String& text, uint8_t textSize) const {
    // Adafruit GFX default font: setiap karakter tepat 6 * textSize pixel lebar
    int16_t textWidth = (int16_t)(text.length()) * 6 * (int16_t)textSize;
    int16_t posX      = ((int16_t)_width - textWidth) / 2;
    return posX < 0 ? 0 : posX;
}

// ─────────────────────────────────────────────────────────────
//  _drawText() - Gambar teks di posisi (x, y)
// ─────────────────────────────────────────────────────────────
void LCDDisplay::_drawText(const String& text, int16_t x, int16_t y, uint8_t size) {
    _display.setTextSize(size);
    _display.setCursor(x, y);
    _display.print(text);
}

// ─────────────────────────────────────────────────────────────
//  _drawCentered() - Gambar teks rata tengah pada baris Y
// ─────────────────────────────────────────────────────────────
void LCDDisplay::_drawCentered(const String& text, int16_t y, uint8_t size) {
    int16_t x = _centerX(text, size);
    _drawText(text, x, y, size);
}
