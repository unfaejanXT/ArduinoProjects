# 🛡️ SONAR GUARDIAN

## Smart Proximity Alert System — Arduino Uno

> Sistem radar sonar cerdas berbasis Arduino Uno yang menampilkan visualisasi jarak secara real-time pada OLED 128×64 dengan animasi radar sweep, dan membunyikan alarm buzzer adaptif berdasarkan jarak deteksi.

---

## 🎬 Fitur Utama

| Fitur                      | Deskripsi                                                         |
| -------------------------- | ----------------------------------------------------------------- |
| 🎯 **Radar Sweep Animasi** | Tampilan radar berputar 360° di OLED seperti sonar kapal selam    |
| 📏 **Jarak Real-time**     | Pembacaan jarak HC-SR04 dengan moving average untuk hasil stabil  |
| 🔊 **Alarm Adaptif**       | Beep semakin cepat seiring objek mendekat (seperti sensor parkir) |
| 🎵 **Melodi Startup**      | Nada boot animatif saat sistem pertama kali dinyalakan            |
| ⚠️ **Tiga Zona Deteksi**   | SAFE 🟢 / WARNING 🟡 / DANGER 🔴 dengan warna & suara berbeda     |
| ✨ **Efek Berkedip**       | Border OLED berkedip saat memasuki zona DANGER                    |
| 🖥️ **Splash Screen**       | Animasi boot dengan progress bar dan animasi radar                |
| 📊 **Bar Chart Jarak**     | Visualisasi persentase jarak dalam bentuk bar di panel kanan      |

---

## 🔌 Hardware & Wiring

### Komponen

- **Arduino Uno** (mikrokontroler utama)
- **OLED Display 128×64** (SSD1306, komunikasi I2C)
- **Sensor Ultrasonik HC-SR04** (pengukur jarak)
- **Passive Buzzer** (output suara)

### Tabel Wiring

| Komponen           | Pin Komponen | Pin Arduino |
| ------------------ | ------------ | ----------- |
| **OLED SSD1306**   | SCL          | A5          |
| **OLED SSD1306**   | SDA          | A4          |
| **OLED SSD1306**   | VCC          | 3.3V / 5V   |
| **OLED SSD1306**   | GND          | GND         |
| **HC-SR04**        | TRIG         | 9           |
| **HC-SR04**        | ECHO         | 10          |
| **HC-SR04**        | VCC          | 5V          |
| **HC-SR04**        | GND          | GND         |
| **Passive Buzzer** | + (signal)   | 3 (PWM)     |
| **Passive Buzzer** | - (GND)      | GND         |

### Diagram Koneksi

```
Arduino Uno
  ┌──────────────────────────┐
  │  D3  ────────── Buzzer + │
  │  D9  ────────── HC-SR04 TRIG
  │  D10 ────────── HC-SR04 ECHO
  │  A4  ────────── OLED SDA │
  │  A5  ────────── OLED SCL │
  │  5V  ────┬───── HC-SR04 VCC
  │          └───── OLED VCC │
  │  GND ────┬───── HC-SR04 GND
  │          ├───── OLED GND │
  │          └───── Buzzer - │
  └──────────────────────────┘
```

---

## 📚 Library yang Dibutuhkan

Install via **Arduino IDE → Tools → Library Manager**:

| Library                | Author   | Fungsi                                   |
| ---------------------- | -------- | ---------------------------------------- |
| `Adafruit SSD1306`     | Adafruit | Driver OLED SSD1306                      |
| `Adafruit GFX Library` | Adafruit | Primitif grafis (garis, lingkaran, teks) |

> **Wire.h** sudah termasuk bawaan Arduino IDE, tidak perlu install terpisah.

---

## 📁 Struktur File

```
DoYourMagic_Arduino/
├── DoYourMagic_Arduino.ino   ← Entry point (setup & loop)
├── config.h                  ← Konfigurasi pin & parameter
├── .gitignore
├── README.md                 ← (file ini)
└── src/
    ├── buzzer.h              ← Modul buzzer: melodi & alarm adaptif
    ├── sonar.h               ← Modul sensor ultrasonik HC-SR04
    ├── display.h             ← Modul OLED: radar, info panel, animasi
    └── guardian.h            ← Orchestrator: state machine utama
```

### Penjelasan Modul

#### `config.h` — Konfigurasi Sentral

Semua pin dan parameter yang bisa disesuaikan ada di sini. Tidak perlu buka file lain untuk konfigurasi dasar.

#### `src/buzzer.h` — Modul Buzzer

- Melodi startup 9-nada futuristik
- Beep adaptif: interval beep mengecil seiring objek mendekat
- Efek suara transisi zona (danger pulse, all clear)
- Non-blocking: menggunakan state machine internal

#### `src/sonar.h` — Modul Sensor Ultrasonik

