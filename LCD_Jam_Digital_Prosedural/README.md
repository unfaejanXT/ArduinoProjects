# LCD Jam Digital — Prosedural

Jam digital berbasis NTP yang menampilkan waktu (`HH:MM:SS`) dan tanggal (`DD-MM-YYYY`) di layar OLED SSD1306 128×64 menggunakan **NodeMCU v3 (ESP8266)**.

> **📌 Ini adalah versi Prosedural** dari project [LCD_Jam_Digital](../LCD_Jam_Digital/).
> Fungsionalitas identik, namun paradigma pemrograman diubah dari OOP ke **Prosedural**.

---

## 🔄 Perbandingan OOP vs Prosedural

| Aspek          | OOP (`LCD_Jam_Digital`)      | Prosedural (`LCD_Jam_Digital_Prosedural`) |
| -------------- | ---------------------------- | ----------------------------------------- |
| Modul WiFi     | `class WiFiManager` + `.cpp` | `wifi_manager.h` — fungsi bebas           |
| Modul NTP      | `class NTPManager` + `.cpp`  | `ntp_manager.h` — fungsi bebas            |
| Modul LCD      | `class LCDDisplay` + `.cpp`  | `lcd_display.h` — fungsi bebas            |
| Orchestrator   | `class ClockApp` + `.cpp`    | `clock_app.h` — fungsi bebas              |
| Instansiasi    | `ClockApp app;` (objek)      | tidak ada — langsung panggil fungsi       |
| State          | private member variable      | variabel global file-local (`static`)     |
| File per modul | `.h` + `.cpp`                | `.h` saja                                 |
| Entry point    | `app.begin()` / `app.run()`  | `appSetup()` / `appLoop()`                |

---

## 📁 Struktur File

```
LCD_Jam_Digital_Prosedural/
├── LCD_Jam_Digital_Prosedural.ino  ← Entry point utama
├── config.h                        ← Konfigurasi WiFi & NTP  (dibuat dari .example)
├── config.h.example                ← Template konfigurasi
└── src/
    ├── wifi_manager.h  ← Modul koneksi WiFi (prosedural)
    ├── ntp_manager.h   ← Modul sinkronisasi waktu NTP
    ├── lcd_display.h   ← Modul tampilan OLED SSD1306
    └── clock_app.h     ← Orchestrator state machine
```

---

## ⚡ Hardware

| Komponen    | Keterangan                      |
| ----------- | ------------------------------- |
| **Board**   | NodeMCU v3 (ESP-12E / ESP8266)  |
| **Display** | OLED SSD1306 128×64 pixel (I2C) |

### Koneksi OLED ke NodeMCU v3

| OLED Pin | NodeMCU v3 |
| -------- | ---------- |
| VCC      | 3.3V       |
| GND      | GND        |
| SDA      | D2 (GPIO4) |
| SCL      | D1 (GPIO5) |

---

## 📦 Library yang Dibutuhkan

Install via Arduino IDE → **Library Manager**:

1. **NTPClient** by Fabrice Weinberg
2. **Adafruit SSD1306** by Adafruit
3. **Adafruit GFX Library** by Adafruit

---

## ⚙️ Board Manager

| Setting | Nilai                        |
| ------- | ---------------------------- |
| Board   | NodeMCU 1.0 (ESP-12E Module) |
| Package | esp8266 by ESP8266 Community |

---

## 🛠️ Cara Setup

1. **Clone / salin** folder ini ke direktori Arduino project Anda
2. **Salin** `config.h.example` → `config.h`
3. **Edit** `config.h`:
   ```cpp
   #define WIFI_SSID      "NamaWiFiAnda"
   #define WIFI_PASSWORD  "PasswordWiFiAnda"
   ```
4. **Sesuaikan zona waktu** (default: WIB = UTC+7):
   ```cpp
   #define NTP_UTC_OFFSET  25200  // WIB
   // #define NTP_UTC_OFFSET 28800  // WITA
   // #define NTP_UTC_OFFSET 32400  // WIT
   ```
5. **Install library** dan pilih board yang benar
6. **Upload** ke NodeMCU

---

## 🔄 State Machine

```
[CONNECTING_WIFI] ──OK──→ [SYNCING_NTP] ──OK──→ [RUNNING]
      |                                               |
  (gagal)                                     (WiFi putus)
      ↓                                               ↓
  coba lagi                              [WIFI_LOST] → reconnect
```

| State             | Tampilan OLED                     |
| ----------------- | --------------------------------- |
| `CONNECTING_WIFI` | Animasi titik bergerak            |
| `SYNCING_NTP`     | "Sync NTP..."                     |
| `RUNNING`         | HH:MM:SS + DD-MM-YYYY             |
| `WIFI_LOST`       | "WiFi Terputus! / Reconnecting.." |
| `APP_ERROR`       | "!! ERROR !!"                     |

---

## 📐 Konsep Prosedural

Dalam paradigma **prosedural**, tidak ada class atau instansiasi objek. Sebagai gantinya:

- **Variabel global file-local** (`static`) menyimpan state internal setiap modul
- **Fungsi bebas** (_free functions_) mengoperasikan state tersebut
- Modul dipisah ke file `.h` masing-masing untuk keterbacaan
- Include antar-modul dilakukan secara eksplisit

Contoh perbedaan:

```cpp
// OOP:
ClockApp app;        // instansiasi objek
app.begin();         // panggil metode
app.run();

// Prosedural:
// (tidak ada instansiasi)
appSetup();          // panggil fungsi bebas
appLoop();
```
