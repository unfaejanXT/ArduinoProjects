#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║           display.h - Modul Tampilan OLED & Animasi          ║
 * ║              SONAR GUARDIAN - Smart Proximity System         ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  Modul ini mengelola semua tampilan OLED 128x64:
 *    - Screen splash / boot animasi
 *    - Tampilan utama: radar sweep + info jarak + bar + zona
 *    - Frame DANGER dengan animasi berkedip
 *    - Layout responsif berdasarkan zona aktif
 *    - Semua rendering menggunakan Adafruit GFX
 *
 *  LAYOUT LAYAR UTAMA (128x64):
 *  ┌────────────────────────────────┐
 *  │[RADAR SWEEP 52x52] │ [INFO]   │
 *  │     (kiri)         │ cm : 000 │ ← baris 1
 *  │                    │ ZONE:    │ ← baris 2
 *  │                    │ WARNING  │ ← baris 3
 *  │                    │ [BAR]    │ ← baris 4
 *  │                    │         │
 *  └────────────────────────────────┘
 *
 *  PARADIGMA: PROSEDURAL
 */

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"
#include "sonar.h"

// ─────────────────────────────────────────────────────────────────
//  INSTANCE OLED (global, hanya satu layar)
// ─────────────────────────────────────────────────────────────────
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// ─────────────────────────────────────────────────────────────────
//  STATE INTERNAL DISPLAY
// ─────────────────────────────────────────────────────────────────
static unsigned long _disp_lastUpdate  = 0;
static unsigned long _disp_lastSweep   = 0;
static float         _disp_sweepAngle  = -90.0f;  // Derajat sweep radar (mulai atas)
static bool          _disp_blinkState  = false;
static unsigned long _disp_lastBlink   = 0;
static bool          _disp_oledOk      = false;

// Parameter radar
#define RADAR_CX     26    // Pusat radar X
#define RADAR_CY     32    // Pusat radar Y
#define RADAR_R      24    // Radius radar

// Kolom panel kanan mulai dari
#define PANEL_X      56
#define PANEL_W      (OLED_WIDTH - PANEL_X)  // = 72

// ─────────────────────────────────────────────────────────────────
//  _dispDrawLine(angle, len, color)
//  Gambar garis dari pusat radar ke arah sudut tertentu
// ─────────────────────────────────────────────────────────────────
static void _dispDrawRadarLine(float angleDeg, int len, uint16_t color) {
    float rad = angleDeg * DEG_TO_RAD;
    int x1 = RADAR_CX + (int)(cos(rad) * len);
    int y1 = RADAR_CY + (int)(sin(rad) * len);
    oled.drawLine(RADAR_CX, RADAR_CY, x1, y1, color);
}

// ─────────────────────────────────────────────────────────────────
//  _dispDrawRadar(sweepAngle, distCm, zone)
//  Gambar tampilan radar dengan sweep animasi
// ─────────────────────────────────────────────────────────────────
static void _dispDrawRadar(float sweepAngle, float distCm, int zone) {
    // Lingkaran radar (3 cincin konsentris)
    oled.drawCircle(RADAR_CX, RADAR_CY, RADAR_R,     WHITE);
    oled.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, WHITE);
    oled.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, WHITE);

    // Garis silang (crosshair)
    oled.drawLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_CX + RADAR_R, RADAR_CY, WHITE);
    oled.drawLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_CX, RADAR_CY + RADAR_R, WHITE);

    // Sweep lines dengan efek fade (gambar beberapa garis di belakang sweep)
    for (int trail = 20; trail >= 0; trail -= 5) {
        float trailAngle = sweepAngle - trail;
        // Hanya gambar garis "solid" untuk sweep utama
        if (trail == 0) {
            _dispDrawRadarLine(trailAngle, RADAR_R, WHITE);
        } else {
            // Trail: gambar titik di ujung garis (efek ghost)
            float rad = trailAngle * DEG_TO_RAD;
            int len = RADAR_R - trail / 4;
            if (len > 2) {
                int px = RADAR_CX + (int)(cos(rad) * len);
                int py = RADAR_CY + (int)(sin(rad) * len);
                if (px >= 0 && px < PANEL_X && py >= 0 && py < OLED_HEIGHT) {
                    oled.drawPixel(px, py, WHITE);
                }
            }
        }
    }

    // Titik objek terdeteksi di radar (kalau dalam range)
    if (distCm > 0 && distCm <= DIST_DISPLAY_MAX) {
        // Proyeksikan jarak ke radius radar
        float ratio = constrain(distCm / (float)DIST_DISPLAY_MAX, 0.0, 1.0);
        int objR = (int)(ratio * RADAR_R);

        // Posisi objek di arah sweep saat ini (perkiraan)
        float rad = sweepAngle * DEG_TO_RAD;
        int ox = RADAR_CX + (int)(cos(rad) * objR);
        int oy = RADAR_CY + (int)(sin(rad) * objR);

        if (ox >= 0 && ox < PANEL_X && oy >= 0 && oy < OLED_HEIGHT) {
            if (zone == ZONE_DANGER) {
                // Titik besar berkedip saat DANGER
                oled.fillCircle(ox, oy, 2, WHITE);
            } else if (zone == ZONE_WARNING) {
                oled.fillCircle(ox, oy, 1, WHITE);
            } else {
                oled.drawPixel(ox, oy, WHITE);
            }
        }
    }

    // Center dot
    oled.fillCircle(RADAR_CX, RADAR_CY, 2, WHITE);
}