- Pembacaan sensor HC-SR04 dengan timeout aman
- Moving average (5 sample) untuk smoothing jarak
- Deteksi dan tracking perubahan zona
- Getter functions: jarak, zona, persentase

#### `src/display.h` — Modul Tampilan OLED

- Splash screen animatif saat boot (radar sweep + progress bar)
- Layout split: radar kiri | info panel kanan
- Radar animasi: cincin konsentris + garis sweep berputar
- Titik objek terdeteksi dipetakan ke posisi radar
- Overlay berkedip saat zona DANGER
- Non-blocking: update pada interval tetap

#### `src/guardian.h` — Orchestrator

- Mengorkestrasi semua modul
- Menangani event perubahan zona
- Debug output via Serial Monitor (9600 baud)
- Fungsi publik: `appSetup()` dan `appLoop()`

---

## ⚙️ Konfigurasi

Edit file `config.h` untuk menyesuaikan perilaku sistem:

```cpp
// Zona jarak (dalam cm) — sesuaikan kebutuhan
#define ZONE_DANGER_CM   20    // Di bawah ini → DANGER  (merah)
#define ZONE_WARNING_CM  60    // Di bawah ini → WARNING (kuning)
                               // Di atas ini  → SAFE    (hijau)

// Kecepatan beep
#define BEEP_INTERVAL_MIN  80   // Interval terpendek (ms) saat sangat dekat
#define BEEP_INTERVAL_MAX  1500 // Interval terpanjang (ms) saat zona warning

// Matikan melodi startup
#define STARTUP_MELODY_ENABLED  false
```

---

## 🚀 Cara Upload

1. **Buka** `DoYourMagic_Arduino.ino` di Arduino IDE
2. **Install library** yang dibutuhkan (lihat bagian Library)
3. **Pilih board**: `Tools → Board → Arduino Uno`
4. **Pilih port** COM yang sesuai
5. **Upload** ✓

### Serial Monitor

Buka Serial Monitor (9600 baud) untuk melihat output debug:

```
╔══════════════════════════════╗
║     SONAR GUARDIAN v1.0      ║
║  Smart Proximity Alert Sys   ║
╚══════════════════════════════╝
[INIT] Buzzer OK
[INIT] Sonar sensor OK
[INIT] OLED OK
[SONAR] Dist: 45.3 cm | Zone: WARNING
[GUARDIAN] Zone changed → DANGER
```

---

## 🎮 Cara Kerja

```
BOOT
 │
 ├─► displayShowSplash()   → Animasi radar + progress bar
 ├─► buzzerPlayStartupMelody() → Nada futuristik
 │
 └─► LOOP
      │
      ├─► sonarLoop()       → Baca jarak (setiap 50ms)
      │    └─► Moving average dari 5 sample terakhir
      │
      ├─► Zone Changed?
      │    ├─► → DANGER  → buzzerPlayDangerPulse()
      │    └─► → SAFE    → buzzerPlayAllClear()
      │
      ├─► buzzerSetAlarm()  → Set interval beep berdasarkan jarak
      ├─► buzzerLoop()      → Tick beep (non-blocking)
      │
      └─► displayLoop()     → Update OLED (setiap 60ms)
           ├─► Radar sweep berputar
           ├─► Angka jarak & zona
           ├─► Bar chart jarak
           └─► Overlay berkedip (jika DANGER)
```

---

## 📐 Zona Deteksi

```
   0cm          20cm              60cm              200cm+
    │<── DANGER ──>│<─── WARNING ────>│<──── SAFE ──────>│
    │  Beep cepat  │  Beep sedang     │   Diam           │
    │  (80-30ms)   │  (80-1500ms)     │                  │
    │  🔴 Border   │  🟡 Normal       │  🟢 Normal       │
    │  berkedip    │                  │                  │
```

---

## 📊 Tampilan OLED

```
┌──────────────────────────────────┐
│  ╭─────╮  │ SONAR               │
│ ╭┼─────┼╮ │ DIST:               │
│ │╭──╮  │├ │  45                 │
│ ╰┼╭╮┼─╯│ │           cm        │
│  ╰╯╰╯  │ │ WARNING             │
│ (radar) │ │▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒│  │
└──────────────────────────────────┘
 ← Radar Sweep → ← Info Panel →
```

---

## 🧠 Paradigma Pemrograman

Project ini menggunakan **paradigma prosedural** murni:

| Aspek        | Implementasi                                               |
| ------------ | ---------------------------------------------------------- |
| State        | Variabel `static` per modul (scope terbatas)               |
| Organisasi   | Satu modul = satu file `.h` berisi fungsi bebas            |
| Coupling     | Modul-modul di-include secara hierarkis lewat `guardian.h` |
| Entry Point  | `setup()` → `appSetup()`, `loop()` → `appLoop()`           |
| Non-blocking | Semua operasi utama menggunakan timestamp (`millis()`)     |

---

_Dibuat dengan ❤️ — SONAR GUARDIAN v1.0.0_
