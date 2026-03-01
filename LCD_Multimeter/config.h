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
