#pragma once
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "FPSCounter.h"
#include "animations/Anim3DCube.h"
#include "animations/AnimBouncingCube.h"
#include "animations/AnimPipeCrawler.h"
#include "animations/AnimText3D.h"

// ============================================================
// AnimationManager.h
// Mengelola rotasi antar animasi setiap ANIM_DURATION_MS milidetik.
//
// Cara menambah animasi baru:
//  1. Buat file class baru di src/animations/
//  2. #include di sini
//  3. Tambahkan instance sebagai anggota private
//  4. Daftarkan di switch-case _runCurrent() dan _resetCurrent()
//  5. Update TOTAL_ANIMATIONS di config.h
// ============================================================

class AnimationManager {
  private:
    Adafruit_SSD1306* _display;
    FPSCounter        _fps;

    uint8_t           _currentIndex;
    unsigned long     _lastSwitch;

    // ---- Instansi semua animasi ----
    Anim3DCube       _cube;
    AnimBouncingCube _bouncingCube;
    AnimPipeCrawler  _pipe;
    AnimText3D       _text3D;

    // Reset state animasi yang sedang aktif
    void _resetCurrent() {
      switch (_currentIndex) {
        case 0: _cube.reset();         break;
        case 1: _bouncingCube.reset(); break;
        case 2: _pipe.reset();         break;
        case 3: _text3D.reset();       break;
      }
    }

    // Jalankan satu frame animasi yang sedang aktif
    void _runCurrent() {
      switch (_currentIndex) {
        case 0:
          _cube.update();
          _cube.draw();
          break;
        case 1:
          _bouncingCube.update();
          _bouncingCube.draw();
          break;
        case 2:
          _pipe.update();
          _pipe.draw();
          break;
        case 3:
          _text3D.update();
          _text3D.draw();
          break;
      }
    }

  public:
    AnimationManager(Adafruit_SSD1306* disp)
      : _display(disp),
        _currentIndex(0),
        _lastSwitch(0),
        _cube(disp),
        _bouncingCube(disp),
        _pipe(disp),
        _text3D(disp)
    {}

    void begin() {
      randomSeed(analogRead(A0)); // Seed acak dari noise pin analog
      _display->setTextSize(1);
      _display->setTextColor(SSD1306_WHITE);
      _resetCurrent();
      _lastSwitch = millis();
    }

    // Panggil di setiap loop()
    void tick() {
      unsigned long now = millis();

      // Cek apakah sudah waktunya ganti animasi
      if (now - _lastSwitch >= ANIM_DURATION_MS) {
        _currentIndex = (_currentIndex + 1) % TOTAL_ANIMATIONS;
        _resetCurrent();
        _lastSwitch = now;
      }

      // Render frame normal
      _display->clearDisplay();
      _display->setTextSize(1);
      _display->setTextColor(SSD1306_WHITE);

      _runCurrent();      // Gambar animasi
      _fps.update();
      _fps.draw(_display); // Overlay FPS di pojok kiri atas

      _display->display();
    }
};
