// ============================================================
//  Text3D_Standalone.ino
//
//  Animasi teks "JANWAR" berputar 3D pada layar OLED SSD1306
//  Hardware : NodeMCU V3 (ESP8266) + OLED 128x64 I2C
//
//  Library  : Adafruit SSD1306 + Adafruit GFX Library
//
//  Cara kerja:
//   - Setiap huruf didefinisikan sebagai stroke (garis) dalam
//     grid lokal 5 lebar × 8 tinggi unit.
//   - Semua stroke dirotasi pada sumbu Y menggunakan LUT sinus
//     integer (tanpa float) lalu diproyeksikan ke layar 2D.
//   - Efeknya seperti papan nama yang berputar horizontal.
// ============================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

// ------------------------------------------------------------
// Konfigurasi
// ------------------------------------------------------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_ADDR    0x3C

// ------------------------------------------------------------
// Look-Up Table Sinus — sin(angle) × 256
// 256 entry = satu lingkaran penuh (0°–360°)
// Disimpan di Flash (PROGMEM) untuk hemat RAM
// ------------------------------------------------------------
const int16_t SIN_LUT[256] PROGMEM = {
    0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92,
    98, 104, 109, 115, 121, 126, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177,
    181, 185, 190, 194, 198, 202, 206, 209, 213, 216, 220, 223, 226, 229, 231, 234,
    237, 239, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 255, 255, 256, 256,
    256, 256, 256, 255, 255, 254, 253, 252, 251, 250, 248, 247, 245, 243, 241, 239,
    237, 234, 231, 229, 226, 223, 220, 216, 213, 209, 206, 202, 198, 194, 190, 185,
    181, 177, 172, 167, 162, 157, 152, 147, 142, 137, 132, 126, 121, 115, 109, 104,
    98, 92, 86, 80, 74, 68, 62, 56, 50, 44, 38, 31, 25, 19, 13, 6,
    0, -6, -13, -19, -25, -31, -38, -44, -50, -56, -62, -68, -74, -80, -86, -92,
    -98, -104, -109, -115, -121, -126, -132, -137, -142, -147, -152, -157, -162, -167, -172, -177,
    -181, -185, -190, -194, -198, -202, -206, -209, -213, -216, -220, -223, -226, -229, -231, -234,
    -237, -239, -241, -243, -245, -247, -248, -250, -251, -252, -253, -254, -255, -255, -256, -256,
    -256, -256, -256, -255, -255, -254, -253, -252, -251, -250, -248, -247, -245, -243, -241, -239,
    -237, -234, -231, -229, -226, -223, -220, -216, -213, -209, -206, -202, -198, -194, -190, -185,
    -181, -177, -172, -167, -162, -157, -152, -147, -142, -137, -132, -126, -121, -115, -109, -104,
    -98, -92, -86, -80, -74, -68, -62, -56, -50, -44, -38, -31, -25, -19, -13, -6
};

// ------------------------------------------------------------
// FPS Counter
// ------------------------------------------------------------
class FPSCounter {
  private:
    unsigned long _lastTime;
    int           _frames;
    int           _currentFps;
  public:
    FPSCounter() : _lastTime(0), _frames(0), _currentFps(0) {}

    void update() {
      _frames++;
      unsigned long now = millis();
      if (now - _lastTime >= 1000UL) {
        _currentFps = _frames;
        _frames     = 0;
        _lastTime   = now;
      }
    }

    void draw(Adafruit_SSD1306* d) {
      d->setCursor(0, 0);
      d->print(F("FPS:"));
      d->print(_currentFps);
    }
};

// ------------------------------------------------------------
// AnimText3D — "JANWAR" berputar 3D pada sumbu Y
// ------------------------------------------------------------
class AnimText3D {
  private:
    Adafruit_SSD1306* _display;
    uint8_t           _angle;     // 0–255 = 0°–360°

    // Scale: 1 unit lokal = 3 pixel 3D
    // Tinggi lokal = 8 unit → 24px; setengahnya = 12 (untuk centering)
    static const int8_t SCALE  = 3;
    static const int8_t Y_HALF = 12;

