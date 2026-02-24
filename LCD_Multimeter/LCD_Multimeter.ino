/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║                  PROYEK LCD MULTIMETER                       ║
 * ║  Multimeter Listrik Digital dengan Layar OLED (Procedural)   ║
 * ╚══════════════════════════════════════════════════════════════╝
 * 
 * Hardware:
 * 1x Arduino Uno
 * 1x Passive Buzzer (D3)
 * 1x AC Voltage Sensor ZMPT1010B LM358 V3 (A2)
 * 1x AC Current Sensor ACS712 (A3)
 * 1x Relay Module (D4)
 * 1x OLED 128x64 Display (SDA=A4, SCL=A5)
 * 1x Button / Btn0 (D2)
 * 
 * Paradigma Pemoatgraman: Procedural
 * Data terisolasi di dalam masing-masing module `src/`
 */

#include "src/app.h"

void setup() {
    appSetup();
}

void loop() {
    appLoop();
}
