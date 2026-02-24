#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─────────────────────────────────────────────────────────────────
//  PIN DEFINITIONS
// ─────────────────────────────────────────────────────────────────
#define PIN_VOLT       A2    // AC Voltage Sensor ZMPT1010B
#define PIN_CURRENT    A3    // AC Current Sensor ACS712
#define PIN_BTN        2     // Tombol Power (D2)
#define PIN_RELAY      4     // Relay Module (D4)

// ─────────────────────────────────────────────────────────────────
//  OLED SETTINGS
// ─────────────────────────────────────────────────────────────────
#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_ADDR      0x3C
#define OLED_RESET     -1

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

bool relayState = false; // Terputus awal
bool buttonState = HIGH; // Status tombol sesudah debounce
bool lastBtnReading = HIGH; // Status pin terakhir dipantau
unsigned long lastDebounce = 0;
unsigned long lastDisplayMs = 0;

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_VOLT, INPUT);
    pinMode(PIN_CURRENT, INPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);
    
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Relay Active-LOW (HIGH = Mati)

    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("OLED Gagal"));
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Memulai DEBUG Mode"));
    display.display();
    delay(1000);
}

void loop() {
    // 1. Debounce Tombol Relay (Mode Toggle)
    bool reading = digitalRead(PIN_BTN);
    
    // Perbarui waktu jika status tombol berganti
    if (reading != lastBtnReading) {
        lastDebounce = millis();
    }
    
    // Jika state sudah cukup stabil > 50ms
    if ((millis() - lastDebounce) > 50) {
        // Jika pembacaan berbeda dengan status kita saat ini
        if (reading != buttonState) {
            buttonState = reading;
            
            // Hanya deteksi ketika tombol DITEKAN (HIGH --> LOW)
            if (buttonState == LOW) {
                relayState = !relayState;
                
                if (relayState) {
                    digitalWrite(PIN_RELAY, LOW); // Relay Nyala
                } else {
                    digitalWrite(PIN_RELAY, HIGH); // Relay Mati
                }
            }
        }
    }
    lastBtnReading = reading;

    // 2 & 3. Baca Sensor dan Tampilkan di OLED (Update tiap 250ms tanpa delay memblokir)
    if (millis() - lastDisplayMs >= 250) {
        lastDisplayMs = millis();
        
        int v_raw = analogRead(PIN_VOLT);
        int i_raw = analogRead(PIN_CURRENT);

        display.clearDisplay();
        display.setTextSize(1);
        
        // Status Header
        display.setCursor(0, 0);
        display.print(F("MULTIMETER DEBUG RAW"));
        display.setCursor(0, 10);
        display.print(F("Relay: "));
        display.print(relayState ? F("ON") : F("OFF"));
        
        // Blok Voltage
        display.setCursor(0, 22);
        display.print(F("VOLT | A2 (ZMPT)"));
        display.setCursor(0, 32);
        display.print(F("Raw: ")); display.print(v_raw);

        // Blok Current
        display.setCursor(0, 44);
        display.print(F("CURR | A3 (ACS)"));
        display.setCursor(0, 54);
        display.print(F("Raw: ")); display.print(i_raw);
        
        display.display();
    }
}