// ─────────────────────────────────────────────────────────────────
//  _dispDrawInfoPanel(distCm, zone)
//  Panel kanan: jarak, zona, dan bar chart
// ─────────────────────────────────────────────────────────────────
static void _dispDrawInfoPanel(float distCm, int zone) {
    // Garis pemisah vertikal
    oled.drawLine(PANEL_X - 2, 0, PANEL_X - 2, OLED_HEIGHT - 1, WHITE);

    int px = PANEL_X + 2;  // Margin kiri panel

    // ── Header: Label "SONAR" kecil ──────
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(px, 1);
    oled.print(F("SONAR"));

    // ── Jarak dalam CM (angka besar) ─────
    oled.setTextSize(1);
    oled.setCursor(px, 12);
    oled.print(F("DIST:"));

    // Angka jarak (sedikit lebih besar)
    oled.setTextSize(2);
    char distStr[8];
    if (distCm < 0 || distCm > DIST_MAX_CM) {
        strcpy(distStr, "---");
    } else if (distCm >= 1000) {
        strcpy(distStr, "999");
    } else {
        // Format: max 3 digit
        int d = (int)distCm;
        if (d < 10)        sprintf(distStr, " %d", d);
        else if (d < 100)  sprintf(distStr, "%d", d);
        else               sprintf(distStr, "%d", d);
        sprintf(distStr, "%d", d);
    }
    oled.setCursor(px, 21);
    oled.print(distStr);

    // Satuan "cm" kecil
    oled.setTextSize(1);
    oled.setCursor(px + 42, 28);
    oled.print(F("cm"));

    // ── Zona status ───────────────────────
    oled.setTextSize(1);
    oled.setCursor(px, 38);
    switch (zone) {
        case ZONE_DANGER:
            oled.print(F("! DANGER!"));
            break;
        case ZONE_WARNING:
            oled.print(F("WARNING "));
            break;
        default:
            oled.print(F("  SAFE  "));
            break;
    }

    // ── Bar Chart Jarak ───────────────────
    // Bar di bawah panel (lebar penuh panel, tinggi 6px)
    int barMaxW = PANEL_W - 6;  // lebar max bar
    int barX    = px;
    int barY    = 50;
    int barH    = 8;

    int distPct = sonarGetDistPercent();
    // Invert: makin dekat → bar makin penuh
    int barFill = barMaxW - (int)((distPct / 100.0f) * barMaxW);
    barFill = constrain(barFill, 0, barMaxW);

    // Border bar
    oled.drawRect(barX, barY, barMaxW, barH, WHITE);

    // Fill bar (sesuai warna zona)
    if (barFill > 0) {
        oled.fillRect(barX, barY, barFill, barH, WHITE);
    }
}

// ─────────────────────────────────────────────────────────────────
//  _dispDrawDangerOverlay() - Overlay berkedip saat DANGER
// ─────────────────────────────────────────────────────────────────
static void _dispDrawDangerOverlay() {
    // Border berkedip mengelilingi layar
    oled.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, WHITE);
    oled.drawRect(1, 1, OLED_WIDTH - 2, OLED_HEIGHT - 2, WHITE);
}

// ─────────────────────────────────────────────────────────────────
//  displaySetup() - Inisialisasi OLED
//  Return: true jika berhasil
// ─────────────────────────────────────────────────────────────────
bool displaySetup() {
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        _disp_oledOk = false;
        return false;
    }
    _disp_oledOk = true;
    oled.clearDisplay();
    oled.display();
    return true;
}

