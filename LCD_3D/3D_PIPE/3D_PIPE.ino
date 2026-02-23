#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ==========================================
// LOOK-UP TABLE (LUT) SINUS (Tetap sama)
// ==========================================
const int16_t sin_LUT[256] PROGMEM = {
    0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 109, 115, 121, 126, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177, 181, 185, 190, 194, 198, 202, 206, 209, 213, 216, 220, 223, 226, 229, 231, 234, 237, 239, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 255, 255, 256, 256,
    256, 256, 256, 255, 255, 254, 253, 252, 251, 250, 248, 247, 245, 243, 241, 239, 237, 234, 231, 229, 226, 223, 220, 216, 213, 209, 206, 202, 198, 194, 190, 185, 181, 177, 172, 167, 162, 157, 152, 147, 142, 137, 132, 126, 121, 115, 109, 104, 98, 92, 86, 80, 74, 68, 62, 56, 50, 44, 38, 31, 25, 19, 13, 6,
    0, -6, -13, -19, -25, -31, -38, -44, -50, -56, -62, -68, -74, -80, -86, -92, -98, -104, -109, -115, -121, -126, -132, -137, -142, -147, -152, -157, -162, -167, -172, -177, -181, -185, -190, -194, -198, -202, -206, -209, -213, -216, -220, -223, -226, -229, -231, -234, -237, -239, -241, -243, -245, -247, -248, -250, -251, -252, -253, -254, -255, -255, -256, -256,
    -256, -256, -256, -255, -255, -254, -253, -252, -251, -250, -248, -247, -245, -243, -241, -239, -237, -234, -231, -229, -226, -223, -220, -216, -213, -209, -206, -202, -198, -194, -190, -185, -181, -177, -172, -167, -162, -157, -152, -147, -142, -137, -132, -126, -115, -109, -104, -98, -92, -86, -80, -74, -68, -62, -56, -50, -44, -38, -31, -25, -19, -13, -6
};

// ==========================================
// CLASS: FPSCounter (Tetap sama)
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
        currentFps = frames; frames = 0; lastTime = currentTime;
      }
    }
    void draw(Adafruit_SSD1306* display) {
      display->setCursor(0, 0); display->print("FPS:"); display->print(currentFps);
    }
};

// ==========================================
// CLASS: PipeCrawler (Pipa Windows XP Style)
// ==========================================
class PipeCrawler {
  private:
    Adafruit_SSD1306* display;
    int16_t currentPos[3]; // Posisi "kepala" pipa saat ini (ujung depan)
    int16_t startPos[3];   // Posisi "ekor" segmen saat ini (ujung belakang)
    
    uint8_t currentDir; // 0:+X, 1:-X, 2:+Y, 3:-Y, 4:+Z, 5:-Z
    uint8_t stepsTaken; // Berapa langkah sudah berjalan di arah ini
    uint8_t stepsToTake; // Target langkah sebelum belok

    const int16_t PIPE_RADIUS = 8; // Ketebalan pipa
    const int16_t STEP_SIZE = 3;   // Kecepatan pertumbuhan per frame
    const int16_t BOUNDARY = 50;   // Batas area agar tidak keluar layar terlalu jauh

    // Fungsi bantuan untuk memproyeksikan 3D ke 2D (Optimized)
    void project(int16_t x, int16_t y, int16_t z, int16_t* outX, int16_t* outY) {
      // Kita gunakan rotasi statis sedikit agar terlihat 3D (Isometric-ish view)
      uint8_t angleX = 40; 
      uint8_t angleY = 40;
      
      int16_t sX = (int16_t)pgm_read_word(&sin_LUT[angleX]);
      int16_t cX = (int16_t)pgm_read_word(&sin_LUT[(uint8_t)(angleX + 64)]);
      int16_t sY = (int16_t)pgm_read_word(&sin_LUT[angleY]);
      int16_t cY = (int16_t)pgm_read_word(&sin_LUT[(uint8_t)(angleY + 64)]);

      // Rotasi Y dulu
      int32_t zx_long = (int32_t)z * cY - (int32_t)x * sY;
      int32_t xx_long = (int32_t)z * sY + (int32_t)x * cY;
      int16_t z_rotY = zx_long >> 8;
      int16_t x_rotY = xx_long >> 8;

      // Rotasi X kemudian
      int32_t yx_long = (int32_t)y * cX - (int32_t)z_rotY * sX;
      int16_t y_final = yx_long >> 8;

      *outX = x_rotY + 64;
      *outY = y_final + 32;
    }