    // Proyeksi rotasi sumbu Y → koordinat layar
    // Karena z = 0: x' = x × cos(θ), y' = y (tidak berubah)
    void project(int16_t x3d, int16_t y3d, int16_t* sx, int16_t* sy) {
      int16_t c  = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(_angle + 64)]);
      int16_t xr = (int16_t)(((int32_t)x3d * c) >> 8);
      *sx = xr + (SCREEN_WIDTH  / 2);
      *sy = -y3d + (SCREEN_HEIGHT / 2);   // flip Y: 3D atas = layar atas
    }

    // Gambar satu garis dari (lx1,ly1) → (lx2,ly2) di koordinat lokal
    // xOff = posisi awal huruf di sumbu X ruang 3D
    void stroke(int8_t lx1, int8_t ly1, int8_t lx2, int8_t ly2, int16_t xOff) {
      int16_t px1, py1, px2, py2;
      project(xOff + (int16_t)lx1 * SCALE, (int16_t)ly1 * SCALE - Y_HALF, &px1, &py1);
      project(xOff + (int16_t)lx2 * SCALE, (int16_t)ly2 * SCALE - Y_HALF, &px2, &py2);
      _display->drawLine(px1, py1, px2, py2, SSD1306_WHITE);
    }

    // ---- Stroke font — grid 5 lebar × 8 tinggi ----
    // Titik (0,0) = kiri bawah, (5,8) = kanan atas

    void drawJ(int16_t x) {
      stroke(0,8, 4,8, x);   // garis atas horizontal
      stroke(4,8, 4,2, x);   // batang turun kanan
      stroke(4,2, 2,0, x);   // lengkung kanan bawah
      stroke(2,0, 0,2, x);   // lengkung kiri bawah
    }

    void drawA(int16_t x) {
      stroke(0,0, 2,8, x);   // kaki kiri
      stroke(2,8, 4,0, x);   // kaki kanan
      stroke(1,4, 3,4, x);   // palang tengah
    }

    void drawN(int16_t x) {
      stroke(0,0, 0,8, x);   // tiang kiri
      stroke(0,8, 4,0, x);   // diagonal
      stroke(4,0, 4,8, x);   // tiang kanan
    }

    void drawW(int16_t x) {
      stroke(0,8, 1,0, x);   // kaki kiri luar
      stroke(1,0, 2,4, x);   // kaki kiri dalam
      stroke(2,4, 3,0, x);   // kaki kanan dalam
      stroke(3,0, 4,8, x);   // kaki kanan luar
    }

    void drawR(int16_t x) {
      stroke(0,0, 0,8, x);   // tiang kiri
      stroke(0,8, 3,8, x);   // garis atas
      stroke(3,8, 4,6, x);   // sudut kanan atas (punuk)
      stroke(4,6, 3,4, x);   // sudut kanan bawah (punuk)
      stroke(3,4, 0,4, x);   // palang tengah
      stroke(0,4, 4,0, x);   // kaki diagonal
    }

  public:
    AnimText3D(Adafruit_SSD1306* disp) : _display(disp), _angle(0) {}

    void reset() { _angle = 0; }

    // Naikkan += 2 jika ingin lebih cepat
    void update() { _angle += 1; }

    void draw() {
      // Setiap huruf menempati 18 unit X (lebar 15 + jarak 3)
      // 6 huruf × 18 = 108 total, start = -54 agar teks di tengah
      const int16_t STRIDE = 18;
      const int16_t START  = -(6 * STRIDE / 2);  // = -54

      drawJ(START + STRIDE * 0);   // J  pada x = -54
      drawA(START + STRIDE * 1);   // A  pada x = -36
      drawN(START + STRIDE * 2);   // N  pada x = -18
      drawW(START + STRIDE * 3);   // W  pada x =   0
      drawA(START + STRIDE * 4);   // A  pada x =  18
      drawR(START + STRIDE * 5);   // R  pada x =  36
    }
};

// ------------------------------------------------------------
// Global objects
// ------------------------------------------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
FPSCounter       fpsCounter;
AnimText3D       textAnim(&display);

// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[ERROR] OLED tidak terdeteksi!"));
    while (true);
  }

  Wire.setClock(400000UL);   // Fast Mode I2C

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  textAnim.reset();
  Serial.println(F("[OK] Text3D siap!"));
}

// ------------------------------------------------------------
void loop() {
  display.clearDisplay();

  textAnim.update();
  textAnim.draw();

  fpsCounter.update();
  fpsCounter.draw(&display);

  display.display();
}
