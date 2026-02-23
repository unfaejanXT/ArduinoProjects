#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float cube[8][3] = {
  {-20, -20, -20},
  { 20, -20, -20},
  { 20,  20, -20},
  {-20,  20, -20},
  {-20, -20,  20},
  { 20, -20,  20},
  { 20,  20,  20},
  {-20,  20,  20}
};

int edges[12][2] = {
  {0,1},{1,2},{2,3},{3,0},
  {4,5},{5,6},{6,7},{7,4},
  {0,4},{1,5},{2,6},{3,7}
};

float angle = 0;

// FPS variables
unsigned long lastTime = 0;
int frames = 0;
int fps = 0;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  float rotated[8][3];

  // Rotasi cube (X + Y)
  for (int i = 0; i < 8; i++) {
    float x = cube[i][0];
    float y = cube[i][1];
    float z = cube[i][2];

    float yx = y * cos(angle) - z * sin(angle);
    float zx = y * sin(angle) + z * cos(angle);

    float xx = x * cos(angle) - zx * sin(angle);
    float zz = x * sin(angle) + zx * cos(angle);

    rotated[i][0] = xx;
    rotated[i][1] = yx;
    rotated[i][2] = zz;
  }

  // Gambar edges
  for (int i = 0; i < 12; i++) {
    int p1 = edges[i][0];
    int p2 = edges[i][1];

    int x1 = rotated[p1][0] + 64;
    int y1 = rotated[p1][1] + 32;

    int x2 = rotated[p2][0] + 64;
    int y2 = rotated[p2][1] + 32;

    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // ===== FPS Counter =====
  frames++;
  if (millis() - lastTime >= 1000) {
    fps = frames;
    frames = 0;
    lastTime = millis();
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("FPS:");
  display.print(fps);

  display.display();

  angle += 0.04;
}