// ─────────────────────────────────────────────────────────────────
//  displayShowSplash() - Layar boot animatif
//  Blocking, dipanggil sekali saat setup
// ─────────────────────────────────────────────────────────────────
void displayShowSplash() {
    if (!_disp_oledOk) return;

    // ── Frame 1: Logo radar + sweep cepat (~700ms) ──
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.drawCircle(64, 32, 30, WHITE);
    oled.drawCircle(64, 32, 20, WHITE);
    oled.drawCircle(64, 32, 10, WHITE);
    oled.drawLine(64, 2, 64, 62, WHITE);
    oled.drawLine(34, 32, 94, 32, WHITE);
    oled.fillCircle(64, 32, 3, WHITE);
    oled.display();
    delay(250);                             // 500ms → 250ms

    // Sweep singkat: step 30° (lebih cepat dari 15°)
    for (int a = -90; a <= 270; a += 30) {
        oled.clearDisplay();
        oled.drawCircle(64, 32, 30, WHITE);
        oled.drawCircle(64, 32, 20, WHITE);
        oled.drawCircle(64, 32, 10, WHITE);
        float rad = a * DEG_TO_RAD;
        oled.drawLine(64, 32,
                      64 + (int)(cos(rad) * 30),
                      32 + (int)(sin(rad) * 30), WHITE);
        oled.fillCircle(64, 32, 3, WHITE);
        oled.display();
        delay(15);                          // 20ms → 15ms
    }

    // ── Frame 2: Titel (~500ms) ──
    oled.clearDisplay();
    oled.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, WHITE);
    oled.setTextSize(1);
    oled.setCursor(12, 10);
    oled.print(F("SONAR GUARDIAN"));
    oled.setCursor(20, 24);
    oled.print(F("Smart Proximity"));
    oled.setCursor(30, 34);
    oled.print(F("Alert System"));
    oled.setCursor(38, 50);
    oled.print(F("v1.0.0"));
    oled.display();
    delay(600);                             // 1800ms → 600ms

    // ── Frame 3: Progress bar (~400ms) ──
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(10, 16);
    oled.print(F("Initializing..."));
    // 6 langkah (dari 13), setiap step 45ms → total ~270ms
    for (int p = 0; p <= 108; p += 18) {
        oled.fillRect(10, 30, p, 8, WHITE);
        oled.drawRect(10, 30, 108, 8, WHITE);
        oled.display();
        delay(45);                          // 60ms → 45ms, step 18 (dari 9)
    }
    oled.setCursor(22, 48);
    oled.print(F("SYSTEM READY!"));
    oled.display();
    delay(300);                             // 600ms → 300ms
}

// ─────────────────────────────────────────────────────────────────
//  displayLoop() - Update tampilan utama (non-blocking)
//  Panggil di loop() setiap iterasi
// ─────────────────────────────────────────────────────────────────
void displayLoop(float distCm, int zone) {
    if (!_disp_oledOk) return;

    unsigned long now = millis();

    // Update sweep angle (terus berputar)
    if (now - _disp_lastSweep >= RADAR_SWEEP_MS) {
        _disp_sweepAngle += 6.0f;  // 6 derajat per frame
        if (_disp_sweepAngle >= 270.0f) _disp_sweepAngle = -90.0f;
        _disp_lastSweep = now;
    }

    // Update blink state (untuk animasi DANGER)
    if (now - _disp_lastBlink >= 200) {
        _disp_blinkState  = !_disp_blinkState;
        _disp_lastBlink   = now;
    }

    // Update display pada interval yang ditentukan
    if (now - _disp_lastUpdate < DISPLAY_UPDATE_MS) return;
    _disp_lastUpdate = now;

    oled.clearDisplay();

    // Gambar panel radar (sisi kiri)
    _dispDrawRadar(_disp_sweepAngle, distCm, zone);

    // Gambar panel info (sisi kanan)
    _dispDrawInfoPanel(distCm, zone);

    // Overlay DANGER berkedip
    if (zone == ZONE_DANGER && _disp_blinkState) {
        _dispDrawDangerOverlay();
    }

    oled.display();
}

// ─────────────────────────────────────────────────────────────────
//  displayShowError(msg) - Tampilkan pesan error
// ─────────────────────────────────────────────────────────────────
void displayShowError(const char* msg) {
    if (!_disp_oledOk) return;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.print(F("! ERROR !"));
    oled.setCursor(0, 16);
    oled.print(msg);
    oled.display();
}

// ─────────────────────────────────────────────────────────────────
//  displayIsOk() - Cek apakah OLED berhasil diinisialisasi
// ─────────────────────────────────────────────────────────────────
bool displayIsOk() {
    return _disp_oledOk;
}
