#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║                    config.h - Konfigurasi                    ║
 * ║              SONAR GUARDIAN - Smart Proximity System         ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  Ubah nilai di sini untuk menyesuaikan perilaku sistem.
 *  Tidak perlu menyentuh file lain untuk konfigurasi dasar.
 */

// ─────────────────────────────────────────────────────────────────
//  PIN DEFINITIONS
// ─────────────────────────────────────────────────────────────────
#define PIN_BUZZER        3     // Passive Buzzer  → Digital Pin 3 (PWM)
#define PIN_TRIG          9     // HC-SR04 TRIG    → Digital Pin 9
#define PIN_ECHO          10    // HC-SR04 ECHO    → Digital Pin 10
// OLED  → SDA (A4), SCL (A5) → Wire.h menangani otomatis

// ─────────────────────────────────────────────────────────────────
//  OLED DISPLAY SETTINGS
// ─────────────────────────────────────────────────────────────────
#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_I2C_ADDR     0x3C  // Alamat I2C OLED (biasanya 0x3C atau 0x3D)
#define OLED_RESET        -1    // Reset pin (-1 = tidak pakai pin reset)

// ─────────────────────────────────────────────────────────────────
//  SENSOR ULTRASONIK - PARAMETER JARAK
// ─────────────────────────────────────────────────────────────────
#define DIST_MAX_CM       400   // Jarak maksimum sensor HC-SR04 (cm)
#define DIST_DISPLAY_MAX  200   // Jarak maksimum yang ditampilkan di bar (cm)

// Zona jarak (dalam cm)
#define ZONE_DANGER_CM    20    // Di bawah ini → DANGER  🔴
#define ZONE_WARNING_CM   60    // Di bawah ini → WARNING 🟡
                                // Di atas ini  → SAFE    🟢

// Timeout pembacaan sensor (mikrosecond)
#define SONAR_TIMEOUT_US  30000UL

// ─────────────────────────────────────────────────────────────────
//  BUZZER - PENGATURAN ALARM
// ─────────────────────────────────────────────────────────────────
// Frekuensi nada (Hz)
#define BEEP_FREQ_DANGER  1200  // Nada tinggi saat DANGER
#define BEEP_FREQ_WARNING 800   // Nada sedang saat WARNING
#define BEEP_FREQ_SAFE    400   // Nada rendah saat SAFE (silent after 1 beep)

// Durasi beep (ms)
#define BEEP_DUR_DANGER   60    // Beep sangat pendek & cepat
#define BEEP_DUR_WARNING  120   // Beep sedang
#define BEEP_DUR_SAFE     0     // Tidak bunyi saat SAFE

// Interval antar beep berdasarkan jarak (ms)
#define BEEP_INTERVAL_MIN 80    // Interval minimum saat paling dekat
#define BEEP_INTERVAL_MAX 1500  // Interval saat di zona warning

// ─────────────────────────────────────────────────────────────────
//  TIMING - REFRESH RATE
// ─────────────────────────────────────────────────────────────────
#define SENSOR_READ_MS    50    // Baca sensor setiap 50ms
#define DISPLAY_UPDATE_MS 60    // Update display setiap 60ms
#define RADAR_SWEEP_MS    30    // Animasi radar sweep setiap 30ms

// ─────────────────────────────────────────────────────────────────
//  MELODI STARTUP
// ─────────────────────────────────────────────────────────────────
// Aktifkan/nonaktifkan melodi saat boot
#define STARTUP_MELODY_ENABLED  true

// ─────────────────────────────────────────────────────────────────
//  SMOOTHING - RATA-RATA JARAK
// ─────────────────────────────────────────────────────────────────
#define SMOOTH_SAMPLES    5     // Jumlah sample untuk moving average
