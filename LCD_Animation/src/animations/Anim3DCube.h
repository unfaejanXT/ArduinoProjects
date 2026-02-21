#pragma once
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "../config.h"

// ============================================================
// Anim3DCube.h
// Animasi kubus 3D berputar di tengah layar (float-based rotation)
// ============================================================

class Anim3DCube {
  private:
    Adafruit_SSD1306* _display;
    float _angle;

    // 8 vertex kubus
    const float _vertices[8][3] = {
      {-20, -20, -20}, { 20, -20, -20}, { 20,  20, -20}, {-20,  20, -20},
      {-20, -20,  20}, { 20, -20,  20}, { 20,  20,  20}, {-20,  20,  20}
    };

    // 12 edge yang menghubungkan vertex
    const uint8_t _edges[12][2] = {
      {0,1},{1,2},{2,3},{3,0},
      {4,5},{5,6},{6,7},{7,4},
      {0,4},{1,5},{2,6},{3,7}
    };

  public:
    Anim3DCube(Adafruit_SSD1306* disp) : _display(disp), _angle(0.0f) {}

    // Reset state saat animasi ini mulai tampil
    void reset() {
      _angle = 0.0f;
    }

    void update() {
      _angle += 0.04f;
    }

    void draw() {
      float rotated[8][2]; // Simpan hanya proyeksi X,Y

      for (uint8_t i = 0; i < 8; i++) {
        float x = _vertices[i][0];
        float y = _vertices[i][1];
        float z = _vertices[i][2];

        // Rotasi sumbu X
        float yx = y * cos(_angle) - z * sin(_angle);
        float zx = y * sin(_angle) + z * cos(_angle);

        // Rotasi sumbu Y
        float xx = x * cos(_angle) - zx * sin(_angle);

        // Proyeksi ortografis ke 2D (tengah layar)
        rotated[i][0] = xx + (SCREEN_WIDTH  / 2);
        rotated[i][1] = yx + (SCREEN_HEIGHT / 2);
      }

      // Gambar 12 garis edge
      for (uint8_t i = 0; i < 12; i++) {
        uint8_t p1 = _edges[i][0];
        uint8_t p2 = _edges[i][1];
        _display->drawLine(
          (int16_t)rotated[p1][0], (int16_t)rotated[p1][1],
          (int16_t)rotated[p2][0], (int16_t)rotated[p2][1],
          SSD1306_WHITE
        );
      }
    }
};
