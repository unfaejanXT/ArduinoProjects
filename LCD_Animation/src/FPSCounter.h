#pragma once
#include <Adafruit_SSD1306.h>

// ============================================================
// FPSCounter.h
// Class ringan untuk menghitung dan menampilkan FPS ke layar
// ============================================================

class FPSCounter {
  private:
    unsigned long _lastTime;
    int           _frames;
    int           _currentFps;

  public:
    FPSCounter() : _lastTime(0), _frames(0), _currentFps(0) {}

    // Panggil sekali per frame untuk menghitung FPS
    void update() {
      _frames++;
      unsigned long now = millis();
      if (now - _lastTime >= 1000UL) {
        _currentFps = _frames;
        _frames     = 0;
        _lastTime   = now;
      }
    }

    // Render teks "FPS:XX" di pojok kiri atas
    void draw(Adafruit_SSD1306* display) {
      display->setCursor(0, 0);
      display->print(F("FPS:"));
      display->print(_currentFps);
    }

    int getFPS() const { return _currentFps; }
};