  public:
    PipeCrawler(Adafruit_SSD1306* disp) {
      display = disp;
      reset();
    }

    void reset() {
      currentPos[0] = 0; currentPos[1] = 0; currentPos[2] = 0;
      startPos[0] = 0; startPos[1] = 0; startPos[2] = 0;
      currentDir = random(6); // Pilih arah awal acak
      stepsTaken = 0;
      stepsToTake = random(10, 30); // Jalan 10-30 langkah sebelum belok
    }

    void update() {
      // Jika sudah mencapai target langkah, tentukan arah baru
      if (stepsTaken >= stepsToTake) {
        // Simpan posisi kepala saat ini menjadi ekor untuk segmen berikutnya
        startPos[0] = currentPos[0];
        startPos[1] = currentPos[1];
        startPos[2] = currentPos[2];
        
        stepsTaken = 0;
        stepsToTake = random(10, 30);

        // Logika Belok 90 Derajat:
        // Jika bergerak di X (dir 0/1), harus belok ke Y (2/3) atau Z (4/5)
        uint8_t newAxis = random(2); // 0 atau 1
        if (currentDir <= 1) { // Sedang di X
          currentDir = (newAxis == 0) ? random(2, 4) : random(4, 6);
        } else if (currentDir <= 3) { // Sedang di Y
           currentDir = (newAxis == 0) ? random(0, 2) : random(4, 6);
        } else { // Sedang di Z
           currentDir = (newAxis == 0) ? random(0, 2) : random(2, 4);
        }
      }

      // Gerakkan kepala pipa
      switch(currentDir) {
        case 0: currentPos[0] += STEP_SIZE; break; // +X
        case 1: currentPos[0] -= STEP_SIZE; break; // -X
        case 2: currentPos[1] += STEP_SIZE; break; // +Y
        case 3: currentPos[1] -= STEP_SIZE; break; // -Y
        case 4: currentPos[2] += STEP_SIZE; break; // +Z
        case 5: currentPos[2] -= STEP_SIZE; break; // -Z
      }
      stepsTaken++;

      // Reset jika keluar batas terlalu jauh
      if (abs(currentPos[0]) > BOUNDARY || abs(currentPos[1]) > BOUNDARY || abs(currentPos[2]) > BOUNDARY) {
        reset();
      }
    }

