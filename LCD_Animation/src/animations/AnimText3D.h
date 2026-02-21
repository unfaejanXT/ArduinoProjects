#pragma once
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>
#include "../config.h"
#include "../sin_lut.h"

// ============================================================
// AnimText3D.h
// Teks "JANWAR" berputar 3D pada sumbu Y menggunakan stroke font.
//
// Cara kerja:
//  - Setiap huruf didefinisikan sebagai kumpulan garis (stroke)
//    dalam grid lokal: lebar 5 unit, tinggi 8 unit.
//  - Semua stroke di-scale lalu dirotasi pada sumbu Y dengan SIN_LUT.
//  - Proyeksi ortografis: x_screen = x_rot + 64, y_screen = -y + 32
//
// Untuk ganti teks → ubah method draw() dan sesuaikan offsets[].
// ============================================================

class AnimText3D {
  private:
    Adafruit_SSD1306* _display;
    uint8_t           _angle;   // 0-255 = 0-360°, rotasi sumbu Y

    static const int8_t SCALE    = 3;   // Local unit → 3D unit (1 lokal = 3 pixel)
    static const int8_t Y_HALF   = 12;  // Tinggi total / 2 = (8 * 3) / 2 = 12

    // ----------------------------------------------------------
    // Proyeksi: rotasi Y-axis → screen
    //  x'  = x * cos(θ)   [z=0, jadi sin term hilang]
    //  y'  = y             [Y tidak berubah oleh rotasi sumbu Y]
    //  sx  = x' + 64
    //  sy  = -y' + 32      [flip Y: 3D atas = layar atas]
    // ----------------------------------------------------------
    void project(int16_t x3d, int16_t y3d, int16_t* sx, int16_t* sy) {
      int16_t c  = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(_angle + 64)]);
      int16_t xr = (int16_t)(((int32_t)x3d * c) >> 8);
      *sx = xr + (SCREEN_WIDTH  / 2);
      *sy = -y3d + (SCREEN_HEIGHT / 2);
    }

    // Gambar satu garis stroke dari titik lokal (lx1,ly1) → (lx2,ly2)
    // xOff = posisi awal huruf di ruang 3D (sumbu X)
    void stroke(int8_t lx1, int8_t ly1, int8_t lx2, int8_t ly2, int16_t xOff) {
      int16_t px1, py1, px2, py2;
      project(xOff + (int16_t)lx1 * SCALE,  (int16_t)ly1 * SCALE - Y_HALF, &px1, &py1);
      project(xOff + (int16_t)lx2 * SCALE,  (int16_t)ly2 * SCALE - Y_HALF, &px2, &py2);
      _display->drawLine(px1, py1, px2, py2, SSD1306_WHITE);
    }

    // ----------------------------------------------------------
    // Stroke font — grid lokal: x=0..5, y=0..8
    // Kiri-bawah = (0,0), kiri-atas = (0,8)
    // ----------------------------------------------------------

    void drawJ(int16_t x) {
      stroke(0,8,  4,8,  x);   // garis atas
      stroke(4,8,  4,2,  x);   // turun kanan
      stroke(4,2,  2,0,  x);   // lengkung kanan bawah
      stroke(2,0,  0,2,  x);   // lengkung kiri bawah
    }

    void drawA(int16_t x) {
      stroke(0,0,  2,8,  x);   // kaki kiri
      stroke(2,8,  4,0,  x);   // kaki kanan
      stroke(1,4,  3,4,  x);   // palang tengah
    }

    void drawN(int16_t x) {
      stroke(0,0,  0,8,  x);   // tiang kiri
      stroke(0,8,  4,0,  x);   // diagonal
      stroke(4,0,  4,8,  x);   // tiang kanan
    }

    void drawW(int16_t x) {
      stroke(0,8,  1,0,  x);   // kaki kiri luar
      stroke(1,0,  2,4,  x);   // kaki kiri dalam
      stroke(2,4,  3,0,  x);   // kaki kanan dalam
      stroke(3,0,  4,8,  x);   // kaki kanan luar
    }

    void drawR(int16_t x) {
      stroke(0,0,  0,8,  x);   // tiang kiri
      stroke(0,8,  3,8,  x);   // garis atas
      stroke(3,8,  4,6,  x);   // sudut kanan atas
      stroke(4,6,  3,4,  x);   // sudut kanan bawah (punuk)
      stroke(3,4,  0,4,  x);   // palang tengah
      stroke(0,4,  4,0,  x);   // kaki diagonal
    }

  public:
    AnimText3D(Adafruit_SSD1306* disp) : _display(disp), _angle(0) {}

    void reset() { _angle = 0; }

    // Rotasi 1 step/frame → ~1 putaran per 8 detik @30fps
    // Naikkan jadi 2 jika ingin lebih cepat
    void update() { _angle += 1; }

    void draw() {
      // Lebar 1 huruf di 3D = 5 lokal × 3 scale = 15 unit
      // Jarak antar huruf (stride) = 18 unit (= huruf 15 + gap 3)
      // 6 huruf: total span = 6 × 18 = 108, start = -54 (center di 0)
      const int16_t STRIDE = 18;
      const int16_t START  = -(6 * STRIDE / 2);  // = -54

      // J  A  N  W  A  R
      drawJ(START + STRIDE * 0);   // -54
      drawA(START + STRIDE * 1);   // -36
      drawN(START + STRIDE * 2);   // -18
      drawW(START + STRIDE * 3);   //   0
      drawA(START + STRIDE * 4);   //  18
      drawR(START + STRIDE * 5);   //  36
    }
};
