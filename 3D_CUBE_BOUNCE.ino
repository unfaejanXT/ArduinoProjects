#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ==========================================
// LOOK-UP TABLE (LUT) SINUS (No Float)
// ==========================================
const int16_t sin_LUT[256] PROGMEM = {
    0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 109, 115, 121, 126, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177, 181, 185, 190, 194, 198, 202, 206, 209, 213, 216, 220, 223, 226, 229, 231, 234, 237, 239, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 255, 255, 256, 256,
    256, 256, 256, 255, 255, 254, 253, 252, 251, 250, 248, 247, 245, 243, 241, 239, 237, 234, 231, 229, 226, 223, 220, 216, 213, 209, 206, 202, 198, 194, 190, 185, 181, 177, 172, 167, 162, 157, 152, 147, 142, 137, 132, 126, 121, 115, 109, 104, 98, 92, 86, 80, 74, 68, 62, 56, 50, 44, 38, 31, 25, 19, 13, 6,
    0, -6, -13, -19, -25, -31, -38, -44, -50, -56, -62, -68, -74, -80, -86, -92, -98, -104, -109, -115, -121, -126, -132, -137, -142, -147, -152, -157, -162, -167, -172, -177, -181, -185, -190, -194, -198, -202, -206, -209, -213, -216, -220, -223, -226, -229, -231, -234, -237, -239, -241, -243, -245, -247, -248, -250, -251, -252, -253, -254, -255, -255, -256, -256,
    -256, -256, -256, -255, -255, -254, -253, -252, -251, -250, -248, -247, -245, -243, -241, -239, -237, -234, -231, -229, -226, -223, -220, -216, -213, -209, -206, -202, -198, -194, -190, -185, -181, -177, -172, -167, -162, -157, -152, -147, -142, -137, -132, -126, -121, -115, -109, -104, -98, -92, -86, -80, -74, -68, -62, -56, -50, -44, -38, -31, -25, -19, -13, -6
};

// ==========================================
// CLASS: FPSCounter
// ==========================================
class FPSCounter {
  private:
    unsigned long lastTime;
    int frames;
    int currentFps;

  public:
    FPSCounter() : lastTime(0), frames(0), currentFps(0) {}
    
    void update() {
      frames++;
      unsigned long currentTime = millis();
      if (currentTime - lastTime >= 1000) {
        currentFps = frames;
        frames = 0;
        lastTime = currentTime;
      }
    }
    
    void draw(Adafruit_SSD1306* display) {
      display->setCursor(0, 0); 
      display->print("FPS:"); 
      display->print(currentFps);
    }
};

// ==========================================
// CLASS: BouncingCube (Logo DVD Style)
// ==========================================
class BouncingCube {
  private:
    Adafruit_SSD1306* display;
    uint8_t angle; 
    
    // Posisi awal di tengah layar
    int16_t posX = SCREEN_WIDTH / 2;
    int16_t posY = SCREEN_HEIGHT / 2;
    
    // Kecepatan gerak (Velocity). 
    // X jalan 2 pixel, Y jalan 1 pixel per frame (Sudut pantul tidak 45 derajat agar lebih acak)
    int8_t velX = 2; 
    int8_t velY = 1; 
    
    // Perkiraan jarak ujung kubus dari titik pusat (Bounding Box)
    const int8_t radius = 18; 

    // Kubus diperkecil ukurannya menjadi ±12
    const int8_t vertices[8][3] = {
      {-12, -12, -12}, { 12, -12, -12}, { 12,  12, -12}, {-12,  12, -12},
      {-12, -12,  12}, { 12, -12,  12}, { 12,  12,  12}, {-12,  12,  12}
    };

    const uint8_t edges[12][2] = {
      {0,1}, {1,2}, {2,3}, {3,0},
      {4,5}, {5,6}, {6,7}, {7,4},
      {0,4}, {1,5}, {2,6}, {3,7}
    };

  public:
    BouncingCube(Adafruit_SSD1306* disp) {
      display = disp;
      angle = 0;
    }

    void update(uint8_t rotationSpeed = 2) { 
      // 1. Putar kubusnya
      angle += rotationSpeed; 

      // 2. Gerakkan posisi 2D-nya
      posX += velX;
      posY += velY;

      // 3. Logika Pantulan (Bouncing)
      // Jika menabrak tembok kiri ATAU tembok kanan, balik arah horizontal
      if (posX - radius <= 0 || posX + radius >= SCREEN_WIDTH) {
        velX = -velX;
      }
      
      // Jika menabrak atap ATAU lantai, balik arah vertikal
      if (posY - radius <= 0 || posY + radius >= SCREEN_HEIGHT) {
        velY = -velY;
      }
    }

    void draw() {
      int16_t rot_x[8];
      int16_t rot_y[8];

      int16_t s = (int16_t)pgm_read_word(&sin_LUT[angle]);
      int16_t c = (int16_t)pgm_read_word(&sin_LUT[(uint8_t)(angle + 64)]);

      // Hitung 3D rotasi
      for (uint8_t i = 0; i < 8; i++) {
        int8_t x = vertices[i][0];
        int8_t y = vertices[i][1];
        int8_t z = vertices[i][2];

        int16_t yx = (y * c - z * s) >> 8;
        int16_t zx = (y * s + z * c) >> 8;
        int16_t xx = (x * c - zx * s) >> 8;

        // Terapkan posisi X dan Y yang dinamis sebagai offset layar
        rot_x[i] = xx + posX;
        rot_y[i] = yx + posY;
      }

      // Gambar 12 garis
      for (uint8_t i = 0; i < 12; i++) {
        uint8_t p1 = edges[i][0];
        uint8_t p2 = edges[i][1];
        display->drawLine(rot_x[p1], rot_y[p1], rot_x[p2], rot_y[p2], SSD1306_WHITE);
      }
    }
};

// ==========================================
// GLOBAL VARIABLES
// ==========================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
FPSCounter fpsCounter;
BouncingCube myCube(&display); 

// ==========================================
// SETUP
// ==========================================
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.setClock(400000UL); // Fast Mode
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  display.clearDisplay();

  // Update pergerakan dan putaran
  myCube.update(2); // Angka 2 adalah kecepatan rotasi 3D-nya
  fpsCounter.update();   

  // Render ke layar
  myCube.draw();
  fpsCounter.draw(&display); 

  display.display();
}