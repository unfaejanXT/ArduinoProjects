#pragma once
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>
#include "../config.h"
#include "../sin_lut.h"

// ============================================================
// AnimPipeCrawler.h
// Animasi pipa 3D yang tumbuh acak seperti screensaver Windows XP Pipes
// ============================================================

class AnimPipeCrawler {
  private:
    Adafruit_SSD1306* _display;

    int16_t _headPos[3];   // Ujung depan pipa (bergerak)
    int16_t _tailPos[3];   // Ujung belakang segmen saat ini

    uint8_t _dir;          // Arah: 0:+X, 1:-X, 2:+Y, 3:-Y, 4:+Z, 5:-Z
    uint8_t _stepsTaken;
    uint8_t _stepsToTake;

    static const int16_t PIPE_R   = 8;   // "Radius" penampang pipa
    static const int16_t STEP_SZ  = 3;   // Piksel per frame
    static const int16_t BOUNDARY = 50;  // Batas area 3D

    // Proyeksi isometrik 3D → 2D menggunakan LUT sinus
    void project(int16_t x, int16_t y, int16_t z, int16_t* ox, int16_t* oy) {
      const uint8_t AX = 40, AY = 40;  // Sudut rotasi statis (tampilan iso)

      int16_t sX = (int16_t)pgm_read_word(&SIN_LUT[AX]);
      int16_t cX = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(AX + 64)]);
      int16_t sY = (int16_t)pgm_read_word(&SIN_LUT[AY]);
      int16_t cY = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(AY + 64)]);

      // Rotasi Y terlebih dahulu
      int16_t x_ry = (int16_t)(((int32_t)z * sY + (int32_t)x * cY) >> 8);
      int16_t z_ry = (int16_t)(((int32_t)z * cY - (int32_t)x * sY) >> 8);

      // Rotasi X
      int16_t y_rx = (int16_t)(((int32_t)y * cX - (int32_t)z_ry * sX) >> 8);

      *ox = x_ry + (SCREEN_WIDTH  / 2);
      *oy = y_rx + (SCREEN_HEIGHT / 2);
    }

    // Pilih arah baru yang berbeda sumbu dari arah saat ini
    void pickNewDir() {
      uint8_t newAxis = random(2);
      if      (_dir <= 1) { _dir = (newAxis == 0) ? random(2, 4) : random(4, 6); }
      else if (_dir <= 3) { _dir = (newAxis == 0) ? random(0, 2) : random(4, 6); }
      else                { _dir = (newAxis == 0) ? random(0, 2) : random(2, 4); }
    }

  public:
    AnimPipeCrawler(Adafruit_SSD1306* disp) : _display(disp) {}

    void reset() {
      _headPos[0] = _headPos[1] = _headPos[2] = 0;
      _tailPos[0] = _tailPos[1] = _tailPos[2] = 0;
      _dir = random(6);
      _stepsTaken = 0;
      _stepsToTake = random(10, 30);
    }

    void update() {
      // Saatnya belok?
      if (_stepsTaken >= _stepsToTake) {
        // Ekor  = posisi kepala sebelum belok
        _tailPos[0] = _headPos[0];
        _tailPos[1] = _headPos[1];
        _tailPos[2] = _headPos[2];
        _stepsTaken  = 0;
        _stepsToTake = random(10, 30);
        pickNewDir();
      }

      // Majukan kepala pipa
      switch (_dir) {
        case 0: _headPos[0] += STEP_SZ; break;
        case 1: _headPos[0] -= STEP_SZ; break;
        case 2: _headPos[1] += STEP_SZ; break;
        case 3: _headPos[1] -= STEP_SZ; break;
        case 4: _headPos[2] += STEP_SZ; break;
        case 5: _headPos[2] -= STEP_SZ; break;
      }
      _stepsTaken++;

      // Reset jika terlalu jauh
      if (abs(_headPos[0]) > BOUNDARY || abs(_headPos[1]) > BOUNDARY || abs(_headPos[2]) > BOUNDARY) {
        reset();
      }
    }

    void draw() {
      int16_t r = PIPE_R;
      int16_t p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
      int16_t e1x, e1y, e2x, e2y, e3x, e3y, e4x, e4y;

      // Hitung 4 sudut penampang di ekor & kepala berdasarkan arah gerak
      if (_dir <= 1) { // Arah X → penampang di bidang YZ
        project(_tailPos[0], _tailPos[1]-r, _tailPos[2]-r, &p1x, &p1y);
        project(_tailPos[0], _tailPos[1]+r, _tailPos[2]-r, &p2x, &p2y);
        project(_tailPos[0], _tailPos[1]+r, _tailPos[2]+r, &p3x, &p3y);
        project(_tailPos[0], _tailPos[1]-r, _tailPos[2]+r, &p4x, &p4y);
        project(_headPos[0], _headPos[1]-r, _headPos[2]-r, &e1x, &e1y);
        project(_headPos[0], _headPos[1]+r, _headPos[2]-r, &e2x, &e2y);
        project(_headPos[0], _headPos[1]+r, _headPos[2]+r, &e3x, &e3y);
        project(_headPos[0], _headPos[1]-r, _headPos[2]+r, &e4x, &e4y);
      } else if (_dir <= 3) { // Arah Y → penampang di bidang XZ
        project(_tailPos[0]-r, _tailPos[1], _tailPos[2]-r, &p1x, &p1y);
        project(_tailPos[0]+r, _tailPos[1], _tailPos[2]-r, &p2x, &p2y);
        project(_tailPos[0]+r, _tailPos[1], _tailPos[2]+r, &p3x, &p3y);
        project(_tailPos[0]-r, _tailPos[1], _tailPos[2]+r, &p4x, &p4y);
        project(_headPos[0]-r, _headPos[1], _headPos[2]-r, &e1x, &e1y);
        project(_headPos[0]+r, _headPos[1], _headPos[2]-r, &e2x, &e2y);
        project(_headPos[0]+r, _headPos[1], _headPos[2]+r, &e3x, &e3y);
        project(_headPos[0]-r, _headPos[1], _headPos[2]+r, &e4x, &e4y);
      } else { // Arah Z → penampang di bidang XY
        project(_tailPos[0]-r, _tailPos[1]-r, _tailPos[2], &p1x, &p1y);
        project(_tailPos[0]+r, _tailPos[1]-r, _tailPos[2], &p2x, &p2y);
        project(_tailPos[0]+r, _tailPos[1]+r, _tailPos[2], &p3x, &p3y);
        project(_tailPos[0]-r, _tailPos[1]+r, _tailPos[2], &p4x, &p4y);
        project(_headPos[0]-r, _headPos[1]-r, _headPos[2], &e1x, &e1y);
        project(_headPos[0]+r, _headPos[1]-r, _headPos[2], &e2x, &e2y);
        project(_headPos[0]+r, _headPos[1]+r, _headPos[2], &e3x, &e3y);
        project(_headPos[0]-r, _headPos[1]+r, _headPos[2], &e4x, &e4y);
      }

      // 4 garis badan pipa (dari ekor ke kepala)
      _display->drawLine(p1x, p1y, e1x, e1y, SSD1306_WHITE);
      _display->drawLine(p2x, p2y, e2x, e2y, SSD1306_WHITE);
      _display->drawLine(p3x, p3y, e3x, e3y, SSD1306_WHITE);
      _display->drawLine(p4x, p4y, e4x, e4y, SSD1306_WHITE);

      // Tutup kepala pipa
      _display->drawLine(e1x, e1y, e2x, e2y, SSD1306_WHITE);
      _display->drawLine(e2x, e2y, e3x, e3y, SSD1306_WHITE);
      _display->drawLine(e3x, e3y, e4x, e4y, SSD1306_WHITE);
      _display->drawLine(e4x, e4y, e1x, e1y, SSD1306_WHITE);
    }
};
