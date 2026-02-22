/**
 * ============================================================
 *  FILE: src/LCDDisplay/LCDDisplay.h
 *  Deskripsi: Class untuk mengelola tampilan OLED SSD1306
 *             Driver: Adafruit SSD1306 + Adafruit GFX
 *             (sama seperti project LCD_Animation)
 *  Project  : LCD Jam Digital - NodeMCU v3
 * ============================================================
 *
 *  Library yang dibutuhkan (install via Library Manager):
 *  - Adafruit SSD1306
 *  - Adafruit GFX Library
 * ============================================================
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
 * ============================================================
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../../config.h"

class LCDDisplay {
public:
    /**
     * Constructor
     * @param width     : Lebar OLED dalam pixel (128)
     * @param height    : Tinggi OLED dalam pixel (64)
     * @param address   : Alamat I2C (0x3C)
     * @param resetPin  : Pin reset (-1 jika tidak ada)
     */
    LCDDisplay(uint8_t width, uint8_t height, uint8_t address, int8_t resetPin);

    /**
     * Inisialisasi OLED
     * @return true jika berhasil, false jika OLED tidak terdeteksi
     */
    bool begin();

    /**
     * Menampilkan jam besar (HH:MM:SS) dan tanggal (DD-MM-YYYY)
     *
     *  Layout OLED 128x64:
     *  ┌──────────────────────────────────┐
     *  │                                  │  ← 8px padding atas
     *  │         HH : MM : SS             │  ← Font besar (size 3)
     *  │                                  │
     *  │         DD-MM-YYYY               │  ← Font kecil (size 2)
     *  │                                  │
     *  └──────────────────────────────────┘
     *
     * @param timeStr : String waktu "HH:MM:SS"
     * @param dateStr : String tanggal "DD-MM-YYYY"
     */
    void showClock(const String& timeStr, const String& dateStr);

    /**
     * Menampilkan pesan status (saat booting/koneksi WiFi)
     * @param line1 : Teks baris pertama
     * @param line2 : Teks baris kedua (opsional)
     */
    void showStatus(const String& line1, const String& line2 = "");

    /**
     * Menampilkan animasi loading/connecting (dots bergerak)
     * @param step : Langkah animasi (auto-increment dari luar)
     */
    void showConnecting(uint8_t step);

    /**
     * Menampilkan pesan error
     * @param errorMsg : Pesan error
     */
    void showError(const String& errorMsg);

    /**
     * Membersihkan layar OLED
     */
    void clear();

private:
    Adafruit_SSD1306 _display;
    uint8_t          _width;
    uint8_t          _height;
    uint8_t          _address;

    // Menghitung posisi X agar teks rata tengah secara pixel
    int16_t _centerX(const String& text, uint8_t textSize) const;

    // Menggambar teks di posisi tertentu
    void _drawText(const String& text, int16_t x, int16_t y, uint8_t size);

    // Menggambar teks rata tengah pada baris Y tertentu
    void _drawCentered(const String& text, int16_t y, uint8_t size);
};

#endif // LCD_DISPLAY_H
