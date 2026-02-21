#pragma once
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>
#include "../config.h"
#include "../sin_lut.h"

// ============================================================
// AnimBouncingCube.h
// Kubus 3D yang berputar dan memantul di seluruh layar (DVD-logo style)
// Menggunakan LUT sinus integer agar lebih cepat dari floating-point
// ============================================================

class AnimBouncingCube {
  private:
    Adafruit_SSD1306* _display;
    uint8_t  _angle;

    int16_t  _posX;
    int16_t  _posY;
    int8_t   _velX;
    int8_t   _velY;

    static const int8_t RADIUS    = 18; // Bounding box dari pusat ke tepi
    static const int8_t ROT_SPEED = 2;  // Kecepatan rotasi

    // Vertex kubus ukuran ±12
    const int8_t _vertices[8][3] = {
      {-12,-12,-12},{ 12,-12,-12},{ 12, 12,-12},{-12, 12,-12},
      {-12,-12, 12},{ 12,-12, 12},{ 12, 12, 12},{-12, 12, 12}
    };

    const uint8_t _edges[12][2] = {
      {0,1},{1,2},{2,3},{3,0},
      {4,5},{5,6},{6,7},{7,4},
      {0,4},{1,5},{2,6},{3,7}
    };

  public:
    AnimBouncingCube(Adafruit_SSD1306* disp) : _display(disp) {}

    // Reset ke posisi dan kecepatan awal saat animasi mulai
    void reset() {
      _angle = 0;
      _posX  = SCREEN_WIDTH  / 2;
      _posY  = SCREEN_HEIGHT / 2;
      _velX  = 2;
      _velY  = 1;
    }

    void update() {
      _angle += ROT_SPEED;

      _posX += _velX;
      _posY += _velY;

      // Pantulan horizontal
      if (_posX - RADIUS <= 0 || _posX + RADIUS >= SCREEN_WIDTH)  _velX = -_velX;
      // Pantulan vertikal
      if (_posY - RADIUS <= 0 || _posY + RADIUS >= SCREEN_HEIGHT) _velY = -_velY;
    }

    void draw() {
      int16_t rot_x[8], rot_y[8];

      int16_t s = (int16_t)pgm_read_word(&SIN_LUT[_angle]);
      int16_t c = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(_angle + 64)]);

      for (uint8_t i = 0; i < 8; i++) {
        int8_t x = _vertices[i][0];
        int8_t y = _vertices[i][1];
        int8_t z = _vertices[i][2];

        int16_t yx = (y * c - z * s) >> 8;
        int16_t zx = (y * s + z * c) >> 8;
        int16_t xx = (x * c - zx * s) >> 8;

        // Offset dari posisi bouncing
        rot_x[i] = xx + _posX;
        rot_y[i] = yx + _posY;
      }

      for (uint8_t i = 0; i < 12; i++) {
        uint8_t p1 = _edges[i][0];
        uint8_t p2 = _edges[i][1];
        _display->drawLine(rot_x[p1], rot_y[p1], rot_x[p2], rot_y[p2], SSD1306_WHITE);
      }
    }
};