    void draw() {
      int16_t r = PIPE_RADIUS;
      int16_t p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
      int16_t e1x, e1y, e2x, e2y, e3x, e3y, e4x, e4y;

      // Tentukan 4 titik sudut di Pangkal (Start) dan Ujung (End)
      // berdasarkan arah gerakan saat ini.
      if (currentDir <= 1) { // Gerak di X, penampang di YZ
        project(startPos[0], startPos[1]-r, startPos[2]-r, &p1x, &p1y);
        project(startPos[0], startPos[1]+r, startPos[2]-r, &p2x, &p2y);
        project(startPos[0], startPos[1]+r, startPos[2]+r, &p3x, &p3y);
        project(startPos[0], startPos[1]-r, startPos[2]+r, &p4x, &p4y);
        
        project(currentPos[0], currentPos[1]-r, currentPos[2]-r, &e1x, &e1y);
        project(currentPos[0], currentPos[1]+r, currentPos[2]-r, &e2x, &e2y);
        project(currentPos[0], currentPos[1]+r, currentPos[2]+r, &e3x, &e3y);
        project(currentPos[0], currentPos[1]-r, currentPos[2]+r, &e4x, &e4y);
      } else if (currentDir <= 3) { // Gerak di Y, penampang di XZ
        project(startPos[0]-r, startPos[1], startPos[2]-r, &p1x, &p1y);
        project(startPos[0]+r, startPos[1], startPos[2]-r, &p2x, &p2y);
        project(startPos[0]+r, startPos[1], startPos[2]+r, &p3x, &p3y);
        project(startPos[0]-r, startPos[1], startPos[2]+r, &p4x, &p4y);

        project(currentPos[0]-r, currentPos[1], currentPos[2]-r, &e1x, &e1y);
        project(currentPos[0]+r, currentPos[1], currentPos[2]-r, &e2x, &e2y);
        project(currentPos[0]+r, currentPos[1], currentPos[2]+r, &e3x, &e3y);
        project(currentPos[0]-r, currentPos[1], currentPos[2]+r, &e4x, &e4y);
      } else { // Gerak di Z, penampang di XY
        project(startPos[0]-r, startPos[1]-r, startPos[2], &p1x, &p1y);
        project(startPos[0]+r, startPos[1]-r, startPos[2], &p2x, &p2y);
        project(startPos[0]+r, startPos[1]+r, startPos[2], &p3x, &p3y);
        project(startPos[0]-r, startPos[1]+r, startPos[2], &p4x, &p4y);

        project(currentPos[0]-r, currentPos[1]-r, currentPos[2], &e1x, &e1y);
        project(currentPos[0]+r, currentPos[1]-r, currentPos[2], &e2x, &e2y);
        project(currentPos[0]+r, currentPos[1]+r, currentPos[2], &e3x, &e3y);
        project(currentPos[0]-r, currentPos[1]+r, currentPos[2], &e4x, &e4y);
      }

      // Gambar 4 garis penghubung (badan pipa)
      display->drawLine(p1x, p1y, e1x, e1y, SSD1306_WHITE);
      display->drawLine(p2x, p2y, e2x, e2y, SSD1306_WHITE);
      display->drawLine(p3x, p3y, e3x, e3y, SSD1306_WHITE);
      display->drawLine(p4x, p4y, e4x, e4y, SSD1306_WHITE);

      // Gambar tutup di ujung kepala (agar terlihat solid saat maju)
      display->drawLine(e1x, e1y, e2x, e2y, SSD1306_WHITE);
      display->drawLine(e2x, e2y, e3x, e3y, SSD1306_WHITE);
      display->drawLine(e3x, e3y, e4x, e4y, SSD1306_WHITE);
      display->drawLine(e4x, e4y, e1x, e1y, SSD1306_WHITE);
      
      // Opsional: Gambar tutup di pangkal (jika ingin terlihat seperti potongan terpisah)
      // display->drawLine(p1x, p1y, p2x, p2y, SSD1306_WHITE);
      // display->drawLine(p2x, p2y, p3x, p3y, SSD1306_WHITE);
      // display->drawLine(p3x, p3y, p4x, p4y, SSD1306_WHITE);
      // display->drawLine(p4x, p4y, p1x, p1y, SSD1306_WHITE);
    }
};

// ==========================================
// GLOBAL VARIABLES & SETUP
// ==========================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
FPSCounter fpsCounter;
PipeCrawler myPipe(&display);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.setClock(400000UL); // Fast Mode
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  randomSeed(analogRead(0)); // Agar gerakan acak berbeda setiap restart
}

void loop() {
  display.clearDisplay();
  
  myPipe.update();
  myPipe.draw();
  
  fpsCounter.update();
  fpsCounter.draw(&display);
  
  display.display();
  // delay(20); // Uncomment jika terlalu cepat
}