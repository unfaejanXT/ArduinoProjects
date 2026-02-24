#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║                    config.h - Konfigurasi                    ║
 * ║                  PROYEK LCD MULTIMETER                       ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

// ─────────────────────────────────────────────────────────────────
//  PIN DEFINITIONS
// ─────────────────────────────────────────────────────────────────
#define PIN_VOLT       A2    // AC Voltage Sensor ZMPT1010B
#define PIN_CURRENT    A3    // AC Current Sensor ACS712
#define PIN_BTN        2     // Tombol Power (D2)
#define PIN_BUZZER     3     // Passive Buzzer (D3)
#define PIN_RELAY      4     // Relay Module (D4)
// OLED SDA -> A4, SCL -> A5

// ─────────────────────────────────────────────────────────────────
//  OLED SETTINGS
// ─────────────────────────────────────────────────────────────────
#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_ADDR      0x3C
#define OLED_RESET     -1

// ─────────────────────────────────────────────────────────────────
//  SENSOR AC CALIBRATION SETTINGS
// ─────────────────────────────────────────────────────────────────
// Parameter Kalibrasi Tegangan (ZMPT1010B)
#define VOLT_CALIBRATION    3.14    // Berdasarkan analisis log serial (70 * 3.14 = ~220V)
#define VOLT_OFFSET_RAW     512     // Nilai tengah (2.5V)

// Parameter Kalibrasi Arus (ACS712 5A/20A/30A)
#define CURRENT_CALIBRATION 0.185   // 0.185 V/A untuk 5A, 0.100 untuk 20A, 0.066 untuk 30A
#define CURRENT_OFFSET_RAW  512     // Nilai tengah (2.5V)

#define SAMPLES_PER_CYCLE   100     // Jumlah sample per pengukuran
#define SENSOR_READ_MS      500     // Ambil data setiap 500ms
#define SMOOTH_SAMPLES      5       // Moving average untuk menstabilkan bacaan (Verifikasi Sensor)

// ─────────────────────────────────────────────────────────────────
//  BUZZER TONES
// ─────────────────────────────────────────────────────────────────
#define FREQ_STARTUP_1  1000
#define FREQ_STARTUP_2  1500
#define FREQ_STARTUP_3  2000
#define FREQ_SHUTDOWN_1 1500
#define FREQ_SHUTDOWN_2 1000
#define FREQ_SHUTDOWN_3 500

// ─────────────────────────────────────────────────────────────────
//  BUTTON SETTINGS
// ─────────────────────────────────────────────────────────────────
#define DEBOUNCE_MS     50
